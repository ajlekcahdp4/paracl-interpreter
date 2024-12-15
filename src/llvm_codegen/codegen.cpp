#include "llvm_codegen/codegen.hpp"
#include "ezvis/ezvis.hpp"
#include "frontend/analysis/function_explorer.hpp"
#include "frontend/ast/ast_nodes/function_decl.hpp"
#include "frontend/ast/ast_nodes/subscript.hpp"
#include "frontend/ast/ast_nodes/variable_expression.hpp"
#include "frontend/types/types.hpp"

#include <llvm/IR/IRBuilder.h>

#include <ranges>
#include <stdexcept>
#include <variant>
#include <vector>

namespace paracl::llvm_codegen {
using namespace llvm;
namespace ranges = std::ranges;

struct symbol {
  Value *val = nullptr;
  const frontend::ast::variable_expression *var = nullptr;
};

class symtab_scope final : private std::unordered_map<std::string, symbol> {
public:
  symtab_scope() = default;

  auto lookup(std::string_view name) const -> std::optional<symbol> {
    auto found = unordered_map::find(std::string(name));
    if (found == unordered_map::end()) return std::nullopt;
    return found->second;
  }

  void add(std::string_view name, symbol s) { unordered_map::try_emplace(std::string(name), s); }
};

class symtab final : private std::vector<symtab_scope> {
public:
  using vector::begin;
  using vector::empty;
  using vector::end;
  using vector::size;

  void begin_scope() { vector::emplace_back(); }
  void end_scope() { vector::emplace_back(); }

  void add(std::string_view name, symbol value) { vector::back().add(name, value); }

  auto lookup(std::string_view name) -> std::optional<symbol> {
    auto found = ranges::find_if(*this, [name](auto &scope) -> bool {
      return scope.lookup(name).has_value();
    });
    if (found == end()) return std::nullopt;
    return found->lookup(name).value();
  }
};

namespace intrinsics {

void print(int32_t val) {
  fmt::println("{}", val);
}

int32_t read() {
  int32_t v;
  std::cin >> v;
  if (!std::cin.good()) throw std::runtime_error("Invalid read");
  return v;
}

namespace {
auto get_intrinsic_function(std::string_view name, Module &m, Type *ret, ArrayRef<Type *> args = {})
    -> Function * {
  if (auto *ptr = m.getFunction(name)) return ptr;
  auto *func_type = FunctionType::get(ret, args, false);
  return Function::Create(func_type, Function::ExternalLinkage, name, &m);
}
} // namespace
auto get_print_function(Module &m) -> Function * {
  auto &ctx = m.getContext();
  return get_intrinsic_function("__print", m, Type::getVoidTy(ctx), {Type::getInt32Ty(ctx)});
}

auto get_read_function(Module &m) -> Function * {
  auto &ctx = m.getContext();
  return get_intrinsic_function("__read", m, Type::getInt32Ty(ctx));
}

// auto get_print_function(Module &m) -> Function * {
//   auto &ctx = m.getContext();
//   return get_intrinsic_function("__print", m, Type::getVoidTy(ctx), {Type::getInt32Ty(ctx)});
// }
} // namespace intrinsics

namespace ast = frontend::ast;
class codegen_visitor final
    : public ezvis::visitor_base<const ast::i_ast_node, codegen_visitor, llvm::Value *> {
  std::unique_ptr<Module> m;
  IRBuilder<> builder;
  std::unordered_map<const frontend::ast::function_definition *, Function *> funcs;
  const frontend::functions_analytics &fun_analysis;
  symtab sym;
  Function *current_function = nullptr;
  Function *entry = nullptr;

public:
  using to_visit = std::tuple<
      ast::assignment_statement, ast::binary_expression, ast::constant_expression,
      ast::print_statement, ast::read_expression, ast::statement_block, ast::unary_expression,
      ast::variable_expression, ast::return_statement, ast::function_call, ast::if_statement,
      ast::while_statement, ast::function_definition_to_ptr_conv, ast::subscript>;

  EZVIS_VISIT_CT(to_visit)

  codegen_visitor(
      std::string_view module_name, LLVMContext &ctx, const frontend::frontend_driver &drv
  )
      : m(std::make_unique<Module>(module_name, ctx)), builder(ctx), fun_analysis(drv.functions()) {
  }

  Value *generate(const ast::binary_expression &);
  Value *generate(const ast::unary_expression &);
  Value *generate(const ast::variable_expression &);
  Value *generate(const ast::constant_expression &);
  Value *generate(const ast::print_statement &);
  Value *generate(const ast::read_expression &);
  Value *generate(const ast::function_call &);
  Value *generate(const ast::function_definition_to_ptr_conv &);
  Value *generate(const ast::subscript &, bool do_load = true);
  Value *generate(const ast::return_statement &);

  Value *generate(const ast::assignment_statement &);
  Value *generate(const ast::if_statement &);
  Value *generate(const ast::while_statement &);

  Value *generate(const ast::statement_block &, bool global = false);

  EZVIS_VISIT_INVOKER(generate);

  auto &get_ctx() { return m->getContext(); }

  auto emit_module() { return std::move(m); }

  void begin_scope(const frontend::symtab &stab) {
    sym.begin_scope();

    for (auto &[name, attrs] : stab) {
      auto *variable_def = attrs.m_definition;
      assert(variable_def);
      auto *value = [&] {
        auto &ref = *variable_def;
        auto &type = ref.type.base();
        if (variable_def->type && type.get_class() == frontend::types::type_class::E_ARRAY) {
          auto &array_type =
              static_cast<const frontend::types::type_array &>(variable_def->type.base());
          auto *arr = builder.CreateAlloca(
              to_llvm_type(array_type.element_type.base()),
              Constant::getIntegerValue(Type::getInt32Ty(get_ctx()), APInt(32, array_type.size))
          );
          builder.CreateMemSet(
              arr, Constant::getIntegerValue(Type::getInt8Ty(get_ctx()), APInt(8, 0)),
              array_type.size * 4, MaybeAlign()
          );
          return arr;
        } else
          return builder.CreateAlloca(
              to_llvm_type(variable_def->type), 0, nullptr, variable_def->name()
          );
      }();
      sym.add(name, {value, variable_def});
    }
  }

  Type *to_llvm_type(const frontend::types::i_type &type) {
    using namespace frontend::types;
    return ezvis::visit<Type *, type_composite_function, type_builtin>(
        utils::visitors{
            [this](const type_composite_function &func) {
              auto *return_type = to_llvm_type(func.return_type());
              auto args_types = std::views::all(func) |
                  ranges::views::transform([this](auto &type) { return to_llvm_type(type); }) |
                  ranges::to<std::vector>();
              return FunctionType::get(return_type, args_types, false);
            },
            [this](const type_array &arr) { throw std::runtime_error("Array type encountered"); },
            [this](const type_builtin &type) -> Type * {
              switch (type.get_builtin_type_class()) {
              case frontend::types::builtin_type_class::E_BUILTIN_INT:
                return Type::getInt32Ty(get_ctx());
              case frontend::types::builtin_type_class::E_BUILTIN_VOID:
                return Type::getVoidTy(get_ctx());
              default: throw std::invalid_argument("Unknown type encountered");
              }
            }
        },
        type
    );
  }

  void declare_functions(const frontend::functions_analytics &functions) {
    for (auto &&[key, attr] : functions.usegraph) {
      auto &&[name, func] = attr.value;
      auto *func_type = to_llvm_type(func->type);
      auto *llvm_func = Function::Create(
          static_cast<FunctionType *>(func_type), Function::ExternalLinkage, 0, func->name.value(),
          m.get()
      );
      funcs.try_emplace(func, llvm_func);
    }
    auto *entry_type = FunctionType::get(Type::getVoidTy(get_ctx()), false);
    entry = Function::Create(entry_type, Function::ExternalLinkage, 0, "main", m.get());
  }

  void declare_globals(const frontend::ast::ast_container &ast) {
    auto *stab = fun_analysis.global_stab;
    assert(sym.empty());
    sym.begin_scope();
    if (!stab) return;
    for (auto &&[name, attrs] : *fun_analysis.global_stab) {
      auto *def = attrs.m_definition;
      assert(def);
      auto *type = to_llvm_type(def->type);
      auto *global = static_cast<GlobalVariable *>(m->getOrInsertGlobal(name, type));
      global->setInitializer(Constant::getNullValue(type));
      sym.add(name, {global, nullptr});
    }
  }

  auto generate_function(const frontend::ast::function_definition &func) {

    auto *function = this->funcs.at(&func);
    current_function = function;
    auto *entry_block = BasicBlock::Create(get_ctx(), "entry", function);
    builder.SetInsertPoint(entry_block);
    begin_scope(func.param_stab);

    for (auto &&[arg_value, variable_expr] : llvm::zip(function->args(), func)) {
      builder.CreateStore(&arg_value, sym.lookup(variable_expr.name()).value().val);
    }

    generate(
        static_cast<const ast::statement_block &>(func.body()),
        /*global_scope=*/false
    );

    sym.end_scope();
    if (func.type.return_type() == frontend::types::type_builtin::type_void)
      builder.CreateRetVoid();
  }

  void generate(const frontend::ast::ast_container &ast, const frontend::frontend_driver &drv) {
    declare_functions(drv.functions());
    declare_globals(ast);
    for (auto &&[_, attr] : drv.functions().usegraph) {
      auto &&[name, func] = attr.value;
      assert(func);
      generate_function(*func);
    }

    if (ast.get_root_ptr()) {
      ezvis::visit<void, ast::statement_block>(
          [this](auto &st) { generate(st, /*global_scope=*/true); }, *ast.get_root_ptr()
      );
    }
  }
};

Value *codegen_visitor::generate(const ast::binary_expression &expr) {
  auto *lhs = apply(expr.left());
  auto *rhs = apply(expr.right());
  using namespace ast;
  switch (expr.op_type()) {
  case binary_operation::E_BIN_OP_ADD: return builder.CreateAdd(lhs, rhs);
  case binary_operation::E_BIN_OP_SUB: return builder.CreateSub(lhs, rhs);
  case binary_operation::E_BIN_OP_MUL: return builder.CreateMul(lhs, rhs);
  case binary_operation::E_BIN_OP_DIV: return builder.CreateSDiv(lhs, rhs);
  case binary_operation::E_BIN_OP_MOD: return builder.CreateSRem(lhs, rhs);
  case binary_operation::E_BIN_OP_AND: return builder.CreateAnd(lhs, rhs);
  case binary_operation::E_BIN_OP_OR: return builder.CreateOr(lhs, rhs);
  // compare ops
  case binary_operation::E_BIN_OP_EQ: return builder.CreateICmpEQ(lhs, rhs);
  case binary_operation::E_BIN_OP_NE: return builder.CreateICmpNE(lhs, rhs);
  case binary_operation::E_BIN_OP_GT: return builder.CreateICmpSGT(lhs, rhs);
  case binary_operation::E_BIN_OP_LS: return builder.CreateICmpSLT(lhs, rhs);
  case binary_operation::E_BIN_OP_GE: return builder.CreateICmpSGE(lhs, rhs);
  case binary_operation::E_BIN_OP_LE: return builder.CreateICmpSLE(lhs, rhs);
  default: throw std::invalid_argument("Unknown binary operation");
  }
}

Value *codegen_visitor::generate(const ast::unary_expression &expr) {
  auto *val = apply(expr.expr());
  assert(val);
  using enum ast::unary_operation;
  switch (expr.op_type()) {
  case E_UN_OP_NEG: return builder.CreateNeg(val);
  case E_UN_OP_POS: return val;
  case E_UN_OP_NOT: return builder.CreateNot(val);
  default: throw std::invalid_argument("Unknown unary operation");
  }
}

Value *codegen_visitor::generate(const ast::variable_expression &expr) {
  auto *type = to_llvm_type(expr.type);
  auto *ptr = sym.lookup(expr.name()).value().val;
  return builder.CreateLoad(type, ptr);
}

Value *codegen_visitor::generate(const ast::subscript &sub, bool do_load) {
  auto [ptr, var] = sym.lookup(sub.name()).value();
  auto *subscript = apply(*sub.get_subscript());
  assert(var->type);
  assert(var->type.base().get_class() == frontend::types::type_class::E_ARRAY);
  auto &arr_type = static_cast<const frontend::types::type_array &>(var->type.base());
  auto *elem_type = to_llvm_type(arr_type.element_type);
  auto *elem = builder.CreateGEP(
      ArrayType::get(elem_type, arr_type.size), ptr,
      {Constant::getIntegerValue(Type::getInt32Ty(get_ctx()), APInt::getZero(32)), subscript}
  );
  return do_load ? builder.CreateLoad(elem_type, elem) : elem;
}

Value *codegen_visitor::generate(const ast::constant_expression &expr) {
  return Constant::getIntegerValue(to_llvm_type(expr.type), APInt(32, expr.value()));
}

Value *codegen_visitor::generate(const ast::return_statement &ret) {
  if (ret.empty()) return builder.CreateRetVoid();
  auto *val = apply(ret.expr());
  return builder.CreateRet(val);
}

Value *codegen_visitor::generate(const ast::if_statement &stmt) {
  auto *if_true = BasicBlock::Create(get_ctx(), "then", current_function);
  auto *if_false =
      stmt.else_block() ? BasicBlock::Create(get_ctx(), "else", current_function) : nullptr;
  auto *after_if = BasicBlock::Create(get_ctx(), "next", current_function);
  begin_scope(stmt.control_block_symtab);
  auto *condition = builder.CreateIsNotNull(apply(*stmt.cond()));
  builder.CreateCondBr(condition, if_true, if_false ? if_false : after_if);
  auto create_block = [&](const ast::i_ast_node &stblock, BasicBlock *block) {
    builder.SetInsertPoint(block);
    apply(stblock);
    if (block->empty() || !(isa<BranchInst>(block->back()) || isa<ReturnInst>(block->back())))
      builder.CreateBr(after_if);
  };
  create_block(*stmt.true_block(), if_true);
  if (if_false) create_block(*stmt.else_block(), if_false);
  sym.end_scope();
  builder.SetInsertPoint(after_if);
  return nullptr;
}

Value *codegen_visitor::generate(const ast::while_statement &stmt) {
  auto *body = BasicBlock::Create(get_ctx(), "body", current_function);
  auto *cond = BasicBlock::Create(get_ctx(), "condition", current_function);
  auto *exit = BasicBlock::Create(get_ctx(), "exit", current_function);
  auto *after = BasicBlock::Create(get_ctx(), "after", current_function);
  begin_scope(stmt.symbol_table);
  builder.CreateBr(cond);

  builder.SetInsertPoint(cond);
  auto *condition = builder.CreateIsNotNull(apply(*stmt.cond()));
  builder.CreateCondBr(condition, body, exit);

  builder.SetInsertPoint(body);
  apply(*stmt.block());
  builder.CreateBr(cond);

  sym.end_scope();

  builder.SetInsertPoint(exit);
  builder.CreateBr(after);
  builder.SetInsertPoint(after);

  return nullptr;
}

Value *codegen_visitor::generate(const ast::statement_block &stmt_block, bool global) {
  if (global) {
    current_function = entry;
    auto *entry_block = BasicBlock::Create(get_ctx(), "", entry);
    builder.SetInsertPoint(entry_block);
  } else {
    begin_scope(stmt_block.stab);
  }

  for (auto *st : stmt_block) {
    if (frontend::ast::identify_node(st) == ast::ast_node_type::E_FUNCTION_DEFINITION) continue;
    if (frontend::ast::identify_node(st) == ast::ast_node_type::E_RETURN_STATEMENT) break;
    apply(*st);
  }
  if (global) builder.CreateRetVoid();
  sym.end_scope();
  return nullptr;
}

Value *codegen_visitor::generate(const ast::read_expression &read) {
  return builder.CreateCall(intrinsics::get_read_function(*m), {});
}

Value *codegen_visitor::generate(const ast::print_statement &prnt) {
  auto *val = apply(prnt.expr());
  return builder.CreateCall(intrinsics::get_print_function(*m), {val});
}

Value *codegen_visitor::generate(const ast::function_definition_to_ptr_conv &conv) {
  return funcs.at(&conv.definition());
}

Value *codegen_visitor::generate(const ast::assignment_statement &stmt) {
  auto *val = apply(stmt.right());
  assert(val);
  for (auto &&v : ranges::views::reverse(stmt)) {

    auto *ptr = [&] -> Value * {
      if (std::holds_alternative<ast::subscript>(v)) {
        auto &sub = std::get<ast::subscript>(v);
        return generate(sub, false);
      }
      auto &var = std::get<ast::variable_expression>(v);
      if (!var.type || var.type.base().get_class() != frontend::types::type_class::E_ARRAY)
        return sym.lookup(var.name()).value().val;
      return nullptr;
    }();
    if (!ptr) continue;
    builder.CreateStore(val, ptr);
  }
  return val;
}

Value *codegen_visitor::generate(const ast::function_call &call) {
  auto args = call | ranges::views::transform([this](auto &a) { return apply(*a); }) |
      ranges::to<std::vector>();
  auto *callee = call.get_callee();
  assert(callee);
  return builder.CreateCall(funcs.at(callee), args);
}

auto emit_llvm(const frontend::frontend_driver &drv, LLVMContext &ctx)
    -> std::unique_ptr<llvm::Module> {
  codegen_visitor visitor(drv.get_filename(), ctx, drv);
  visitor.generate(drv.ast(), drv);
  return visitor.emit_module();
}

} // namespace paracl::llvm_codegen

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include <iostream>
#include <stdexcept>

#include "bytecode_vm/decl_vm.hpp"
#include "bytecode_vm/opcodes.hpp"
#include "utils/serialization.hpp"

namespace paracl::bytecode_vm::instruction_set {

constexpr decl_vm::instruction_desc<E_PUSH_CONST_UNARY, uint32_t> push_const_desc = "push_const";
constexpr auto push_const_instr = push_const_desc >>
    [](auto &&ctx, auto &&attr) { ctx.push(ctx.constant(std::get<0>(attr))); };

constexpr decl_vm::instruction_desc<E_RETURN_NULLARY> return_desc = "ret";
constexpr auto return_instr = return_desc >> [](auto &&ctx, auto &&) {
  if (ctx.stack_empty()) ctx.halt();
  else {
    auto sp = ctx.pop();
    ctx.set_sp(sp);
    auto ip = ctx.pop();
    ctx.set_ip(ip);
  }
};

constexpr decl_vm::instruction_desc<E_POP_NULLARY> pop_desc = "pop";
constexpr auto pop_instr = pop_desc >> [](auto &&ctx, auto &&) { ctx.pop(); };

constexpr decl_vm::instruction_desc<E_ADD_NULLARY> add_desc = "add";
constexpr auto add_instr = add_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first + second);
};

constexpr decl_vm::instruction_desc<E_SUB_NULLARY> sub_desc = "sub";
constexpr auto sub_instr = sub_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first - second);
};

constexpr decl_vm::instruction_desc<E_MUL_NULLARY> mul_desc = "mul";
constexpr auto mul_instr = mul_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first * second);
};

constexpr decl_vm::instruction_desc<E_DIV_NULLARY> div_desc = "div";
constexpr auto div_instr = div_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first / second);
};

constexpr decl_vm::instruction_desc<E_MOD_NULLARY> mod_desc = "mod";
constexpr auto mod_instr = mod_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first % second);
};

constexpr decl_vm::instruction_desc<E_AND_NULLARY> and_desc = "and";
constexpr auto and_instr = and_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first && second);
};

constexpr decl_vm::instruction_desc<E_OR_NULLARY> or_desc = "or";
constexpr auto or_instr = or_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first || second);
};

constexpr decl_vm::instruction_desc<E_NOT_NULLARY> not_desc = "not";
constexpr auto not_instr = not_desc >> [](auto &&ctx, auto &&) {
  auto first = ctx.pop();
  ctx.push(!first);
};

constexpr decl_vm::instruction_desc<E_CMP_EQ_NULLARY> cmp_eq_desc = "cmp_eq";
constexpr auto cmp_eq_instr = cmp_eq_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first == second);
};

constexpr decl_vm::instruction_desc<E_CMP_NE_NULLARY> cmp_ne_desc = "cmp_ne";
constexpr auto cmp_ne_instr = cmp_ne_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first != second);
};

constexpr decl_vm::instruction_desc<E_CMP_GT_NULLARY> cmp_gt_desc = "cmp_gt";
constexpr auto cmp_gt_instr = cmp_gt_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first > second);
};

constexpr decl_vm::instruction_desc<E_CMP_LS_NULLARY> cmp_ls_desc = "cmp_ls";
constexpr auto cmp_ls_instr = cmp_ls_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first < second);
};

constexpr decl_vm::instruction_desc<E_CMP_GE_NULLARY> cmp_ge_desc = "cmp_ge";
constexpr auto cmp_ge_instr = cmp_ge_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first >= second);
};

constexpr decl_vm::instruction_desc<E_CMP_LE_NULLARY> cmp_le_desc = "cmp_le";
constexpr auto cmp_le_instr = cmp_le_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first <= second);
};

constexpr decl_vm::instruction_desc<E_PRINT_NULLARY> print_desc = "print";
constexpr auto print_instr = print_desc >> [](auto &&ctx, auto &&) {
  auto first = ctx.pop();
  std::cout << std::dec << first << "\n";
};

constexpr decl_vm::instruction_desc<E_PUSH_READ_NULLARY> push_read_desc = "push_read";
constexpr auto push_read = push_read_desc >> [](auto &&ctx, auto &&) {
  int val;
  std::cin >> val;
  ctx.push(val);
};

constexpr decl_vm::instruction_desc<E_MOV_LOCAL_UNARY, uint32_t> mov_local_desc = "mov_local";
constexpr auto mov_local_instr = mov_local_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.pop();
  ctx.at_stack(std::get<0>(attr)) = val;
};

constexpr decl_vm::instruction_desc<E_MOV_LOCAL_REL_UNARY, int32_t> mov_local_rel_desc = "mov_local_rel";
constexpr auto mov_local_rel_instr = mov_local_rel_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.pop();
  ctx.at_stack(std::get<0>(attr) + ctx.sp()) = val;
};

constexpr decl_vm::instruction_desc<E_PUSH_LOCAL_UNARY, uint32_t> push_local_desc = "push_local";
constexpr auto push_local_instr = push_local_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.at_stack(std::get<0>(attr));
  ctx.push(val);
};

constexpr decl_vm::instruction_desc<E_PUSH_LOCAL_DYNAMIC_REL_NULLARY> push_local_dynamic_rel_desc =
    "push_local_dyn_rel";
constexpr auto push_local_dynamic_rel_instr = push_local_desc >> [](auto &&ctx, auto &&attr) {
  auto first = ctx.pop();
  auto val = ctx.at_stack(first);
  ctx.push(val);
};

constexpr decl_vm::instruction_desc<E_PUSH_LOCAL_REL_UNARY, int32_t> push_local_rel_desc = "push_local_rel";
constexpr auto push_local_rel_instr = push_local_rel_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.at_stack(std::get<0>(attr) + ctx.sp());
  ctx.push(val);
};

constexpr auto conditional_jump = [](auto &&ctx, auto &&attr, bool cond) {
  if (!cond) return;
  ctx.set_ip(std::get<0>(attr));
};

constexpr decl_vm::instruction_desc<E_JMP_UNARY, uint32_t> jmp_desc = "jmp";
constexpr auto jmp_instr = jmp_desc >> [](auto &&ctx, auto &&attr) { conditional_jump(ctx, attr, true); };

constexpr decl_vm::instruction_desc<E_JMP_DYNAMIC_NULLARY> jmp_dynamic_desc = "jmp_dynamic";
constexpr auto jmp_dynamic_instr = jmp_dynamic_desc >> [](auto &&ctx, auto &&attr) {
  auto first = ctx.pop();
  ctx.set_ip(first);
};

constexpr decl_vm::instruction_desc<E_JMP_DYNAMIC_REL_UNARY, int32_t> jmp_dynamic_rel_desc = "jmp_dynamic_rel";
constexpr auto jmp_dynamic_rel_instr = jmp_dynamic_rel_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.at_stack(std::get<0>(attr) + ctx.sp());
  ctx.set_ip(val);
};

constexpr decl_vm::instruction_desc<E_JMP_TRUE_UNARY, uint32_t> jmp_true_desc = "jmp_true";
constexpr auto jmp_true_instr = jmp_true_desc >> [](auto &&ctx, auto &&attr) {
  auto first = ctx.pop();
  conditional_jump(ctx, attr, first);
};

constexpr decl_vm::instruction_desc<E_JMP_FALSE_UNARY, uint32_t> jmp_false_desc = "jmp_false";
constexpr auto jmp_false_instr = jmp_false_desc >> [](auto &&ctx, auto &&attr) {
  auto first = ctx.pop();
  conditional_jump(ctx, attr, !first);
};

constexpr decl_vm::instruction_desc<E_SETUP_CALL_NULLARY> setup_call_desc = "call_setup";
constexpr auto setup_call_instr = setup_call_desc >> [](auto &&ctx, auto &&attr) {
  auto cur_sp = ctx.sp();
  ctx.push(cur_sp);
  ctx.set_sp(ctx.stack_size());
};

constexpr decl_vm::instruction_desc<E_PUSH_SP_NULLARY> push_sp_desc = "push_sp";
constexpr auto push_sp_instr = push_sp_desc >> [](auto &&ctx, auto &&attr) {
  auto cur_sp = ctx.sp();
  ctx.push(cur_sp);
};

constexpr decl_vm::instruction_desc<E_UPDATE_SP_UNARY, uint32_t> update_sp_desc = "update_sp";
constexpr auto update_sp_instr = update_sp_desc >> [](auto &&ctx, auto &&attr) {
  auto new_sp = ctx.stack_size() - std::get<0>(attr);
  ctx.set_sp(new_sp);
};

static const auto paracl_isa = decl_vm::instruction_set_description(
    push_const_instr, return_instr, pop_instr, add_instr, sub_instr, mul_instr, div_instr, mod_instr, and_instr,
    or_instr, cmp_eq_instr, cmp_ne_instr, cmp_gt_instr, cmp_ls_instr, cmp_ge_instr, cmp_le_instr, print_instr,
    push_read, mov_local_instr, mov_local_rel_instr, push_local_instr, push_local_rel_instr, jmp_instr, jmp_true_instr,
    jmp_false_instr, not_instr, setup_call_instr, jmp_dynamic_instr, jmp_dynamic_rel_instr,
    push_local_dynamic_rel_instr, push_sp_instr, update_sp_instr
);

} // namespace paracl::bytecode_vm::instruction_set

namespace paracl::bytecode_vm {

inline auto create_paracl_vm() {
  return decl_vm::virtual_machine<decltype(instruction_set::paracl_isa)>{instruction_set::paracl_isa};
}

} // namespace paracl::bytecode_vm
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

#include <iostream>
#include <stdexcept>

#include "bytecode_vm/decl_vm.hpp"
#include "bytecode_vm/opcodes.hpp"
#include "utils/serialization.hpp"

namespace paracl::bytecode_vm::instruction_set {

static constexpr decl_vm::instruction_desc<E_PUSH_CONST_UNARY, uint32_t> push_const_desc = "push_const";
static constexpr auto                                                    push_const_instr = push_const_desc >>
                                         [](auto &&ctx, auto &&attr) { ctx.push(ctx.pool().at(std::get<0>(attr))); };

static constexpr decl_vm::instruction_desc<E_RETURN_NULLARY> return_desc = "ret";
static constexpr auto return_instr = return_desc >> [](auto &&ctx, auto &&) { ctx.halt(); };

static constexpr decl_vm::instruction_desc<E_POP_NULLARY> pop_desc = "pop";
static constexpr auto pop_instr = pop_desc >> [](auto &&ctx, auto &&) { ctx.pop(); };

static constexpr decl_vm::instruction_desc<E_ADD_NULLARY> add_desc = "add";
static constexpr auto                                     add_instr = add_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first + second);
};

static constexpr decl_vm::instruction_desc<E_SUB_NULLARY> sub_desc = "sub";
static constexpr auto                                     sub_instr = sub_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first - second);
};

static constexpr decl_vm::instruction_desc<E_MUL_NULLARY> mul_desc = "mul";
static constexpr auto                                     mul_instr = mul_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first * second);
};

static constexpr decl_vm::instruction_desc<E_DIV_NULLARY> div_desc = "div";
static constexpr auto                                     div_instr = div_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first / second);
};

static constexpr decl_vm::instruction_desc<E_MOD_NULLARY> mod_desc = "mod";
static constexpr auto                                     mod_instr = mod_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first % second);
};

static constexpr decl_vm::instruction_desc<E_AND_NULLARY> and_desc = "and";
static constexpr auto                                     and_instr = and_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first && second);
};

static constexpr decl_vm::instruction_desc<E_OR_NULLARY> or_desc = "or";
static constexpr auto                                    or_instr = or_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first || second);
};

static constexpr decl_vm::instruction_desc<E_NOT_NULLARY> not_desc = "not";
static constexpr auto                                     not_instr = not_desc >> [](auto &&ctx, auto &&) {
  auto first = ctx.pop();
  ctx.push(!first);
};

static constexpr decl_vm::instruction_desc<E_CMP_EQ_NULLARY> cmp_eq_desc = "cmp_eq";
static constexpr auto                                        cmp_eq_instr = cmp_eq_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first == second);
};

static constexpr decl_vm::instruction_desc<E_CMP_NE_NULLARY> cmp_ne_desc = "cmp_ne";
static constexpr auto                                        cmp_ne_instr = cmp_ne_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first != second);
};

static constexpr decl_vm::instruction_desc<E_CMP_GT_NULLARY> cmp_gt_desc = "cmp_gt";
static constexpr auto                                        cmp_gt_instr = cmp_gt_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first > second);
};

static constexpr decl_vm::instruction_desc<E_CMP_LS_NULLARY> cmp_ls_desc = "cmp_ls";
static constexpr auto                                        cmp_ls_instr = cmp_ls_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first < second);
};

static constexpr decl_vm::instruction_desc<E_CMP_GE_NULLARY> cmp_ge_desc = "cmp_ge";
static constexpr auto                                        cmp_ge_instr = cmp_ge_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first >= second);
};

static constexpr decl_vm::instruction_desc<E_CMP_LE_NULLARY> cmp_le_desc = "cmp_le";
static constexpr auto                                        cmp_le_instr = cmp_le_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first <= second);
};

static constexpr decl_vm::instruction_desc<E_PRINT_NULLARY> print_desc = "print";
static constexpr auto                                       print_instr = print_desc >> [](auto &&ctx, auto &&) {
  auto first = ctx.pop();
  std::cout << std::dec << first << "\n";
};

static constexpr decl_vm::instruction_desc<E_PUSH_READ_NULLARY> push_read_desc = "push_read";
static constexpr auto                                           push_read = push_read_desc >> [](auto &&ctx, auto &&) {
  int val;
  std::cin >> val;
  ctx.push(val);
};

static constexpr decl_vm::instruction_desc<E_MOV_LOCAL_UNARY, uint32_t> mov_local_desc = "mov_local";
static constexpr auto mov_local_instr = mov_local_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.pop();
  ctx.at_stack(std::get<0>(attr)) = val;
};

static constexpr decl_vm::instruction_desc<E_PUSH_LOCAL_UNARY, uint32_t> push_local_desc = "push_local";
static constexpr auto push_local_instr = push_local_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.at_stack(std::get<0>(attr));
  ctx.push(val);
};

static constexpr auto conditional_jump = [](auto &&ctx, auto &&attr, bool cond) {
  if (!cond) return;
  ctx.set_ip(std::get<0>(attr));
};

static constexpr decl_vm::instruction_desc<E_JMP_UNARY, uint32_t> jmp_desc = "jmp";
static constexpr auto jmp_instr = jmp_desc >> [](auto &&ctx, auto &&attr) { conditional_jump(ctx, attr, true); };

static constexpr decl_vm::instruction_desc<E_JMP_TRUE_UNARY, uint32_t> jmp_true_desc = "jmp_true";
static constexpr auto jmp_true_instr = jmp_true_desc >> [](auto &&ctx, auto &&attr) {
  auto first = ctx.pop();
  conditional_jump(ctx, attr, first);
};

static constexpr decl_vm::instruction_desc<E_JMP_FALSE_UNARY, uint32_t> jmp_false_desc = "jmp_false";
static constexpr auto jmp_false_instr = jmp_false_desc >> [](auto &&ctx, auto &&attr) {
  auto first = ctx.pop();
  conditional_jump(ctx, attr, !first);
};

static const auto paracl_isa = decl_vm::instruction_set_description(
    push_const_instr, return_instr, pop_instr, add_instr, sub_instr, mul_instr, div_instr, mod_instr, and_instr,
    or_instr, cmp_eq_instr, cmp_ne_instr, cmp_gt_instr, cmp_ls_instr, cmp_ge_instr, cmp_le_instr, print_instr,
    push_read, mov_local_instr, push_local_instr, jmp_instr, jmp_true_instr, jmp_false_instr, not_instr);

} // namespace paracl::bytecode_vm::instruction_set

namespace paracl::bytecode_vm {

static inline auto create_paracl_vm() {
  using namespace paracl::bytecode_vm::decl_vm;
  using namespace paracl::bytecode_vm::instruction_set;

  return virtual_machine<decltype(paracl_isa)>{paracl_isa};
}

} // namespace paracl::bytecode_vm
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

#include <cstdint>

#include <iostream>
#include <stdexcept>

#include "bytecode_vm/decl_vm.hpp"
#include "bytecode_vm/opcodes.hpp"
#include "utils/serialization.hpp"

namespace paracl::bytecode_vm {

namespace instruction_set {

using namespace decl_vm;

static constexpr instruction_desc<E_PUSH_CONST_UNARY, uint32_t> push_const_desc = "push_const";
static constexpr auto                                           push_const_instr = push_const_desc >>
                                         [](auto &&ctx, auto &&attr) { ctx.push(ctx.pool().at(std::get<0>(attr))); };

static constexpr instruction_desc<E_RETURN_NULLARY> return_desc = "ret";
static constexpr auto return_instr = return_desc >> [](auto &&ctx, auto &&) { ctx.halt(); };

static constexpr instruction_desc<E_POP_NULLARY> pop_desc = "pop";
static constexpr auto                            pop_instr = pop_desc >> [](auto &&ctx, auto &&) { ctx.pop(); };

static constexpr instruction_desc<E_ADD_NULLARY> add_desc = "add";
static constexpr auto                            add_instr = add_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first + second);
};

static constexpr instruction_desc<E_SUB_NULLARY> sub_desc = "sub";
static constexpr auto                            sub_instr = sub_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first - second);
};

static constexpr instruction_desc<E_MUL_NULLARY> mul_desc = "mul";
static constexpr auto                            mul_instr = mul_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first * second);
};

static constexpr instruction_desc<E_DIV_NULLARY> div_desc = "div";
static constexpr auto                            div_instr = div_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first / second);
};

static constexpr instruction_desc<E_MOD_NULLARY> mod_desc = "mod";
static constexpr auto                            mod_instr = mod_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first % second);
};

static constexpr instruction_desc<E_PRINT_NULLARY> print_desc = "print";
static constexpr auto                              print_instr = print_desc >> [](auto &&ctx, auto &&) {
  auto first = ctx.pop();
  std::cout << std::dec << first << "\n";
};

static constexpr instruction_desc<E_PUSH_READ_NULLARY> push_read_desc = "push_read";
static constexpr auto                                  push_read = push_read_desc >> [](auto &&ctx, auto &&) {
  int val;
  std::cin >> val;
  ctx.push(val);
};

static constexpr instruction_desc<E_MOV_LOCAL_UNARY, uint32_t> mov_local_desc = "mov_local";
static constexpr auto mov_local_instr = mov_local_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.pop();
  ctx.at_stack(std::get<0>(attr)) = val;
};

static constexpr instruction_desc<E_PUSH_LOCAL_UNARY, uint32_t> push_local_desc = "push_local";
static constexpr auto push_local_instr = push_local_desc >> [](auto &&ctx, auto &&attr) {
  auto val = ctx.at_stack(std::get<0>(attr));
  ctx.push(val);
};

enum compare_state {
  E_CMP_EQ,
  E_CMP_GT,
  E_CMP_LS,
};

static constexpr auto conditional_jump = [](auto &&ctx, auto &&attr, bool cond) {
  if (!cond) return;
  ctx.set_ip(std::get<0>(attr));
};

static constexpr instruction_desc<E_CMP_NULLARY> cmp_desc = "cmp";
static constexpr auto                            cmp_instr = cmp_desc >> [](auto &&ctx, auto &&) {
  auto &state = ctx.state();
  auto  second = ctx.pop();
  auto  first = ctx.pop();

  if (first == second) {
    state = E_CMP_EQ;
  } else if (first < second) {
    state = E_CMP_LS;
  } else {
    state = E_CMP_GT;
  }
};

static constexpr instruction_desc<E_JMP_ABS_UNARY, uint32_t> jmp_desc = "jmp";
static constexpr auto jmp_instr = jmp_desc >> [](auto &&ctx, auto &&attr) { conditional_jump(ctx, attr, true); };

static constexpr instruction_desc<E_JMP_EQ_ABS_UNARY, uint32_t> jmp_eq_desc = "jmp_eq";
static constexpr auto jmp_eq_instr = jmp_eq_desc >> [](auto &&ctx, auto &&attr) {
  conditional_jump(ctx, attr, ctx.state() == E_CMP_EQ);
};

static constexpr instruction_desc<E_JMP_NE_ABS_UNARY, uint32_t> jmp_ne_desc = "jmp_eq";
static constexpr auto jmp_ne_instr = jmp_ne_desc >> [](auto &&ctx, auto &&attr) {
  conditional_jump(ctx, attr, ctx.state() != E_CMP_EQ);
};

static constexpr instruction_desc<E_JMP_GT_ABS_UNARY, uint32_t> jmp_gt_desc = "jmp_gt";
static constexpr auto jmp_gt_instr = jmp_gt_desc >> [](auto &&ctx, auto &&attr) {
  conditional_jump(ctx, attr, ctx.state() == E_CMP_GT);
};

static constexpr instruction_desc<E_JMP_LS_ABS_UNARY, uint32_t> jmp_ls_desc = "jmp_ls";
static constexpr auto jmp_ls_instr = jmp_ls_desc >> [](auto &&ctx, auto &&attr) {
  conditional_jump(ctx, attr, ctx.state() == E_CMP_LS);
};

static constexpr instruction_desc<E_JMP_GE_ABS_UNARY, uint32_t> jmp_ge_desc = "jmp_ge";
static constexpr auto jmp_ge_instr = jmp_ge_desc >> [](auto &&ctx, auto &&attr) {
  conditional_jump(ctx, attr, ctx.state() == E_CMP_GT || ctx.state() == E_CMP_EQ);
};

static constexpr instruction_desc<E_JMP_LE_ABS_UNARY, uint32_t> jmp_le_desc = "jmp_le";
static constexpr auto jmp_le_instr = jmp_le_desc >> [](auto &&ctx, auto &&attr) {
  conditional_jump(ctx, attr, ctx.state() == E_CMP_LS || ctx.state() == E_CMP_EQ);
};

static const auto paracl_isa = instruction_set_description(
    push_const_instr, return_instr, pop_instr, add_instr, sub_instr, mul_instr, div_instr, mod_instr, print_instr,
    push_read, mov_local_instr, push_local_instr, cmp_instr, jmp_instr, jmp_eq_instr, jmp_ne_instr, jmp_gt_instr,
    jmp_ls_instr, jmp_ge_instr, jmp_le_instr);

} // namespace instruction_set

static inline auto create_paracl_vm() {
  return decl_vm::virtual_machine<instruction_set::compare_state, decltype(instruction_set::paracl_isa)>{
      instruction_set::paracl_isa};
}

} // namespace paracl::bytecode_vm
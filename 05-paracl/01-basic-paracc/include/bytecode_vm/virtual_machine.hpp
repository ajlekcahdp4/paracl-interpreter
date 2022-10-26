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

namespace instruction_set {} // namespace instruction_set

namespace instruction_set {

using namespace decl_vm;

static constexpr instruction_desc<E_PUSH_CONST_UNARY, uint32_t> push_const_desc = "push_const";
static constexpr auto                                           push_const_instr = push_const_desc >>
                                         [](auto &&ctx, auto &&attr) { ctx.push(ctx.pool().at(std::get<0>(attr))); };

static constexpr instruction_desc<E_RETURN_NULLARY> return_desc = "ret";
static constexpr auto return_instr = return_desc >> [](auto &&ctx, auto &&) { ctx.halt(); };

static const instruction_desc<E_POP_NULLARY> pop_desc = "pop";
static const auto                            pop_instr = pop_desc >> [](auto &&ctx, auto &&) { ctx.pop(); };

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

static constexpr instruction_desc<E_DIV_NULLARY> div_desc = "mul";
static constexpr auto                            div_instr = div_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first / second);
};

static constexpr instruction_desc<E_MOD_NULLARY> mod_desc = "mul";
static constexpr auto                            mod_instr = mod_desc >> [](auto &&ctx, auto &&) {
  auto second = ctx.pop();
  auto first = ctx.pop();
  ctx.push(first % second);
};

static constexpr instruction_desc<E_PRINT_NULLARY> print_desc = "print";
static constexpr auto                              print_instr = print_desc >> [](auto &&ctx, auto &&) {
  auto first = ctx.pop();
  std::cout << first << "\n";
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

static const auto paracl_isa =
    instruction_set_description(push_const_instr, return_instr, pop_instr, add_instr, sub_instr, mul_instr, div_instr,
                                mod_instr, print_instr, push_read, mov_local_instr, push_local_instr);

} // namespace instruction_set

static inline auto create_paracl_vm() {
  return decl_vm::virtual_machine<instruction_set::compare_state, decltype(instruction_set::paracl_isa)>{instruction_set::paracl_isa};
}

} // namespace paracl::bytecode_vm
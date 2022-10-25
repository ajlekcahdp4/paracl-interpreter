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

#include <array>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>

#include "bytecode_vm/chunk.hpp"
#include "utils/serialization.hpp"
#include "utils/utils.hpp"

namespace paracl::decl_vm {

template <typename, typename> struct instruction;

using opcode_underlying_type = uint8_t;
template <opcode_underlying_type ident, typename... Ts> struct instruction_desc {
  static constexpr auto opcode = ident;
  static constexpr auto binary_size = sizeof(opcode_underlying_type) + (sizeof(Ts) + ...);

  const char *name = nullptr;
  using attribute_types = std::tuple<Ts...>;

  constexpr instruction_desc(const char *debug_name) : name{debug_name} {
    if (name[0] == '\0') throw std::runtime_error{"Empty debug names aren't allowed"};
  }

  constexpr auto operator>>(auto action) const { return instruction(*this, action); }
};

template <typename t_desc, typename t_action> struct instruction {
  using description_type = t_desc;
  using attribute_tuple_type = typename description_type::attribute_types;

  const t_desc description;
  t_action     action = nullptr;

  constexpr instruction(t_desc p_description, t_action p_action) : description{p_description}, action{p_action} {}
  constexpr auto get_name() const { return description.name; }
  constexpr auto get_opcode() const { return t_desc::opcode; }
  constexpr auto get_size() const { return t_desc::binary_size; }

  struct decoded_instruction {
    const instruction   *instr;
    attribute_tuple_type attributes;
  };

  template <std::size_t I>
  static std::tuple_element_t<I, attribute_tuple_type> decode_attribute(auto &first, auto last) {
    auto [val, iter] =
        paracl::utils::serialization::read_little_endian<std::tuple_element_t<I, attribute_tuple_type>>(first, last);
    if (!val) {
      throw std::runtime_error{"Decoding error"};
    }
    first = iter;
    return val.value();
  }

  template <std::size_t... I>
  static attribute_tuple_type decode_attributes(auto &first, auto last, std::index_sequence<I...>) {
    return std::make_tuple(decode_attribute<I>(first, last)...);
  }

  decoded_instruction decode(auto &first, auto last) const {
    return decoded_instruction{
        this, decode_attributes(first, last, std::make_index_sequence<std::tuple_size_v<attribute_tuple_type>>{})};
  }
};

using execution_value_type = int;

template <typename... t_instructions> struct instruction_set_description {
  using instruction_variant_type = std::variant<std::monostate, t_instructions *...>;
  std::array<instruction_variant_type, std::numeric_limits<opcode_underlying_type>::max() + 1> instruction_lookup_table;

  constexpr instruction_set_description(t_instructions... instructions) {
    ((instruction_lookup_table[instructions.get_opcode()] = std::addressof(instructions)), ...);
  }
};

template <typename t_desc> class virtual_machine {
public:
  t_desc instruction_set;

  struct context {
  private:
    paracl::bytecode_vm::chunk                m_program_code;
    std::vector<execution_value_type> m_execution_stack;
    bool                                      halted = false;

  public:
    paracl::bytecode_vm::binary_code_buffer::const_iterator m_ip, m_ip_end;
    context() = default;

    context(const paracl::bytecode_vm::chunk &ch) : m_program_code{ch} {
      m_ip = code().begin();
      m_ip_end = code().end();
    }

    context(paracl::bytecode_vm::chunk &&ch) : m_program_code{std::move(ch)} {
      m_ip = code().begin();
      m_ip_end = code().end();
    }

    auto &ip() { return m_ip; }
    
    const auto &chunk() const { return m_program_code; }
    const auto &code() const { return chunk().m_binary_code; }
    const auto &pool() const { return chunk().m_constant_pool; }

    void set_ip(uint32_t new_ip) {
      m_ip = code().begin();
      std::advance(m_ip, new_ip);
    }

    auto pop() {
      if (m_execution_stack.size() == 0) throw std::runtime_error{"Bad stack pop"};
      auto top = m_execution_stack.back();
      m_execution_stack.pop_back();
      return top;
    }

    void push(execution_value_type val) { m_execution_stack.push_back(val); }
    void halt() { halted = true; }
    bool is_halted() const { return halted; }

  } execution_context;

  auto &ctx() { return execution_context; }

  constexpr virtual_machine(t_desc desc) : instruction_set{desc} {}

  void set_program_code(paracl::bytecode_vm::chunk &&ch) { ctx() = context{std::move(ch)}; }
  void set_program_code(const paracl::bytecode_vm::chunk &ch) { ctx() = context{ch}; }

  void execute_instruction() {
    if (ctx().is_halted()) throw std::runtime_error{"Can't execute, VM is halted"};
    auto current_instruction = instruction_set.instruction_lookup_table[*(ctx().m_ip++)];

    // clang-format off
    std::visit(paracl::utils::visitors{
      [&](std::monostate) {
        ctx().halt();
        throw std::runtime_error{"Unknown opcode"};},
      [&](auto &&instr) {
        auto attr = instr->decode(ctx().m_ip, ctx().m_ip_end).attributes;
        instr->action(ctx(), attr); }}, current_instruction);
    // clang-format on
  }
};

} // namespace paracl::decl_vm
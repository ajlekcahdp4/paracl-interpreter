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
#include <bits/utility.h>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string_view>

#include <tuple>
#include <variant>
#include <vector>

#include "utils.hpp"
#include "utils/serialization.hpp"

namespace paracl::bytecode_vm::decl_vm {

using constant_pool = std::vector<int>;
using binary_code_buffer = std::vector<uint8_t>;

class chunk {
public:
  binary_code_buffer m_binary_code;
  constant_pool      m_constant_pool;

  using value_type = uint8_t;

  chunk() = default;

  chunk(std::vector<uint8_t> &&p_bin, constant_pool &&p_const)
      : m_binary_code{std::move(p_bin)}, m_constant_pool{std::move(p_const)} {}

  template <std::input_iterator binary_it, std::input_iterator constant_it>
  chunk(binary_it bin_begin, binary_it bin_end, constant_it const_begin, constant_it const_end)
      : m_binary_code{bin_begin, bin_end}, m_constant_pool{const_begin, const_end} {}

  template <typename T> void push_value(T val) {
    utils::serialization::write_little_endian(val, std::back_inserter(m_binary_code));
  };

  void push_back(uint8_t code) { m_binary_code.push_back(code); }
  void push_byte(uint8_t code) { m_binary_code.push_back(code); }
  void push_signed_byte(int8_t val) { m_binary_code.push_back(std::bit_cast<uint8_t>(val)); }

  void set_constant_pool(std::vector<int> &&constants) { m_constant_pool = std::move(constants); }
};

std::optional<chunk> read_chunk(std::istream &);
void                 write_chunk(std::ostream &, const chunk &);

template <typename, typename> struct instruction;

using opcode_underlying_type = uint8_t;
template <opcode_underlying_type ident, typename... Ts> struct instruction_desc {
  static constexpr auto opcode = ident;
  static constexpr auto binary_size = sizeof(opcode_underlying_type) + (sizeof(Ts) + ... + 0);

  const std::string_view name;
  using attribute_types = std::tuple<Ts...>;

  constexpr auto        get_name() const { return name; }
  static constexpr auto get_opcode() { return opcode; }
  static constexpr auto get_size() { return binary_size; }

  constexpr instruction_desc(const char *debug_name) : name{debug_name} {
    if (!debug_name || name[0] == '\0') throw std::runtime_error{"Empty debug names aren't allowed"};
  }

  constexpr auto operator>>(auto action) const { return instruction{*this, action}; }

  template <size_t... I> static void pretty_print(auto &os, const attribute_types &tuple, std::index_sequence<I...>) {
    (..., (os << (I == 0 ? "" : ", "), utils::serialization::padded_hex{}(os, std::get<I>(tuple))));
  }

  template <typename t_stream> t_stream &pretty_print(t_stream &os, const attribute_types &attr) const {
    os << name;
    if constexpr (std::tuple_size_v<attribute_types> != 0) {
      os << " [ ";
      pretty_print(os, attr, std::make_index_sequence<std::tuple_size_v<attribute_types>>());
      os << " ]";
    }
    return os;
  }
};

template <typename t_desc, typename t_action> struct instruction {
  using description_type = t_desc;
  using attribute_tuple_type = typename description_type::attribute_types;

  const t_desc description;
  t_action     action = nullptr;

  constexpr instruction(t_desc p_description, t_action p_action) : description{p_description}, action{p_action} {}
  constexpr auto get_name() const { return description.get_name(); }
  constexpr auto get_opcode() const { return description.get_opcode(); }
  constexpr auto get_size() const { return description.get_size(); }

  template <typename t_stream> t_stream &pretty_print(t_stream &os, const attribute_tuple_type &attr) const {
    description.pretty_print(os, attr);
    return os;
  }

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
  using instruction_variant_type = std::variant<std::monostate, const t_instructions *...>;
  using instruction_tuple_type = std::tuple<t_instructions...>;

  std::array<instruction_variant_type, std::numeric_limits<opcode_underlying_type>::max() + 1> instruction_lookup_table;

  constexpr instruction_set_description(const t_instructions &...instructions)
      : instruction_lookup_table{std::monostate{}} {
    ((instruction_lookup_table[instructions.get_opcode()] = std::addressof(instructions)), ...);
  }
};

template <typename t_state, typename t_desc> class virtual_machine {
public:
  t_desc instruction_set;

  struct context {
  private:
    chunk                             m_program_code;
    std::vector<execution_value_type> m_execution_stack;
    t_state                           m_state;
    bool                              halted = false;

  public:
    binary_code_buffer::const_iterator m_ip, m_ip_end;
    context() = default;

    context(const chunk &ch) : m_program_code{ch} {
      m_ip = m_program_code.m_binary_code.begin();
      m_ip_end = m_program_code.m_binary_code.end();
    }

    context(chunk &&ch) : m_program_code{std::move(ch)} {
      m_ip = m_program_code.m_binary_code.begin();
      m_ip_end = m_program_code.m_binary_code.end();
    }

    auto ip() { return m_ip; }

    const auto &code() const { return m_program_code.m_binary_code; }
    const auto &pool() const { return m_program_code.m_constant_pool; }

    auto &at_stack(uint32_t index) { return m_execution_stack.at(index); }

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

    void  push(execution_value_type val) { m_execution_stack.push_back(val); }
    void  halt() { halted = true; }
    bool  is_halted() const { return halted; }
    auto &state() { return m_state; }
  } execution_context;

  auto &ctx() { return execution_context; }

  constexpr virtual_machine(t_desc desc) : instruction_set{desc} {}

  void set_program_code(chunk &&ch) { ctx() = context{std::move(ch)}; }
  void set_program_code(const chunk &ch) { ctx() = context{ch}; }

  void execute_instruction() {
    if (ctx().is_halted()) throw std::runtime_error{"Can't execute, VM is halted"};
    auto current_instruction = instruction_set.instruction_lookup_table[*(ctx().m_ip++)];

    // clang-format off
    std::visit(paracl::utils::visitors{
      [&](std::monostate) {
        ctx().halt();
        throw std::runtime_error{"Unknown opcode"};},
      [&](const auto *instr) {
        auto attr = instr->decode(ctx().m_ip, ctx().m_ip_end).attributes;
        instr->action(ctx(), attr); }}, current_instruction);
    // clang-format on
  }

  void execute() {
    while (!ctx().is_halted()) {
      execute_instruction();
    }
  }
};

} // namespace paracl::bytecode_vm::decl_vm
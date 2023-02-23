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
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string_view>

#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#include "utils/algorithm.hpp"
#include "utils/misc.hpp"
#include "utils/serialization.hpp"

namespace paracl::bytecode_vm::decl_vm {

class vm_error : public std::runtime_error {
public:
  vm_error(std::string err_msg) : std::runtime_error{err_msg} {}
};

using constant_pool_type = std::vector<int>;
using binary_code_buffer_type = std::vector<uint8_t>;

class chunk {
private:
  binary_code_buffer_type m_binary_code;
  constant_pool_type m_constant_pool;

public:
  using value_type = uint8_t;

  chunk() = default;

  chunk(binary_code_buffer_type p_bin, constant_pool_type p_const)
      : m_binary_code{std::move(p_bin)}, m_constant_pool{std::move(p_const)} {}

  template <std::input_iterator binary_it, std::input_iterator constant_it>
  chunk(binary_it bin_begin, binary_it bin_end, constant_it const_begin, constant_it const_end)
      : m_binary_code{bin_begin, bin_end}, m_constant_pool{const_begin, const_end} {}

  template <typename T> void push_value(T val) { utils::write_little_endian(val, std::back_inserter(m_binary_code)); };

  void push_back(value_type code) { m_binary_code.push_back(code); }
  void push_back_signed(int8_t val) { push_back(std::bit_cast<value_type>(val)); }

  void set_constant_pool(constant_pool_type constants) { m_constant_pool = std::move(constants); }

  auto binary_begin() const { return m_binary_code.cbegin(); }
  auto binary_end() const { return m_binary_code.cend(); }
  auto binary_size() const { return m_binary_code.size(); }
  auto binary_data() const { return m_binary_code.data(); }

  auto constants_begin() const { return m_constant_pool.cbegin(); }
  auto constants_end() const { return m_constant_pool.cend(); }
  auto constants_size() const { return m_constant_pool.size(); }

  auto constant_at(std::size_t id) const { return m_constant_pool.at(id); }
};

std::optional<chunk> read_chunk(std::istream &);
void write_chunk(std::ostream &, const chunk &);

template <typename, typename> struct instruction;

using opcode_underlying_type = chunk::value_type;
template <opcode_underlying_type ident, typename... Ts> struct instruction_desc {
  static constexpr auto opcode = ident;
  static constexpr auto binary_size = sizeof(opcode_underlying_type) + (sizeof(Ts) + ... + 0);

  const std::string_view name;
  using attribute_types = std::tuple<Ts...>;

  constexpr auto get_name() const { return name; }
  static constexpr auto get_opcode() { return opcode; }
  static constexpr auto get_size() { return binary_size; }

  constexpr instruction_desc(const char *debug_name) : name{debug_name} {
    if (!debug_name || name[0] == '\0') throw vm_error{"Empty debug names aren't allowed"};
  }

  constexpr auto operator>>(auto action) const { return instruction{*this, action}; }

  template <size_t... I> static void pretty_print(auto &os, const attribute_types &tuple, std::index_sequence<I...>) {
    auto print_list_element = [&os, &tuple](auto i) {
      os << (i == 0 ? "" : ", "), utils::padded_hex_printer(os, std::get<i>(tuple));
    };
    (print_list_element(std::integral_constant<std::size_t, I>()), ...);
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
  t_action action = nullptr;

  constexpr instruction(t_desc p_description, t_action p_action) : description{p_description}, action{p_action} {}
  constexpr auto get_name() const { return description.get_name(); }
  constexpr auto get_opcode() const { return description.get_opcode(); }
  constexpr auto get_size() const { return description.get_size(); }

  template <typename t_stream> t_stream &pretty_print(t_stream &os, const attribute_tuple_type &attr) const {
    description.pretty_print(os, attr);
    return os;
  }

  struct decoded_instruction {
    const instruction *instr;
    attribute_tuple_type attributes;
  };

  template <std::size_t I>
  static std::tuple_element_t<I, attribute_tuple_type> decode_attribute(auto &first, auto last) {
    auto [val, iter] = paracl::utils::read_little_endian<std::tuple_element_t<I, attribute_tuple_type>>(first, last);
    if (!val) throw vm_error{"Decoding error"};
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

template <typename t_desc> class virtual_machine;
using execution_value_type = int;

template <typename t_desc> struct context {
  friend class virtual_machine<t_desc>;

private:
  std::vector<execution_value_type> m_execution_stack;
  binary_code_buffer_type::const_iterator m_ip, m_ip_end;

  chunk m_program_code;
  bool m_halted = false;

public:
  context() = default;

  context(chunk ch) : m_program_code{ch} {
    m_ip = m_program_code.binary_begin();
    m_ip_end = m_program_code.binary_end();
  }

  auto ip() const { return m_ip; }
  auto &at_stack(uint32_t index) & { return m_execution_stack.at(index); }

  void set_ip(uint32_t new_ip) {
    m_ip = m_program_code.binary_begin();
    std::advance(m_ip, new_ip);
  }

  auto pop() {
    if (m_execution_stack.size() == 0) throw vm_error{"Bad stack pop"};
    auto top = m_execution_stack.back();
    m_execution_stack.pop_back();
    return top;
  }

  void push(execution_value_type val) { m_execution_stack.push_back(val); }

  void halt() { m_halted = true; }
  bool is_halted() const { return m_halted; }

  auto constant(uint32_t id) const { return m_program_code.constant_at(id); }
};

template <typename... t_instructions> struct instruction_set_description {
  using instruction_variant_type = std::variant<std::monostate, const t_instructions *...>;
  using instruction_tuple_type = std::tuple<t_instructions...>;

  static constexpr auto max_table_size = std::numeric_limits<opcode_underlying_type>::max() + 1;
  std::array<instruction_variant_type, max_table_size> instruction_lookup_table;

  instruction_set_description(const t_instructions &...instructions) : instruction_lookup_table{std::monostate{}} {
    ((instruction_lookup_table[instructions.get_opcode()] = std::addressof(instructions)), ...);
  }
};

template <typename t_desc> class virtual_machine {
  t_desc instruction_set;
  context<t_desc> m_execution_context;

public:
  virtual_machine(t_desc desc) : instruction_set{desc}, m_execution_context{} {}

  void set_program_code(chunk ch) { m_execution_context = std::move(ch); }

  void execute_instruction() {
    auto &ctx = m_execution_context;

    if (ctx.is_halted()) throw vm_error{"Can't execute, VM is halted"};
    auto current_instruction = instruction_set.instruction_lookup_table[*(m_execution_context.m_ip++)];

    // clang-format off
    std::visit(paracl::utils::visitors{
      [&](std::monostate) {
        m_execution_context.halt();
        throw vm_error{"Unknown opcode"};},
      [&](const auto *instr) {
        auto attr = instr->decode(ctx.m_ip, ctx.m_ip_end).attributes;
        instr->action(ctx, attr); }}, current_instruction);
    // clang-format on
  }

  void execute(bool validate_stack = false) {
    while (!m_execution_context.is_halted()) {
      execute_instruction();
    }

    auto &ctx = m_execution_context;
    if (validate_stack && ctx.m_execution_stack.size() != 0) {
      std::cerr << "Warning: execution finished abnormally: stack size = " << ctx.m_execution_stack.size() << "\n";
    }
  }

  bool is_halted() const { return m_execution_context.is_halted(); }
};

inline std::vector<uint8_t> read_raw_data(std::istream &is) {
  return std::vector<uint8_t>{std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{}};
}

} // namespace paracl::bytecode_vm::decl_vm
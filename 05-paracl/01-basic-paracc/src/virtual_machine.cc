/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <cstdint>
#include <string>

#include <stdexcept>
#include <variant>
#include <vector>

#include "bytecode_vm/chunk.hpp"
#include "bytecode_vm/opcodes.hpp"
#include "bytecode_vm/virtual_machine.hpp"

#include "utils/utils.hpp"

namespace paracl::bytecode_vm {

std::optional<int> virtual_machine::pop() {
  if (m_execution_stack.size() == 0) {
    report_error("Bad stack pop");
    return std::nullopt;
  }

  auto val = m_execution_stack.back();
  m_execution_stack.pop_back();

  return val;
}

// clang-format off

void virtual_machine::push(int val) {
  m_execution_stack.push_back(val);
}

// clang-format on

void virtual_machine::report_error(std::string message) {
  halted = true;

  std::cerr << "Runtime error: " << message << " at: ";
  utils::serialization::padded_hex_printer(std::cerr, std::distance(m_program_code.m_binary_code.begin(), m_ip))
      << "\n";
}

bool virtual_machine::execute_instruction() {
  auto [op, it] = decode_instruction(m_ip, m_ip_end);

  auto report_error = [&](std::string message) {
    halted = true;
    std::cerr << "Runtime error: " << message << " at: ";
    utils::serialization::padded_hex_printer(std::cerr, std::distance(m_program_code.m_binary_code.begin(), m_ip))
        << "\n";
  };

  if (std::holds_alternative<std::monostate>(op)) {
    report_error("Incorrectly encoded instruction");
    return false;
  }

  using enum opcode;

  auto visit_nullary = [&](const nullary_instruction &nullary) -> bool {
    switch (nullary.op) {
    // clang-format off
    case E_ADD_NULLARY: { auto second = pop(); auto first = pop(); if (!first) return false; push(first.value() + second.value()); break; }
    case E_SUB_NULLARY: { auto second = pop(); auto first = pop(); if (!first) return false; push(first.value() - second.value()); break; }
    case E_MUL_NULLARY: { auto second = pop(); auto first = pop(); if (!first) return false; push(first.value() * second.value()); break; }
    case E_DIV_NULLARY: { auto second = pop(); auto first = pop(); if (!first) return false; push(first.value() / second.value()); break; }
    case E_MOD_NULLARY: { auto second = pop(); auto first = pop(); if (!first) return false; push(first.value() % second.value()); break; }
    case E_RETURN_NULLARY: { halted = true; break; }
    case E_POP_NULLARY: { pop(); break; }
    case E_PRINT_NULLARY: { auto top = pop(); if (!top) return false; std::cout << top.value() << "\n"; break; }
    case E_CMP_NULLARY: {
      auto second = pop(); auto first = pop();
      if (second == first) {
        compare_state = compare_result::E_CMP_EQ;
      } else if (first < second) {
        compare_state = compare_result::E_CMP_LS;
      } else {
        compare_state = compare_result::E_CMP_GT;
      }
      break;
    }
      // clang-format on
    }
    std::advance(m_ip, 1); // Magic constants, should be fixed in the future
    return true;
  };

  auto jump_with_condition = [&](unsigned ip, bool guard) -> bool {
    if (!guard) {
      std::advance(m_ip, 5);
      return true;
    }

    if (ip >= m_program_code.m_binary_code.size()) {
      report_error("Incorrect jump");
      return false;
    }

    m_ip = m_program_code.m_binary_code.begin();
    std::advance(m_ip, ip);
    return true;
  };

  auto visit_u32_unary = [&](const unary_u32_instruction &unary) -> bool {
    auto attr = std::get<0>(unary.attributes);
    switch (unary.op) {
    case E_PUSH_CONST_UNARY: {
      if (attr >= m_program_code.m_constant_pool.size()) {
        report_error("Accessing a constant out of range");
        return false;
      }

      push(m_program_code.m_constant_pool.at(attr));
      std::advance(m_ip, 5); // Magic constants, should be fixed in the future
      break;
    }

      using enum compare_result;
    case E_JMP_ABS_UNARY: return jump_with_condition(attr, false);
    case E_JMP_EQ_ABS_UNARY: return jump_with_condition(attr, compare_state == E_CMP_EQ);
    case E_JMP_NE_ABS_UNARY: return jump_with_condition(attr, compare_state != E_CMP_EQ);
    case E_JMP_GT_ABS_UNARY: return jump_with_condition(attr, compare_state == E_CMP_GT);
    case E_JMP_LS_ABS_UNARY: return jump_with_condition(attr, compare_state == E_CMP_LS);
    case E_JMP_GE_ABS_UNARY: return jump_with_condition(attr, compare_state == E_CMP_EQ || compare_state == E_CMP_GT);
    case E_JMP_LE_ABS_UNARY: return jump_with_condition(attr, compare_state == E_CMP_EQ || compare_state == E_CMP_LS);
    }

    return true;
  };

  return std::visit(utils::visitors{visit_nullary, visit_u32_unary, [](std::monostate) { return false; }}, op);
}

} // namespace paracl::bytecode_vm
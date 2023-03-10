/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "frontend/analysis/function_explorer.hpp"

#include "frontend/ast/ast_nodes.hpp"
#include "frontend/ast/node_identifier.hpp"

#include "utils/misc.hpp"

#include <fmt/core.h>

#include <iostream>
#include <string>

namespace paracl::frontend {

void function_explorer::explore(ast::function_definition &ref) {
  auto &&name = ref.name();

  if (name.has_value()) {
    auto &&name_v = name.value();
    auto [ptr, inserted] = m_analytics->m_named.define_function(name_v, &ref);

    if (!inserted) {
      auto report = error_report{
          {fmt::format("Redefinition of function `{}`", name_v), ref.loc()}
      };
      report.add_attachment(error_attachment{fmt::format("[Note] Previously declared here:"), ptr->loc()});
      report_error(report);
      return;
    }

    m_function_stack.push_back({name_v, &ref});
    m_analytics->m_usegraph.insert({std::string{name_v}, &ref});
  }

  else {
    m_analytics->m_anonymous.define_function(&ref);
    std::stringstream ss;
    ss << "anonymous-" << m_analytics->m_anonymous.size();
    m_function_stack.push_back({ss.str(), &ref});
    if (std::find(m_analytics->m_anonymous.begin(), m_analytics->m_anonymous.end(), &ref) ==
        m_analytics->m_anonymous.end()) {
      m_analytics->m_usegraph.insert({ss.str(), &ref});
    }
  }

  apply(ref.body());
  m_function_stack.pop_back();
}

void function_explorer::explore(ast::function_call &ref) {
  auto *found = m_analytics->m_named.lookup(ref.name());
  ref.m_def = found;

  if (!m_function_stack.empty()) {
    auto &&curr_func = m_function_stack.back();
    // Do not create recursive loops. These will get handled separately.
    if (curr_func.m_name != ref.name()) m_analytics->m_usegraph.insert(curr_func, {std::string{ref.name()}, found});
  } else {
    m_analytics->m_usegraph.insert({std::string{ref.name()}, found});
  }

  for (auto *param : ref) {
    assert(param && "Encountered a nullptr in statement block");
    apply(*param);
  }
}

void function_explorer::explore(const ast::function_definition_to_ptr_conv &ref) {
  auto &&def = ref.definition();
  auto name = def.name();
  auto name_v = std::string{};

  if (name.has_value()) {
    name_v = name.value();
  }

  else {
    std::stringstream ss;
    ss << "anonymous-" << m_analytics->m_anonymous.size();
    name_v = ss.str();
  }

  if (!m_function_stack.empty()) {
    auto &&curr_func = m_function_stack.back();
    m_analytics->m_usegraph.insert(curr_func, {name_v, &def});
  }

  else {
    m_analytics->m_usegraph.insert({name_v, &def});
  }

  apply(ref.definition());
}

} // namespace paracl::frontend
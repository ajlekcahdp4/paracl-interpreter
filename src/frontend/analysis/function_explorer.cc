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

#include <iostream>
#include <sstream>
#include <string>

namespace paracl::frontend {

void function_explorer::explore(ast::function_definition &ref) {
  auto &&name = ref.name();

  if (name.has_value()) {
    auto &&name_v = name.value();
    auto [ptr, inserted] = m_analytics->m_named.define_function(name_v, &ref);

    if (!inserted) {
      std::stringstream ss;
      ss << "Redefinition of function "
         << "\"" << name_v << "\""; // add information about previously declared function (in that case 'ptr' will
                                    // be a pointer to already declared function)
      report_error(error_report{
          {ss.str(), ref.loc()}
      });
      return;
    }

    m_function_stack.push_back({name_v, &ref});
    m_analytics->m_callgraph.insert({std::string{name_v}, &ref});
  }

  else {
    m_analytics->m_anonymous.define_function(&ref);
    std::stringstream ss;
    ss << "anonymous-" << m_analytics->m_anonymous.size();
    m_function_stack.push_back({ss.str(), &ref});
    m_analytics->m_callgraph.insert({ss.str(), &ref});
  }

  apply(ref.body());
  m_function_stack.pop_back();
}

void function_explorer::explore(const ast::function_call &ref) {
  if (!m_function_stack.empty()) {
    auto &&curr_func = m_function_stack.back();
    auto found = m_analytics->m_named.lookup(ref.name());
    m_analytics->m_callgraph.insert(curr_func, {std::string{ref.name()}, found});
  }

  for (auto *param : ref) {
    apply(*param);
  }
}

} // namespace paracl::frontend
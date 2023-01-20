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

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer paracl_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL paracl::frontend::parser::symbol_type paracl::frontend::scanner::get_next_token()

#include "bison_paracl_parser.hpp"
#include "location.hpp"

namespace paracl::frontend {
class frontend_driver;

class scanner final : public yyFlexLexer {
private:
  frontend_driver &m_driver;
  position         m_pos;

  frontend_driver &driver() { return m_driver; }

public:
  scanner(frontend_driver &driver) : m_driver{driver} {}

  paracl::frontend::parser::symbol_type get_next_token();
  position                             &pos() { return m_pos; }
};

} // namespace paracl::frontend

#include "frontend_driver.hpp"
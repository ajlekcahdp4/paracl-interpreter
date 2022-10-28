/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

%skeleton "lalr1.cc"
%require "3.8"

%defines

%define api.token.raw
%define api.parser.class { parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { paracl::frontend }

%code requires {
#include <iostream>
#include <string>
#include <vector>

#include "frontend/ast.hpp"

namespace paracl::frontend {
  class scanner;
  class frontend_driver;
}

using namespace paracl::frontend;

}

%code top
{

#include <iostream>
#include <string>

#include "frontend/scanner.hpp"
#include "bison_paracl_parser.hpp"
#include "frontend/frontend_driver.hpp"

static paracl::frontend::parser::symbol_type yylex(paracl::frontend::scanner &p_scanner, paracl::frontend::frontend_driver &p_driver) {
  return p_scanner.get_next_token();
}

}

%lex-param { paracl::frontend::scanner &scanner }
%lex-param { paracl::frontend::frontend_driver &driver }
%parse-param { paracl::frontend::scanner &scanner }
%parse-param { paracl::frontend::frontend_driver &driver }

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

/* Signle letter tokens */
%token LPAREN   "lparen"
%token RPAREN   "rparen"
%token LBRACE   "lbrace"
%token RBRACE   "rbrace"

%token ASSIGN   "assign"

%token COMP_EQ  "=="
%token COMP_NE  "!="
%token COMP_GT  "GT"
%token COMP_LS  "LS"
%token COMP_GE  "GE"
%token COMP_LE  "LE"

%token QMARK    "?"
%token BANG     "!"

%token PLUS       "plus"
%token MINUS      "minus"
%token MULTIPLY   "multiply"
%token DIVIDE     "divide"
%token MODULUS    "modulus"

%token LOGICAL_AND  "and"
%token LOGICAL_OR   "or"

%token SEMICOL  "semicol"
%token EOF 0    "eof"

/* Keywords */
%token WHILE  "while"
%token IF     "if"
%token ELSE   "else"

%token <int> INTEGER_CONSTANT "integer_constant"
%token <std::string> IDENTIFIER "identifier"

%type  <ast::i_expression_node_uptr> primary_expression

%start program

%%

program: additive_expression { std::cout << "This is a test\n"; }

primary_expression: INTEGER_CONSTANT    { $$ = ast::make_constant_expression($1); }
                    | IDENTIFIER        { $$ = ast::make_variable_expression($1); }
                    | QMARK             { $$ = ast::make_read_expression(); }

unary_operator: PLUS | MINUS | BANG { }

unary_expression: unary_operator unary_expression { }
                  | primary_expression 

multiplicative_expression:  multiplicative_expression MULTIPLY unary_expression   { }
                            | multiplicative_expression DIVIDE unary_expression   { }
                            | multiplicative_expression MODULUS unary_expression  { }
                            | unary_expression                                    { }

additive_expression:  additive_expression PLUS multiplicative_expression      { }
                      | additive_expression MINUS multiplicative_expression   { }
                      | multiplicative_expression                             { }
%%

// Bison expects us to provide implementation - otherwise linker complains
void paracl::frontend::parser::error(const std::string &message) {
  std::cout << "Error: " << message << "\n";
}
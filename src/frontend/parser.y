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
%require "3.5"

%defines

%define api.token.raw
%define api.parser.class { parser }
%define api.token.constructor
%define api.value.type variant
%define api.namespace { paracl::frontend }
%define parse.error verbose

%code requires {
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <stdexcept>

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
#include <sstream>

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
%define api.token.prefix {TOKEN_}

%locations
%define api.location.file "location.hpp"

/* Signle letter tokens */
%token LPAREN   "("
%token RPAREN   ")"
%token LBRACE   "{"
%token RBRACE   "}"

%token ASSIGN   "="

%token COMP_EQ  "=="
%token COMP_NE  "!="
%token COMP_GT  ">"
%token COMP_LS  "<"
%token COMP_GE  ">="
%token COMP_LE  "<="
%token EOF 0    "end of file"

%token QMARK    "?"
%token BANG     "!"

%token PLUS       "+"
%token MINUS      "-"
%token MULTIPLY   "*"
%token DIVIDE     "/"
%token MODULUS    "%"

%token LOGICAL_AND  "&&"
%token LOGICAL_OR   "||"

%token SEMICOL  ";"

/* Keywords */
%token WHILE  "while"
%token IF     "if"
%token ELSE   "else"
%token PRINT  "print"

/* Terminals */
%token <int> INTEGER_CONSTANT "constant"
%token <std::string> IDENTIFIER "identifier"

/* Rules that model the AST */
%type <ast::i_ast_node *> primary_expression    
%type <ast::i_ast_node *> multiplicative_expression
%type <ast::i_ast_node *> unary_expression
%type <ast::i_ast_node *> additive_expression
%type <ast::i_ast_node *> comparison_expression
%type <ast::i_ast_node *> equality_expression
%type <ast::i_ast_node *> logical_expression
%type <ast::i_ast_node *> expression

%type <ast::i_ast_node *> assignment_expression_statement

%type <ast::i_ast_node *> print_statement
%type <ast::i_ast_node *> assignment_statement
%type <ast::i_ast_node *> statement_block
%type <ast::i_ast_node *> statement
%type <std::vector<ast::i_ast_node *>> statements
%type <ast::i_ast_node *> if_statement
%type <ast::i_ast_node *> while_statement

%precedence THEN
%precedence ELSE

%start program

%%

program:  statements    { auto ptr = driver.m_ast.make_statement_block(std::move($1), @$); driver.m_ast.set_root_ptr(ptr); }

primary_expression: INTEGER_CONSTANT            { $$ = driver.m_ast.make_constant_expression($1, @$); }
                    | IDENTIFIER                { $$ = driver.m_ast.make_variable_expression($1, @$); }
                    | QMARK                     { $$ = driver.m_ast.make_read_expression(@$); }
                    | LPAREN expression RPAREN  { $$ = $2; }
                    | LPAREN error RPAREN       { auto error = driver.take_error(); $$ = driver.m_ast.make_error_node(error.error_message, error.loc); yyerrok; }

unary_expression: PLUS unary_expression           { $$ = driver.m_ast.make_unary_expression(ast::unary_operation::E_UN_OP_POS, $2, @$); }
                  | MINUS unary_expression        { $$ = driver.m_ast.make_unary_expression(ast::unary_operation::E_UN_OP_NEG, $2, @$); }
                  | BANG unary_expression         { $$ = driver.m_ast.make_unary_expression(ast::unary_operation::E_UN_OP_NOT, $2, @$); }
                  | primary_expression            { $$ = $1; }

multiplicative_expression:  multiplicative_expression MULTIPLY unary_expression   { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_MUL, $1, $3, @$); }
                            | multiplicative_expression DIVIDE unary_expression   { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_DIV, $1, $3, @$); }
                            | multiplicative_expression MODULUS unary_expression  { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_MOD, $1, $3, @$); }
                            | unary_expression                                    { $$ = $1; }

additive_expression:  additive_expression PLUS multiplicative_expression      { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_ADD, $1, $3, @$); }
                      | additive_expression MINUS multiplicative_expression   { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_SUB, $1, $3, @$); }
                      | multiplicative_expression                             { $$ = $1; }

comparison_expression:  comparison_expression COMP_GT additive_expression     { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_GT, $1, $3, @$); }
                        | comparison_expression COMP_LS additive_expression   { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_LS, $1, $3, @$); }
                        | comparison_expression COMP_GE additive_expression   { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_GE, $1, $3, @$); }
                        | comparison_expression COMP_LE additive_expression   { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_LE, $1, $3, @$); }
                        | additive_expression                                 { $$ = $1; }


equality_expression:  equality_expression COMP_EQ comparison_expression   { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_EQ, $1, $3, @$); }
                      | equality_expression COMP_NE comparison_expression { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_NE, $1, $3, @$); }
                      | comparison_expression                             { $$ = $1; }

logical_expression: logical_expression LOGICAL_AND equality_expression    { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_AND, $1, $3, @$); }
                    | logical_expression LOGICAL_OR equality_expression   { $$ = driver.m_ast.make_binary_expression(ast::binary_operation::E_BIN_OP_OR, $1, $3, @$); }
                    | equality_expression                                 { $$ = $1; }

expression: logical_expression                  { $$ = $1; }
            | assignment_expression_statement   { $$ = $1; }

assignment_expression_statement: IDENTIFIER ASSIGN expression             { $$ = driver.m_ast.make_assignment_statement(driver.m_ast.make_variable_expression($1, @1), $3, @$); }

assignment_statement: assignment_expression_statement SEMICOL             { $$ = $1; }

print_statement: PRINT expression SEMICOL { $$ = driver.m_ast.make_print_statement($2, @$); }

statements: statements statement        { $$ = std::move($1); $$.push_back($2); }
            | statements error SEMICOL  { $$ = std::move($1); auto error = driver.take_error(); $$.push_back(driver.m_ast.make_error_node(error.error_message, error.loc)); yyerrok; }
            | statements error EOF      { $$ = std::move($1); auto error = driver.take_error(); $$.push_back(driver.m_ast.make_error_node(error.error_message, error.loc)); yyerrok; }
            | %empty                    { }

statement_block: LBRACE statements RBRACE   { $$ = driver.m_ast.make_statement_block(std::move($2), @$); }

while_statement: WHILE LPAREN expression RPAREN statement { $$ = driver.m_ast.make_while_statement($3, $5, @$); }

if_statement: IF LPAREN expression RPAREN statement %prec THEN        { $$ = driver.m_ast.make_if_statement($3, $5, @$); }
              | IF LPAREN expression RPAREN statement ELSE statement  { $$ = driver.m_ast.make_if_statement($3, $5, $7, @$); }

statement:  assignment_statement  { $$ = $1; }
            | print_statement     { $$ = $1; }
            | statement_block     { $$ = $1; }
            | while_statement     { $$ = $1; }
            | if_statement        { $$ = $1; }

%%

// Custom error reporting function
/* void paracl::frontend::parser::report_syntax_error(const context& ctx) const {
  location loc = ctx.location();

  std::stringstream error_message;
  const auto &lookahead = ctx.lookahead();
  error_message << "Syntax error: Unexpected " << lookahead.name();

  driver.report_error(error_message.str(), loc);
} */

void paracl::frontend::parser::error(const location &loc, const std::string &message) {
  /* When using custom error handling this only gets called when unexpected errors occur, like running out of memory or when an exception gets thrown. 
  Don't know what to do about parser::syntax_error exception for now */

  if (message == "memory exhausted") {
    throw std::runtime_error{"Bison memory exhausted"};
  }

  driver.report_error(message, loc); 
}

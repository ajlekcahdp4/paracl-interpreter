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
%glr-parser
%expect-rr 4

%code requires {
#include "frontend/ast/ast_container.hpp"
#include "frontend/ast/ast_copier.hpp"
#include "frontend/ast/ast_nodes.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cstdlib>
#include <stdexcept>

namespace paracl::frontend {
  class scanner;
  class frontend_driver;
}

}

%code top
{

#include "frontend/scanner.hpp"
#include "bison_paracl_parser.hpp"
#include "frontend/frontend_driver.hpp"

#include <iostream>
#include <string>
#include <sstream>

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
%token COL      ":"
%token COMMA    ","

/* Keywords */
%token WHILE    "while"
%token IF       "if"
%token ELSE     "else"
%token PRINT    "print"
%token FUNC     "func"
%token RET      "return"

/* Terminals */
%token <int> INTEGER_CONSTANT "constant"
%token <std::string> IDENTIFIER "identifier"

/* Rules that model the AST */
%type <ast::i_ast_node *> primary_expression multiplicative_expression unary_expression
additive_expression comparison_expression equality_expression logical_expression expression expression_statement

%type <ast::i_ast_node *> print_statement statement_block statement if_statement while_statement function_def function_call

%type <ast::return_statement *> return_statement
%type <ast::statement_block> statements
%type <ast::assignment_statement *> chainable_assignment chainable_assignment_statement

%type <std::vector<ast::variable_expression>> arglist arglist_or_empty
%type <std::vector<ast::i_ast_node *>> param_list param_list_or_empty

%precedence THEN
%precedence ELSE

%start program

%%

program:  statements    { auto ptr = driver.make_ast_node<ast::statement_block>(std::move($1)); driver.m_ast.set_root_ptr(ptr); }
          | %empty      { }

optional_semicol: %empty {}
                  | SEMICOL {}

primary_expression: INTEGER_CONSTANT            { $$ = driver.make_ast_node<ast::constant_expression>($1, @$); }
                    | IDENTIFIER                { $$ = driver.make_ast_node<ast::variable_expression>($1, @$); }
                    | QMARK                     { $$ = driver.make_ast_node<ast::read_expression>(@$); }
                    | LPAREN expression RPAREN  { $$ = $2; }
                    | LPAREN error RPAREN       { auto error = driver.take_error(); $$ = driver.make_ast_node<ast::error_node>(error.error_message, error.loc); yyerrok; }
                    | statement_block           { $$ = $1; }
                    | function_call             { $$ = $1; }

unary_expression: PLUS unary_expression           { $$ = driver.make_ast_node<ast::unary_expression>(ast::unary_operation::E_UN_OP_POS, *$2, @$); }
                  | MINUS unary_expression        { $$ = driver.make_ast_node<ast::unary_expression>(ast::unary_operation::E_UN_OP_NEG, *$2, @$); }
                  | BANG unary_expression         { $$ = driver.make_ast_node<ast::unary_expression>(ast::unary_operation::E_UN_OP_NOT, *$2, @$); }
                  | primary_expression            { $$ = $1; }

multiplicative_expression:  multiplicative_expression MULTIPLY unary_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_MUL, *$1, *$3, @$); }
                            | multiplicative_expression DIVIDE unary_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_DIV, *$1, *$3, @$); }
                            | multiplicative_expression MODULUS unary_expression  { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_MOD, *$1, *$3, @$); }
                            | unary_expression                                    { $$ = $1; }

additive_expression:  additive_expression PLUS multiplicative_expression      { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_ADD, *$1, *$3, @$); }
                      | additive_expression MINUS multiplicative_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_SUB, *$1, *$3, @$); }
                      | multiplicative_expression                             { $$ = $1; }

comparison_expression:  comparison_expression COMP_GT additive_expression     { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_GT, *$1, *$3, @$); }
                        | comparison_expression COMP_LS additive_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_LS, *$1, *$3, @$); }
                        | comparison_expression COMP_GE additive_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_GE, *$1, *$3, @$); }
                        | comparison_expression COMP_LE additive_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_LE, *$1, *$3, @$); }
                        | additive_expression                                 { $$ = $1; }

equality_expression:  equality_expression COMP_EQ comparison_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_EQ, *$1, *$3, @$); }
                      | equality_expression COMP_NE comparison_expression { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_NE, *$1, *$3, @$); }
                      | comparison_expression                             { $$ = $1; }

logical_expression: logical_expression LOGICAL_AND equality_expression    { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_AND, *$1, *$3, @$); }
                    | logical_expression LOGICAL_OR equality_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_OR, *$1, *$3, @$); }
                    | equality_expression                                 { $$ = $1; }

expression: logical_expression                  { $$ = $1; }
            | chainable_assignment              { $$ = $1; }   

/* Allow statement_block not to be followed by a semicol */
chainable_assignment_statement: IDENTIFIER ASSIGN chainable_assignment_statement    { $$ = $3; auto left = ast::variable_expression{$1, @1}; $$->append_variable(left); }
                      | IDENTIFIER ASSIGN logical_expression SEMICOL                { auto left = ast::variable_expression{$1, @1}; $$ = driver.make_ast_node<ast::assignment_statement>(left, *$3, @$); }
                      | IDENTIFIER ASSIGN statement_block                           { auto left = ast::variable_expression{$1, @1}; $$ = driver.make_ast_node<ast::assignment_statement>(left, *$3, @$); }
                      | IDENTIFIER ASSIGN function_def optional_semicol             { auto left = ast::variable_expression{$1, @1}; $$ = driver.make_ast_node<ast::assignment_statement>(left, *$3, @$); }

chainable_assignment: IDENTIFIER ASSIGN chainable_assignment      { $$ = $3; auto left = ast::variable_expression{$1, @1}; $$->append_variable(left); }
                      | IDENTIFIER ASSIGN logical_expression      { auto left = ast::variable_expression{$1, @1}; $$ = driver.make_ast_node<ast::assignment_statement>(left, *$3, @$); }

print_statement: PRINT expression SEMICOL { $$ = driver.make_ast_node<ast::print_statement>(*$2, @$); }

statements: statements statement        { $$ = std::move($1); $$.append_statement(*$2); }
            | statements error SEMICOL  { $$ = std::move($1); auto error = driver.take_error(); $$.append_statement(*driver.make_ast_node<ast::error_node>(error.error_message, error.loc)); yyerrok; }
            | statements error EOF      { $$ = std::move($1); auto error = driver.take_error(); $$.append_statement(*driver.make_ast_node<ast::error_node>(error.error_message, error.loc)); yyerrok; }
            | statement                 { $$.append_statement(*$1); }

statement_block:  LBRACE statements RBRACE    { $$ = driver.make_ast_node<ast::statement_block>(std::move($2)); }
                  | LBRACE RBRACE             { $$ = driver.make_ast_node<ast::statement_block>(); }
                  | LBRACE error RBRACE       { auto error = driver.take_error(); $$ = driver.make_ast_node<ast::error_node>(error.error_message, error.loc); yyerrok; }

while_statement: WHILE LPAREN expression RPAREN statement { $$ = driver.make_ast_node<ast::while_statement>(*$3, *$5, @$); }

if_statement: IF LPAREN expression RPAREN statement %prec THEN        { $$ = driver.make_ast_node<ast::if_statement>(*$3, *$5, @$); }
              | IF LPAREN expression RPAREN statement ELSE statement  { $$ = driver.make_ast_node<ast::if_statement>(*$3, *$5, *$7, @$); }

expression_statement: logical_expression SEMICOL  { $$ = $1; }

statement:  print_statement                   { $$ = $1; }
            | statement_block                 { $$ = $1; }
            | while_statement                 { $$ = $1; }
            | if_statement                    { $$ = $1; }
            | chainable_assignment_statement  { $$ = $1; }
            | expression_statement            { $$ = $1; }
            | function_def optional_semicol   { $$ = $1; }
            | return_statement                { $$ = $1; }

arglist:  arglist COMMA IDENTIFIER            { $$ = std::move($1); $$.emplace_back($3, @3); }
          | IDENTIFIER                        { $$.emplace_back($1, @1); }

arglist_or_empty: arglist   { $$ = std::move($1); }
                  | %empty  { }

function_def: FUNC LPAREN arglist_or_empty RPAREN statement_block {
                $$ = driver.make_ast_node<ast::function_definition>(std::nullopt, *$5, @$, $3);
              }
              | FUNC LPAREN arglist_or_empty RPAREN COL IDENTIFIER statement_block {
                $$ = driver.make_ast_node<ast::function_definition>($6, *$7, @$, $3);
              }

return_statement: RET expression SEMICOL    { $$ = driver.make_ast_node<ast::return_statement>($2, @$); }
                  | RET SEMICOL             { $$ = driver.make_ast_node<ast::return_statement>(nullptr, @$); }


param_list:  param_list COMMA expression { $$ = std::move($1); $$.push_back($3); }
            | expression { $$.push_back($1); }


param_list_or_empty:  param_list  { $$ = std::move($1); }
                    | %empty      { }


function_call:  IDENTIFIER LPAREN param_list_or_empty RPAREN { $$ = driver.make_ast_node<ast::function_call>($1, @3, $3); } 

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

  if (std::string_view{message} == "memory exhausted") {
    throw std::runtime_error{"Bison memory exhausted"};
  }

  driver.report_error(message, loc); 
}

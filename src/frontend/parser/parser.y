/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

%skeleton "lalr1.cc"
%require "3.5"

%defines

%define api.token.raw
%define api.parser.class
{ parser }

%define api.token.constructor
%define api.value.type variant

%define api.namespace 
{ paracl::frontend }

%define parse.error verbose
%define parse.lac full

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
#include <memory>
#include <stdexcept>

namespace paracl::frontend {
  class scanner;
  class parser_driver;
}

}

%code top {

#include "frontend/scanner.hpp"
#include "bison_paracl_parser.hpp"
#include "frontend/frontend_driver.hpp"

#include <iostream>
#include <string>
#include <sstream>

static paracl::frontend::parser::symbol_type yylex(paracl::frontend::scanner &p_scanner, paracl::frontend::parser_driver &) {
  return p_scanner.get_next_token();
}

}

%lex-param 
{ paracl::frontend::scanner &scanner }

%lex-param 
{ paracl::frontend::parser_driver &driver }

%parse-param 
{ paracl::frontend::scanner &scanner }

%parse-param 
{ paracl::frontend::parser_driver &driver }

%define parse.trace
%define api.token.prefix {TOKEN_}

%locations
%define api.location.file "location.hpp"

/* Signle letter tokens */
%token LPAREN     "("
%token RPAREN     ")"
%token LBRACE     "{"
%token RBRACE     "}"
%token LSQBRACE   "["
%token RSQBRACE   "]"

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
%token INT      "int"
%token VOID     "void"

/* Terminals */
%token <int> INTEGER_CONSTANT "constant"
%token <std::string> IDENTIFIER "identifier"

/* Rules that model the AST */
%type <ast::i_expression *>
  primary_expression
  multiplicative_expression
  unary_expression
  additive_expression
  comparison_expression
  equality_expression
  logical_expression
  expression

%type <ast::i_expression *> 
  value_block
  function_call

%type <ast::i_ast_node *>
  statement

%type <ast::i_statement *>
  if_statement
  while_statement

%type <ast::return_statement *>
  return_statement

%type <std::vector<ast::i_ast_node *>>
  statements_list

%type <ast::statement_block *>
  statement_block


%type <ast::assignment_statement *> 
  chainable_assignment
  chainable_assignment_statement

%type <ast::variable_expression *>
  typified_identifier

%type <ast::subscript *>
  subscript_expression

%type <ast::function_definition *>
  function_def


%type <types::generic_type>
  builtin_type
  function_type
  array_type
  type

%type <std::vector<types::generic_type>> 
  type_list

%type <std::vector<ast::variable_expression>> 
  arglist

%type <std::vector<ast::i_expression *>>
  param_list

%type eof_or_semicol

%precedence THEN
%precedence ELSE

%start program

%%

program:  
  statements_list  { auto ptr = driver.make_ast_node<ast::statement_block>($1, @1); driver.m_ast.set_root_ptr(ptr); }


eof_or_semicol: 
  SEMICOL 
| EOF


optional_semicol: 
  SEMICOL
| %empty


typified_identifier:
  type IDENTIFIER { $$ = driver.make_ast_node<ast::variable_expression>($2, $1, @2); }
| IDENTIFIER      { $$ = driver.make_ast_node<ast::variable_expression>($1, @1); }

subscript_expression:
  IDENTIFIER LSQBRACE expression RSQBRACE { $$ = driver.make_ast_node<ast::subscript>($1, $3, @2); }

primary_expression:
  INTEGER_CONSTANT            { $$ = driver.make_ast_node<ast::constant_expression>($1, @1); }
| IDENTIFIER                  { $$ = driver.make_ast_node<ast::variable_expression>($1, @1); }
| subscript_expression        { $$ = $1; }
| QMARK                       { $$ = driver.make_ast_node<ast::read_expression>(@$); }
| LPAREN expression RPAREN    { $$ = $2; }
| LPAREN error RPAREN         { auto e = driver.take_error(); $$ = driver.make_ast_node<ast::error_node>(e.m_error_message, e.m_loc); yyerrok; }
| value_block                 { $$ = $1; }
| function_call               { $$ = $1; }

unary_expression: 
  PLUS unary_expression         { $$ = driver.make_ast_node<ast::unary_expression>(ast::unary_operation::E_UN_OP_POS, *$2, @$); }
| MINUS unary_expression        { $$ = driver.make_ast_node<ast::unary_expression>(ast::unary_operation::E_UN_OP_NEG, *$2, @$); }
| BANG unary_expression         { $$ = driver.make_ast_node<ast::unary_expression>(ast::unary_operation::E_UN_OP_NOT, *$2, @$); }
| primary_expression            { $$ = $1; }

multiplicative_expression:  
  multiplicative_expression MULTIPLY unary_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_MUL, *$1, *$3, @$); }
| multiplicative_expression DIVIDE unary_expression     { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_DIV, *$1, *$3, @$); }
| multiplicative_expression MODULUS unary_expression    { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_MOD, *$1, *$3, @$); }
| unary_expression                                      { $$ = $1; }

additive_expression:  
  additive_expression PLUS multiplicative_expression    { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_ADD, *$1, *$3, @$); }
| additive_expression MINUS multiplicative_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_SUB, *$1, *$3, @$); }
| multiplicative_expression                             { $$ = $1; }

comparison_expression:  
  comparison_expression COMP_GT additive_expression     { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_GT, *$1, *$3, @$); }
| comparison_expression COMP_LS additive_expression     { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_LS, *$1, *$3, @$); }
| comparison_expression COMP_GE additive_expression     { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_GE, *$1, *$3, @$); }
| comparison_expression COMP_LE additive_expression     { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_LE, *$1, *$3, @$); }
| additive_expression                                   { $$ = $1; }

equality_expression:  
  equality_expression COMP_EQ comparison_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_EQ, *$1, *$3, @$); }
| equality_expression COMP_NE comparison_expression   { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_NE, *$1, *$3, @$); }
| comparison_expression                               { $$ = $1; }

logical_expression:         
  logical_expression LOGICAL_AND equality_expression    { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_AND, *$1, *$3, @$); }
| logical_expression LOGICAL_OR equality_expression     { $$ = driver.make_ast_node<ast::binary_expression>(ast::binary_operation::E_BIN_OP_OR, *$1, *$3, @$); }
| equality_expression                                   { $$ = $1; }

chainable_assignment: 
  IDENTIFIER ASSIGN chainable_assignment  { $$ = $3; auto left = ast::variable_expression{$1, @1}; $$->append(left); } 
| IDENTIFIER ASSIGN logical_expression    { auto left = ast::variable_expression{$1, @1}; $$ = driver.make_ast_node<ast::assignment_statement>(left, *$3, @3); }

chainable_assignment_statement:  
  typified_identifier ASSIGN chainable_assignment SEMICOL   { $$ = $3; $$->append(*$1); }
| typified_identifier ASSIGN logical_expression SEMICOL     { $$ = driver.make_ast_node<ast::assignment_statement>(*$1, *$3, @3); }
| subscript_expression ASSIGN logical_expression SEMICOL { $$ = driver.make_ast_node<ast::assignment_statement>(*$1, *$3, @3); }
| typified_identifier ASSIGN value_block                    { $$ = driver.make_ast_node<ast::assignment_statement>(*$1, *$3, @2); }
| typified_identifier ASSIGN function_def optional_semicol  {
    auto fnc_ptr = driver.make_ast_node<ast::function_definition_to_ptr_conv>(*$3, @3);
    $$ = driver.make_ast_node<ast::assignment_statement>(*$1, *fnc_ptr, @2);
  }

expression:
  logical_expression    { $$ = $1; }
| chainable_assignment  { $$ = $1; } 

return_statement: 
  RET expression SEMICOL  { $$ = driver.make_ast_node<ast::return_statement>($2, @$); }
| RET SEMICOL             { $$ = driver.make_ast_node<ast::return_statement>(nullptr, @$); }

statement:
  PRINT expression SEMICOL         { $$ = driver.make_ast_node<ast::print_statement>(*$2, @$); }
| logical_expression SEMICOL       { $$ = $1; }
| statement_block                  { $$ = $1; }
| chainable_assignment_statement   { $$ = $1; }
| while_statement                  { $$ = $1; }
| if_statement                     { $$ = $1; }
| function_def optional_semicol    { $$ = $1; }
| return_statement                 { $$ = $1; }

statements_list:
  statements_list statement { $$ = std::move($1); $$.push_back($2); }
| statements_list error eof_or_semicol { $$ = std::move($1); auto e = driver.take_error(); $$.push_back(driver.make_ast_node<ast::error_node>(e.m_error_message, e.m_loc)); yyerrok; }
| %empty                    {  }

statement_block:
  LBRACE statements_list RBRACE { $$ = driver.make_ast_node<ast::statement_block>($2, @2); }

value_block:
  LBRACE statements_list RBRACE  { $$ = driver.make_ast_node<ast::value_block>($2, @2); }

if_statement: 
  IF LPAREN expression RPAREN statement %prec THEN      { 
    auto true_block = driver.make_ast_node<ast::statement_block>(std::vector{$5}, @5);
    $$ = driver.make_ast_node<ast::if_statement>(*$3, *true_block, @$);
  }
| IF LPAREN expression RPAREN statement ELSE statement  { 
    auto true_block = driver.make_ast_node<ast::statement_block>(std::vector{$5}, @5);
    auto else_block = driver.make_ast_node<ast::statement_block>(std::vector{$7}, @7);
    $$ = driver.make_ast_node<ast::if_statement>(*$3, *true_block, *else_block, @$);
  }

while_statement:  
  WHILE LPAREN expression RPAREN statement {
    auto block = driver.make_ast_node<ast::statement_block>(std::vector{$5}, @5);
    $$ = driver.make_ast_node<ast::while_statement>(*$3, *block, @$);
  }


/* types and functions */

builtin_type: 
  INT   { $$ = types::type_builtin::type_int; }
| VOID  { $$ = types::type_builtin::type_void; }

type: 
  builtin_type    { $$ = $1; }
| function_type   { $$ = $1; }
| array_type   { $$ = $1; }

array_type:
  type LSQBRACE INTEGER_CONSTANT RSQBRACE { $$ = types::generic_type::make<types::type_array>($1, $3); }

type_list:  
  type_list COMMA type  { $$ = std::move($1); $$.push_back($3); }
| type                  { $$.push_back($1); }
| %empty                {  }

function_type:  
  type FUNC LPAREN type_list RPAREN  { $$ = types::generic_type::make<types::type_composite_function>(std::move($4), $1); }

arglist:  
  arglist COMMA typified_identifier  { $$ = std::move($1); $$.push_back(*$3); }
| typified_identifier                { $$.push_back(*$1); }
| %empty                             {  }

function_def:
  FUNC LPAREN arglist RPAREN value_block                { $$ = driver.make_ast_node<ast::function_definition>(std::nullopt, *$5, @$, $3);} 
| FUNC LPAREN arglist RPAREN COL IDENTIFIER value_block { $$ = driver.make_ast_node<ast::function_definition>($6, *$7, @$, $3); }

param_list: 
  param_list COMMA expression { $$ = std::move($1); $$.push_back($3); }
| expression                  { $$.push_back($1); }
| %empty                      {  }

function_call: 
  IDENTIFIER LPAREN param_list RPAREN { $$ = driver.make_ast_node<ast::function_call>($1, @$, $3); } 


%%

void paracl::frontend::parser::error(const location &loc, const std::string &message) {
  /* When using custom error handling this only gets called when unexpected errors occur, like running out of memory or when an exception gets thrown. 
  Don't know what to do about parser::syntax_error exception for now */

  if (std::string_view{message} == "memory exhausted") {
    throw std::runtime_error{"Bison memory exhausted"};
  }

  driver.report_error(message, loc); 
}

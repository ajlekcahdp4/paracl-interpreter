/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

%{
#include <iostream>
#include <string>

#include "bison_paracl_parser.hpp"
#include "frontend/scanner.hpp"

#define yyterminate() paracl::frontend::parser::make_EOF();

using namespace paracl::frontend;
%}

%option noyywrap nounput noinput nodefault 
%option c++
%option prefix="paracl_"
%option yyclass="scanner"

number      [1-9][0-9]*
identifier  [a-zA-Z_][a-zA-Z_0-9]*

%%

"while"         { std::cout << "WHILE" << "\n"; return parser::make_WHILE(); }
"if"            { std::cout << "IF" << "\n"; return parser::make_IF(); }
"else"          { std::cout << "ELSE" << "\n"; return parser::make_ELSE(); }

"("             { std::cout << "LPAREN" << "\n"; return parser::make_LPAREN(); }
")"             { std::cout << "RPAREN" << "\n"; return parser::make_RPAREN(); }
"{"             { std::cout << "LBRACE" << "\n"; return parser::make_LBRACE(); }
"}"             { std::cout << "RBRACE" << "\n"; return parser::make_RBRACE(); }
";"             { std::cout << "SEMICOL" << "\n"; return parser::make_SEMICOL(); }

"=="            { std::cout << "EQ" << "\n"; return parser::make_COMP_EQ(); }
"!="            { std::cout << "NE" << "\n"; return parser::make_COMP_NE(); }
">"             { std::cout << "GT" << "\n"; return parser::make_COMP_GT(); }
"<"             { std::cout << "LS" << "\n"; return parser::make_COMP_LS(); }
">="            { std::cout << "GE" << "\n"; return parser::make_COMP_GE(); }
"<="            { std::cout << "LE" << "\n"; return parser::make_COMP_LE(); }

"!"             { std::cout << "BANG" << "\n"; return parser::make_BANG(); }
"?"             { std::cout << "QMARK" << "\n"; return parser::make_QMARK(); }

"+"             { std::cout << "PLUS" << "\n"; return parser::make_PLUS(); }
"-"             { std::cout << "MINUS" << "\n"; return parser::make_MINUS(); }
"*"             { std::cout << "MULTIPLY" << "\n"; return parser::make_MULTIPLY(); }
"/"             { std::cout << "DIVIDE" << "\n"; return parser::make_DIVIDE(); }
"%"             { std::cout << "MODULUS" << "\n"; return parser::make_MODULUS(); }

"="             { std::cout << "ASSIGN" << "\n"; return parser::make_ASSIGN(); }

{identifier}    { return parser::make_IDENTIFIER(yytext); }
{number}        { return parser::make_INTEGER_CONSTANT(std::atoi(yytext)); }

[ \t\n\r]       { }
.               { }

<<EOF>>         { return yyterminate(); }

%%

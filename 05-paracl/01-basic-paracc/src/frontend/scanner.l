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

#define YY_USER_ACTION pos() += yyleng;

using namespace paracl::frontend;
%}

%option noyywrap nounput noinput nodefault 
%option c++
%option prefix="paracl_"
%option yyclass="scanner"

number      (0|[1-9][0-9]*)
identifier  [a-zA-Z_][a-zA-Z_0-9]*

%%

"while"         { std::cout << "WHILE" << "\n"; return parser::make_WHILE(location{pos(), pos() + yyleng}); }
"if"            { std::cout << "IF" << "\n"; return parser::make_IF(location{pos(), pos() + yyleng}); }
"else"          { std::cout << "ELSE" << "\n"; return parser::make_ELSE(location{pos(), pos() + yyleng}); }
"print"         { std::cout << "PRINT" << "\n"; return parser::make_PRINT(location{pos(), pos() + yyleng}); }

"("             { std::cout << "LPAREN" << "\n"; return parser::make_LPAREN(location{pos(), pos() + yyleng}); }
")"             { std::cout << "RPAREN" << "\n"; return parser::make_RPAREN(location{pos(), pos() + yyleng}); }
"{"             { std::cout << "LBRACE" << "\n"; return parser::make_LBRACE(location{pos(), pos() + yyleng}); }
"}"             { std::cout << "RBRACE" << "\n"; return parser::make_RBRACE(location{pos(), pos() + yyleng}); }
";"             { std::cout << "SEMICOL" << "\n"; return parser::make_SEMICOL(location{pos(), pos() + yyleng}); }

"=="            { std::cout << "EQ" << "\n"; return parser::make_COMP_EQ(location{pos(), pos() + yyleng}); }
"!="            { std::cout << "NE" << "\n"; return parser::make_COMP_NE(location{pos(), pos() + yyleng}); }
">"             { std::cout << "GT" << "\n"; return parser::make_COMP_GT(location{pos(), pos() + yyleng}); }
"<"             { std::cout << "LS" << "\n"; return parser::make_COMP_LS(location{pos(), pos() + yyleng}); }
">="            { std::cout << "GE" << "\n"; return parser::make_COMP_GE(location{pos(), pos() + yyleng}); }
"<="            { std::cout << "LE" << "\n"; return parser::make_COMP_LE(location{pos(), pos() + yyleng}); }

"!"             { std::cout << "BANG" << "\n"; return parser::make_BANG(location{pos(), pos() + yyleng}); }
"?"             { std::cout << "QMARK" << "\n"; return parser::make_QMARK(location{pos(), pos() + yyleng}); }

"+"             { std::cout << "PLUS" << "\n"; return parser::make_PLUS(location{pos(), pos() + yyleng}); }
"-"             { std::cout << "MINUS" << "\n"; return parser::make_MINUS(location{pos(), pos() + yyleng}); }
"*"             { std::cout << "MULTIPLY" << "\n"; return parser::make_MULTIPLY(location{pos(), pos() + yyleng}); }
"/"             { std::cout << "DIVIDE" << "\n"; return parser::make_DIVIDE(location{pos(), pos() + yyleng}); }
"%"             { std::cout << "MODULUS" << "\n"; return parser::make_MODULUS(location{pos(), pos() + yyleng}); }

"="             { std::cout << "ASSIGN" << "\n"; return parser::make_ASSIGN(location{pos(), pos() + yyleng}); }

{identifier}    { std::cout << yytext << "\n"; return parser::make_IDENTIFIER(yytext, location{pos(), pos() + yyleng}); }
{number}        { std::cout << yytext << "\n"; return parser::make_INTEGER_CONSTANT(std::atoi(yytext), location{pos(), pos() + yyleng}); }

[\n]            { pos().lines(1); }
[ \n]           { }
.               { return parser::make_YYerror(location{pos(), pos() + yyleng}); }

<<EOF>>         { return paracl::frontend::parser::make_EOF(location{pos(), pos() + yyleng}); }

%%

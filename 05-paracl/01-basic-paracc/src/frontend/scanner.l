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
%}

%option noyywrap nounput noinput nodefault 
%option c++
%option prefix="paracl_"
%option yyclass="scanner"

number      [1-9][0-9]*
identifier  [a-zA-Z_][a-zA-Z_0-9]

%%

"while"         { std::cout << "WHILE" << "\n"; return paracl::frontend::parser::make_WHILE(); }
"if"            { std::cout << "IF" << "\n"; return paracl::frontend::parser::make_IF(); }
"else"          { std::cout << "ELSE" << "\n"; return paracl::frontend::parser::make_ELSE(); }

"("             { std::cout << "LPAREN" << "\n"; return paracl::frontend::parser::make_LPAREN(); }
")"             { std::cout << "RPAREN" << "\n"; return paracl::frontend::parser::make_RPAREN(); }
"{"             { std::cout << "LBRACE" << "\n"; return paracl::frontend::parser::make_LBRACE(); }
"}"             { std::cout << "RBRACE" << "\n"; return paracl::frontend::parser::make_RBRACE(); }
";"             { std::cout << "SEMICOL" << "\n"; return paracl::frontend::parser::make_SEMICOL(); }

"=="            { std::cout << "EQ" << "\n"; return paracl::frontend::parser::make_EQ(); }
"!="            { std::cout << "NE" << "\n"; return paracl::frontend::parser::make_NE(); }
">"             { std::cout << "GT" << "\n"; return paracl::frontend::parser::make_GT(); }
"<"             { std::cout << "LS" << "\n"; return paracl::frontend::parser::make_LS(); }
">="            { std::cout << "GE" << "\n"; return paracl::frontend::parser::make_GE(); }
"<="            { std::cout << "LE" << "\n"; return paracl::frontend::parser::make_LE(); }

"!"             { std::cout << "BANG" << "\n"; return paracl::frontend::parser::make_BANG(); }
"?"             { std::cout << "QMARK" << "\n"; return paracl::frontend::parser::make_QMARK(); }

{identifier}    { return paracl::frontend::parser::make_IDENTIFIER(yytext); }
{number}        { return paracl::frontend::parser::make_INTEGER_CONSTANT(std::atoi(yytext)); }

<<EOF>>     { return yyterminate(); }

%%

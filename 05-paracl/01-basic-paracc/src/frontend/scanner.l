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
#include "bison_paracl_parser.hpp"
#include "frontend/scanner.hpp"

#define yyterminate() paracl::frontend::parser::make_EOF();
%}

%option noyywrap
%option nodefault 
%option c++
%option prefix="paracl_"
%option yyclass="scanner"

alpha   [a-zA-Z_]
digit   [0-9]
nonzero [1-9]

number      {nonzero} {digit}*
identifier  {alpha} ({alpha} | {digit})

%%

"("         { std::cout << "LPAREN" << "\n"; return paracl::frontend::parser::make_LPAREN(); }

<<EOF>>     { return yyterminate(); }

%%

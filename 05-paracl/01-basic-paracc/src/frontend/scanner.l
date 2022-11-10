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

using namespace paracl::frontend;
%}

%option noyywrap nounput noinput nodefault 
%option c++
%option prefix="paracl_"
%option yyclass="scanner"

number      (0|[1-9][0-9]*)
identifier  [a-zA-Z_][a-zA-Z_0-9]*

cpp_style_comment "//".*"\n"

%%

"while"         { return parser::make_WHILE(location{pos(), pos() += yyleng}); }
"if"            { return parser::make_IF(location{pos(), pos() += yyleng}); }
"else"          { return parser::make_ELSE(location{pos(), pos() += yyleng}); }
"print"         { return parser::make_PRINT(location{pos(), pos() += yyleng}); }

"("             { return parser::make_LPAREN(location{pos(), pos() += yyleng}); }
")"             { return parser::make_RPAREN(location{pos(), pos() += yyleng}); }
"{"             { return parser::make_LBRACE(location{pos(), pos() += yyleng}); }
"}"             { return parser::make_RBRACE(location{pos(), pos() += yyleng}); }
";"             { return parser::make_SEMICOL(location{pos(), pos() += yyleng}); }

"=="            { return parser::make_COMP_EQ(location{pos(), pos() += yyleng}); }
"!="            { return parser::make_COMP_NE(location{pos(), pos() += yyleng}); }
">"             { return parser::make_COMP_GT(location{pos(), pos() += yyleng}); }
"<"             { return parser::make_COMP_LS(location{pos(), pos() += yyleng}); }
">="            { return parser::make_COMP_GE(location{pos(), pos() += yyleng}); }
"<="            { return parser::make_COMP_LE(location{pos(), pos() += yyleng}); }

"&&"            { return parser::make_LOGICAL_AND(location{pos(), pos() += yyleng}); }
"||"            { return parser::make_LOGICAL_OR(location{pos(), pos() += yyleng}); }

"!"             { return parser::make_BANG(location{pos(), pos() += yyleng}); }
"?"             { return parser::make_QMARK(location{pos(), pos() += yyleng}); }

"+"             { return parser::make_PLUS(location{pos(), pos() += yyleng}); }
"-"             { return parser::make_MINUS(location{pos(), pos() += yyleng}); }
"*"             { return parser::make_MULTIPLY(location{pos(), pos() += yyleng}); }
"/"             { return parser::make_DIVIDE(location{pos(), pos() += yyleng}); }
"%"             { return parser::make_MODULUS(location{pos(), pos() += yyleng}); }

"="             { return parser::make_ASSIGN(location{pos(), pos() += yyleng}); }

{identifier}    { return parser::make_IDENTIFIER(yytext, location{pos(), pos() += yyleng}); }
{number}        { return parser::make_INTEGER_CONSTANT(std::atoi(yytext), location{pos(), pos() += yyleng}); }

[\n]+               { pos().lines(yyleng); }
[ \t]+              { pos().columns(yyleng); }
{cpp_style_comment} { pos().lines(1); }

.               { location loc = {pos(), pos() += yyleng}; std::string msg = "Unknown character"; m_driver.report_error(msg, loc); throw parser::syntax_error{loc, msg}; }

<<EOF>>         { return paracl::frontend::parser::make_EOF(location{pos(), pos() += yyleng}); }

%%

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

%}

%option nodefault noyywrap c++
%option prefix="paracl"
%option yyclass="scanner"

alpha   [a-zA-Z_]
digit   [0-9]
nonzero [1-9]

number      {nonzero} {digit}*
identifier  {alpha} ({alpha} | {digit})

%%

[]

%%

/*****************************************************************************
 *   rules.l - Flex Rules for TIPS Language Subset
 *   MSU CSE 4714/6714 Capstone Project (Fall 2025)
 *   Author: Derek Willis
*****************************************************************************/
%option noyywrap nodefault nounput
%option yylineno

%{
#include "lexer.h"
%}

%%
 /* DEFINE YOUR RULES STARTING HERE */
PROGRAM         { return PROGRAM; }
BEGIN           { return TOK_BEGIN; }
END             { return END; }
WRITE           { return WRITE; }
VAR              { return VAR; }
INTEGER          { return INTEGER; }
REAL             { return REAL; }
READ             { return READ; }

;               { return SEMICOLON; }
\(               { return OPENPAREN; }
\)               { return CLOSEPAREN; }
[ \t\r\n]+      { /* skip whitespace */ }
":="             { return ASSIGN; }
":"              { return COLON; }
[0-9]+[.][0-9]+  { return FLOATLIT; }
[0-9]+           { return INTLIT; }

[A-Z][A-Z0-9]{8,}  {return UNKNOWN;}
[A-Z][A-Z0-9]{0,7}  {return IDENT;}

\'[^'\n]{81,}\'    { return UNKNOWN; }
\'[^'\n]{0,80}\'    { return STRINGLIT; }

 /* End-of-file */
<<EOF>>         { return TOK_EOF; }

.    {return UNKNOWN;}
%%
// ============================================================================
//  parser.cpp — Recursive-descent parser 
// ----------------------------------------------------------------------------
// MSU CSE 4714/6714 Capstone Project (Spring 2026)
// Author: Derek Willis
// ============================================================================

#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <set>
#include "lexer.h"
#include "ast.h"
#include "debug.h"
using namespace std;

// -----------------------------------------------------------------------------
// One-token lookahead
// -----------------------------------------------------------------------------
bool   havePeek = false;
Token  peekTok  = 0;
string peekLex;

inline const char* tname(Token t) { return tokName(t); }

Token peek() 
{
  if (!havePeek) {
    peekTok = yylex();
    if (peekTok == 0) { peekTok = TOK_EOF; peekLex.clear(); }
    else              { peekLex = yytext ? string(yytext) : string(); }
    dbg::line(string("peek: ") + tname(peekTok) + (peekLex.empty() ? "" : " ["+peekLex+"]")
              + " @ line " + to_string(yylineno));
    havePeek = true;
  }
  return peekTok;
}
Token nextTok() 
{
  Token t = peek();
  dbg::line(string("consume: ") + tname(t));
  havePeek = false;
  return t;
}
Token expect(Token want, const char* msg) 
{
  Token got = nextTok();
  if (got != want) {
    dbg::line(string("expect FAIL: wanted ") + tname(want) + ", got " + tname(got));
    ostringstream oss;
    oss << "Parse error (line " << yylineno << "): expected "
        << tname(want) << " — " << msg << ", got " << tname(got)
        << " [" << (yytext ? yytext : "") << "]";
    throw runtime_error(oss.str());
  }
  return got;
}


// TODO: implement parsing functions for each grammar in your language

unique_ptr<Program> parseProgram();
unique_ptr<Block> parseBlock();
unique_ptr<Statement> parseStatement();
unique_ptr<Compound> parseCompound();
unique_ptr<Write> parseWrite();
unique_ptr<Read> parseRead();
unique_ptr<Assign> parseAssign();
unique_ptr<Primary> parsePrimary();
void parseVarDeclSection();

unique_ptr<Read> parseRead() {
  auto read = make_unique<Read>();

  expect(READ, "start of the read statement");
  expect(OPENPAREN, "after read");
  //store identifier
  if(peek() == IDENT) {
    read->target = peekLex;
  }

  if(!symbolTable.contains(read->target)) {
    throw runtime_error("Variable not declared!");
  }

  expect(IDENT, "after parenthesis");
  expect(CLOSEPAREN, "after the identifier");

  return read;
}

unique_ptr<Assign> parseAssign() {
  auto a = make_unique<Assign>();

  a->id = peekLex;
  if (!symbolTable.contains(a->id)) {
    throw runtime_error("Variable not declared!");
  }
  expect(IDENT, "start of assignment");
  expect(ASSIGN, "value comes after");
  
  if(peek() == IDENT) {
    a->value_type = IDENT;
    a->value = peekLex;
    expect(IDENT, "after :=");
  }
  else if(peek() == INTLIT) {
    a->value_type = INTLIT;
    a->value = peekLex;
    expect(INTLIT, "after :=");
  }
  else if(peek() == FLOATLIT) {
    a->value_type = FLOATLIT;
    a->value = peekLex;
    expect(FLOATLIT, "after :=");
  }
  else {
    throw(runtime_error("runtime error!"));
  }

  return a;
}

unique_ptr<Compound> parseCompound() 
{
  auto bob = make_unique<Compound>();
  
  expect(TOK_BEGIN, "start of compound statement");
  bob->stmts.push_back(parseStatement());
  while (peek() == SEMICOLON) {
    expect(SEMICOLON, "between statements in compound statement");
    bob->stmts.push_back(parseStatement());
  }

  expect(END, "end of compound statement");
  return bob;
}


unique_ptr<Statement> parseStatement() 
{
  if(peek() == TOK_BEGIN) {
    return parseCompound();
  }
  else if (peek() == WRITE) {
    return parseWrite();
  }
  else if(peek() == READ) {
    return parseRead();
  }
  else if(peek() == IDENT) {
    return parseAssign();
  }
  else{
    throw(runtime_error("runtime error!"));
  }

}

unique_ptr<Write> parseWrite() {
  auto write = make_unique<Write>();

  expect(WRITE, "start of the write statement");
  expect(OPENPAREN, "after write");
  
  if (peek() == STRINGLIT) {
    write->stringlit_or_ident = peekLex;
    write->type = STRINGLIT;
    expect(STRINGLIT, "after parenthesis");
  }
  else if (peek() == IDENT) {
    write->stringlit_or_ident = peekLex;
    if(!symbolTable.contains(write->stringlit_or_ident)) {
      throw runtime_error("Variable not declared!");
    }
    write->type = IDENT;
    expect(IDENT, "after parenthesis");
  }
  else {
    throw(runtime_error("runtime error!"));
  }
  expect(CLOSEPAREN, "after the string literal");

  return write;
}



void parseVarDeclSection() {
  //consume VAR
  expect(VAR, "start of var declaration section");
  //one or more ident, colon, type, semicolon
  while (peek() == IDENT) {
    string varName = peekLex;
    expect(IDENT, "var name");
    if (symbolTable.contains(varName)) {
      throw runtime_error("Variable already declared!");
    }
    expect(COLON, "after var name");

    if(peek() == INTEGER) {
      symbolTable[varName] = 0; // default value for int
      expect(INTEGER, "after colon");
    }
    else if(peek() == REAL) {
      symbolTable[varName] = 0.0; // default value for double
      expect(REAL, "after colon");
    }
    else {
      throw runtime_error("Expected REAL or INTEGER");
    }
    expect(SEMICOLON, "afteletsr var declaration");

  }
}

unique_ptr<Block> parseBlock(){
  // Start by creating a pointer to the node we need
  auto block = make_unique<Block>();

  if(peek() == VAR) {
    parseVarDeclSection(); 
  }

  // Step through the grammar, storing anything necessary as member variables
  block->compound = parseCompound();
  
  // When done with the grammar, return the pointer to our node
  return block;
}

// -----------------------------------------------------------------------------
// Program → PROGRAM IDENT ';' Block EOF
// -----------------------------------------------------------------------------
unique_ptr<Program> parseProgram() {
  // Make a pointer to the node we need to build
  auto p = make_unique<Program>();
  // Step through the grammar, storing anything necessary as member variables
  expect(PROGRAM, "start of program");
  expect(IDENT, "program name");
  // Store the program name 
  p->name  = peekLex;
  expect(SEMICOLON, "after program name");
  // Store a pointer to the appropriate block
  p->block = parseBlock();
  expect(TOK_EOF, "at end of file (no trailing tokens after program)");
  // Nothing left in the grammar so we return our node pointer
  return p;
}

// -----------------------------------------------------------------------------
// Parser entry point (called by driver)
// -----------------------------------------------------------------------------
// *****************************************************
// To test piece-wise change the pointer type below
unique_ptr<Program> parse()
// *****************************************************
{
  // Reset lookahead state for a fresh parse
  havePeek = false;
  peekTok = 0;
  peekLex.clear();
  
  // *****************************************************
  // To test piece-wise change the parser function you set as your root
  auto root = parseProgram();
  // *****************************************************

  // Ensure no extra tokens remain
  if (peek() != TOK_EOF) {
    ostringstream oss;
    oss << "Parse error (line " << yylineno << "): extra tokens after <program>, got "
        << tname(peekTok) << " [" << (yytext ? yytext : "") << "]";
    throw runtime_error(oss.str());
  }

  return root;
}
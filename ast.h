// =============================================================================
//   ast.h 
// =============================================================================
// MSU CSE 4714/6714 Capstone Project (Spring 2026)
// Author: Derek Willis
// =============================================================================
#pragma once
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include "lexer.h"
#include <map>
#include <variant>
using namespace std;

// -----------------------------------------------------------------------------
// Pretty printer
// -----------------------------------------------------------------------------
inline void ast_line(ostream& os, string prefix, bool last, string label) {
  os << prefix << (last ? "└── " : "├── ") << label << "\n";
}

// TODO: Define and Implement structures to hold each data node!
// Tip: Build with the root of your tree as the lowest struct in this file
//      Implement each higher node in the tree HIGHER up in this file than its children
//      i.e. The root struct at the bottom of the file
//           The leaves of the tree toward the top of the file

inline map<string, variant<int,double>> symbolTable;

struct Block;
struct Program;
struct Statement;
struct Compound;
struct Write;
struct Read;
struct Primary;
struct Assign;


struct Primary
{
  string id;

  virtual ~Primary() = default;
  virtual void print_tree(ostream& os, string prefix, bool last) = 0;
  virtual void interpret(ostream& out) = 0;

};

struct Statement 
{
  virtual ~Statement() = default;
  virtual void print_tree(ostream& os, string prefix, bool last) = 0;
  virtual void interpret(ostream& out) = 0;
};

struct Assign : Statement
{
  //member variables
  string id;
  Token value_type;
  string value;

  //mmember functions
  void print_tree(ostream& os, string prefix, bool last) {
    ast_line(os, prefix, last, "Assign " + id + " :=");
    string child_prefix = prefix + (last ? "    " : "|   ");
    ast_line(os, child_prefix, true, "value: " + value);
}
  void interpret(ostream& out) {
    (void)out;
    auto& lhs = symbolTable[id]; // guaranteed to exist
    visit([&](auto& slot) {
    using T = decay_t<decltype(slot)>; // T is int or double
    if (value_type == INTLIT) {
      slot = static_cast<T>(stoi(value));
    } 
    else if (value_type == FLOATLIT) {
      slot = static_cast<T>(stod(value));
    }
    else { // IDENT
      auto& rhs = symbolTable[value]; // guaranteed to exist
      visit([&](auto r) { slot = static_cast<T>(r); }, rhs);
    }
 }, lhs);
}
};

struct Read : Statement
{
  string target;

  void print_tree(ostream& os, string prefix, bool last) {
    ast_line(os, prefix, last, "Read " + target);
  }
  void interpret(ostream& out) {
    auto it = symbolTable.find(target);
    visit([&](auto& value) { cin >> value; }, it->second);
  };
};


struct Compound : Statement
{
  //member variable
  vector<unique_ptr<Statement>> stmts;
  //member functions
  void print_tree(ostream& os, string prefix, bool last) {
    ast_line(os, prefix, last, "Compound Statement");
    string child_prefix = prefix + (last ? "    " : "|   ");

    for (size_t i = 0; i < stmts.size(); i++) {
      stmts[i]->print_tree(os, child_prefix, i == stmts.size() - 1);
    }
  }

  void interpret(ostream& out)  {
    for (size_t i = 0; i < stmts.size(); i++) {
      stmts[i]->interpret(out);
    }
  }
};

struct Write : Statement
{
  //member variable
  string stringlit_or_ident;
  Token type;

  //member function
  void print_tree(ostream& os, string prefix, bool last) {
    ast_line(os, prefix, last, "Write Statement");
    string child_prefix = prefix + (last ? "    " : "|   ");
    ast_line(os, child_prefix, true, "Output: " + stringlit_or_ident);
  }

  void interpret(ostream& os)  {
    if (type == STRINGLIT) {
        os << stringlit_or_ident << endl; 
    } else { 
        auto it = symbolTable.find(stringlit_or_ident);
        visit([&os](auto&& value){ os << value << endl; }, it->second);
    } 
  }
};


// TODO: Finish this struct for Block
struct Block
{
  // TODO: Declare Any Member Variables
  unique_ptr<Compound> compound;
  // Member Function to Print
  void print_tree(ostream& os, string prefix, bool last){
    // TODO: Finish this function
    ast_line(os, prefix, last, "Block");
    string child_prefix = prefix + (last ? "    " : "|   ");
    if (compound) compound->print_tree(os, child_prefix, true);
  }

  // Member Function to Interpret
  void interpret(ostream& out){
    if (compound) compound->interpret(out);
  }
};

// You do not need to edit this struct, but can if you choose
struct Program
{
  // Member Variables
  string name; 
  unique_ptr<Block> block;

  // Member Functions
  void print_tree(ostream& os)
  {
    cout << "Program\n";
    ast_line(os, "", false, "name: " + name);
    if (block) block->print_tree(os, "", true);
    else 
    { 
      ast_line(os, "", true, "Block"); 
      ast_line(os, "    ", true, "(empty)");
    }
  }

  void interpret(ostream& out) 
  { 
    if (block) block->interpret(out); 
  }
};
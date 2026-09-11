//
// Created by nick on 9/9/26.
//

#ifndef BENCTH_SYMBOLS_H
#define BENCTH_SYMBOLS_H

#include "../structs.h"

//symbols for lookup, stored in a linked list
typedef struct Symbol {

  const Token* token;
  struct Symbol* next;

  int offset; //filled in at codegen for stack lookup
} Symbol;

typedef struct SymbolTable{

  Arena* arena;

  Symbol* head;
  Symbol* tail;

  const struct SymbolTable* outerScope;
} SymbolTable;

SymbolTable* newSymbolTable(const SymbolTable* outer);
int isDefined(const SymbolTable* sT, const Token* token);
Symbol* insertSymbol(SymbolTable* sT, const Token* token);

#endif //BENCTH_SYMBOLS_H
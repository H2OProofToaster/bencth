//
// Created by nick on 9/9/26.
//

#ifndef BENCTH_SYMBOLS_H
#define BENCTH_SYMBOLS_H

#include "../structs.h"

//symbols for lookup, stored in a linked list
typedef struct Symbol {

  Token* token;
  struct Symbol* next;

  int offset; //filled in at codegen for stack lookup
} Symbol;

typedef struct SymbolTable{

  Symbol* head;

  struct SymbolTable* outerScope;
} SymbolTable;

SymbolTable* newSymbolTable(SymbolTable* outer);
void deleteSymbolTable(SymbolTable*);

void insertSymbol(Token* symbol);
void removeSymbol(Token* symbol);

int isSymbol(Token* symbol);

#endif //BENCTH_SYMBOLS_H
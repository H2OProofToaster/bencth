//
// Created by nick on 9/9/26.
//

#include "symbols.h"

static Arena* symbolArena = b_allocArena();

//symbol table control
SymbolTable* newSymbolTable(SymbolTable* outer) {

  SymbolTable* sT = b_alloc(symbolArena, sizeof(SymbolTable));
  sT->head = NULL;
  sT->outerScope = outer;
  return sT;
}

int isDefined(const SymbolTable* sT, const char* name) {

  for (const Symbol* s = sT->head; s != NULL; s = s->next) {

    if (b_strcmp(s->token->literal.b_string, name) == 0) { return 1; }
  }

  return 0;
}

void addSymbol(SymbolTable* sT, Token* token) {

  if (isDefined(sT, token->literal.b_string)) { die("Symbol already exists"); }

  Symbol* s = b_alloc(blindParserArena, sizeof(Symbol));
  s->token = token;
  s->next = sT->head;
  sT->head = s;
}

SymbolTable* newSymbolTable();
void deleteSymbolTable(SymbolTable*);

void insertSymbol(Token* symbol);
void removeSymbol(Token* symbol);

int isSymbol(Token* symbol);
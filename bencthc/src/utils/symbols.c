//
// Created by nick on 9/9/26.
//

#include "symbols.h"
#include "allocator.h"
#include "exit.h"
#include "string.h"

static Arena* symbolArena = NULL;

//symbol table control
SymbolTable* newSymbolTable(const SymbolTable* outer) {

  /* allocate new arena if one doesn't exist*/
  if (symbolArena == NULL) { symbolArena = b_allocArena(); }

  SymbolTable* sT = b_alloc(symbolArena, sizeof(SymbolTable));
  sT->head = NULL;
  sT->tail = NULL;
  sT->outerScope = outer;
  return sT;
}

int isDefined(const SymbolTable* sT, const Token* token) {

  for (const Symbol* s = sT->head; s != NULL; s = s->next) {

    if (b_strcmp(s->token->literal.b_string, token->literal.b_string) == 0) { return 1; }
  }

  return 0;
}

Symbol* insertSymbol(SymbolTable* sT, const Token* token) {

  if (isDefined(sT, token)) { die("Symbol already exists"); }

  Symbol* s = b_alloc(symbolArena, sizeof(Symbol));
  s->token = token;
  s->next = NULL;
  if (sT->head == NULL) { sT->head = s; sT->tail = s; }
  else { sT->tail->next = s; sT->tail = s; }

  return s;
}
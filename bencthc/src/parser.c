//
// Created by nick on 7/13/26.
//

#include "parser.h"

#include "utils/exit.h"
#include "utils/allocator.h"
#include "utils/symbols.h"
#include "utils/string.h"
#include "structs.h"

//forward declarations
static Arena* blindParserArena;
static TokenStream* tokenStream;
static SymbolTable* symbolTable;

//helpers for tokenstream
Token* p_lookN(const size_t n) {

  if (tokenStream->current >= tokenStream->count) { die("end of tokenstream"); }

  return &tokenStream->tokens[tokenStream->current + n - 1];
}

Token* p_peek() {

  return p_lookN(1);
}

Token* p_peekNext() {

  return p_lookN(2);
}

Token* p_advance() {

  Token* t = &tokenStream->tokens[tokenStream->current];
  if (tokenStream->current < tokenStream->count) { tokenStream->current++; }
  return t;
}

int p_check(const TokenType type) {

  return p_peek()->type == type;
}

Token* p_consume(const TokenType type, const char* err) {

  if (p_check(type)) { return p_advance(); }

  die(err);
}

Function* parseFunction(const Scanner* s) {

  Function* function = b_alloc(blindParserArena, sizeof(Function));

  function->returnType = p_consume(INT, "expected integer return type for function definition");
  function->identifier = p_consume(IDENTIFIER, "expected identifier after function return type");
  //rest of function
}

Source* parse(const Scanner* s) {

  //source can only begin with main function definition returning int
  if (p_peek()->type != INT) { die("main function does not return int"); }
  if (p_peekNext()->type != IDENTIFIER) { die("source does not begin wit function definition"); }
  if (b_strcmp(p_peekNext()->literal.b_string, "main") != 0) { die("source does not begin with definition for \"main\""); }

  Source* source = b_alloc(blindParserArena, sizeof(Source));
  source->main = parseFunction(s);
}

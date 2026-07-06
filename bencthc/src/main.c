//
// Created by nick on 6/26/26.
//

#include "bencthc/src/utils/file.h"
#include "bencthc/src/utils/print.h"
#include "bencthc/src/utils/exit.h"
#include "bencthc/src/utils/allocator.h"
#include "bencthc/src/scanner.h"

#include <stdlib.h>

void printLiteral(const Token* t) {

  switch (t->type) {

    case EQUALS: case PLUS: case LEFT_PAREN: case RIGHT_PAREN:
    case LEFT_BRACE: case RIGHT_BRACE: case SEMICOLON:
      b_printChar(t->literal.b_char);
      break;

    case INTEGER:
      b_printInt(t->literal.b_integer);
      break;

    case IDENTIFIER:
    case STRING:
      b_printString(t->literal.b_string);
      break;

    case RETURN:
    case INT:
      b_printString("kms"); // no real literal, print the keyword name
      break;

    case B_EOF:
      b_printString("EOF");
      break;
  }

  b_printChar('\n');
}

void printToken(const Token* t) {

  b_printString("Token type: ");
  b_printString("n/a");
  b_printString(" Literal: ");
  printLiteral(t); //yes this is hacky, idrc tho
  b_printString(" Line: ");
  b_printInt(t->line);
  b_printString("\n");
}

int main() {

  const int f = b_fopen("bencthc/tests/return67.c");
  if (f < 0) { die("could not open file"); }

  Arena* data = b_fread(f);
  if (data == NULL) { die("could not read file"); }

  b_fclose(f);

  b_printString((char*)data + sizeof(Arena));

  free(data);

  const Scanner* scanner = scan("bencthc/tests/return67.c");

  for (size_t count = 0; count < scanner->count; count++) {

    printToken(scanner->tokens + count);
  }

  return 0;
}

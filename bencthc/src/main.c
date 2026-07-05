//
// Created by nick on 6/26/26.
//

#include "bencthc/src/utils/file.h"
#include "bencthc/src/utils/print.h"
#include "bencthc/src/utils/exit.h"
#include "bencthc/src/utils/allocator.h"
#include "bencthc/src/scanner.h"

#include <stdlib.h>

void printToken(const Token* t) {

  b_printString("Token type: ");
  b_printString("n/a");
  b_printString(" Lexeme: ");
  b_printString(t->lexeme);
  b_printString(" Line: ");
  b_printInt(t->line);
  b_printString("\n");
}

int main() {

  const int f = b_fopen("bencthc/tests/hello.txt");

  Arena* data = b_fread(f);

  b_fclose(f);

  b_printString((char*)data);

  free(data);

  const Scanner* scanner = scan("bencthc/tests/hello.txt");

  for (size_t count = 0; count < scanner->count; count++) {

    printToken(scanner->tokens + count);
  }

  return 0;
}

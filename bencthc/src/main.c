//
// Created by nick on 6/26/26.
//

#include "bencthc/src/utils/file.h"
#include "bencthc/src/utils/print.h"
#include "bencthc/src/utils/exit.h"
#include "bencthc/src/utils/allocator.h"
#include "bencthc/src/scanner.h"
#include "bencthc/src/parser.h"
#include "bencthc/src/codeGenerator.h"

#include <stdlib.h>

int main() {

  const int f = b_fopenRead("bencthc/tests/return67.c");
  if (f < 0) { die("could not open file"); }

  Arena* data = b_fread(f);
  if (data == NULL) { die("could not read file"); }

  b_fclose(f);

  b_printString((char*)data + sizeof(Arena));

  free(data);

  const Scanner* scanner = scan("bencthc/tests/return67.c");

  printTokens(scanner);

  const Parser* parser = parse(scanner);

  printProgram(parser->program);

  generate(parser);

  return 0;
}

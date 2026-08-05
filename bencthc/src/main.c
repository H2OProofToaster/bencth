//
// Created by nick on 6/26/26.
//

#include "utils/file.h"
#include "utils/print.h"
#include "utils/exit.h"
#include "utils/allocator.h"
#include "utils/string.h"
#include "structs.h"
#include "scanner.h"
#include "parser.h"
#include "codeGenerator.h"

int main(int argc, char** argv) {

  int f;
  char* name = NULL;

  switch (argc) {

    //no name specified, just use a.s
    case 2:
      f = b_fopenRead(argv[1]);
      break;

    //pass name to codegen
    case 4:
      f = b_fopenRead(argv[1]);

      if ( b_strcmp(argv[2], "-o") != 0 ) { die("missing '-o'"); }
      name = argv[3];
      break;


    default:
      die("incorrect structuring of arguments");
  }

  if (f < 0) { die("could not open file"); }

  Arena* data = b_fread(f);
  if (data == NULL) { die("could not read file"); }

  b_printString((char*)data + sizeof(Arena));
  b_free(data);

  const Scanner* scanner = scan(f);

  const Parser* parser = parse(scanner);

  Arena* codegenArena = generate(parser, name);

  //free arenas
  b_free(scanner->a);
  b_free(parser->a);
  b_free(codegenArena);

  return 0;
}

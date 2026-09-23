//
// Created by nick on 6/26/26.
//

#include "utils/file.h"
#include "utils/print.h"
#include "utils/exit.h"
#include "utils/allocator.h"
#include "utils/string.h"
#include "utils/syscalls/syscall.h"
#include "structs.h"
#include "lexer.h"
#include "blindParser.h"
#include "codeGenerator.h"

int main(const int argc, char** argv) {

  int fd;
  const char* name = NULL;

  switch (argc) {

    //no name specified, just use a.s
    case 2:
      fd = b_fopenRead(argv[1]);
      break;

    //pass name to codegen
    case 4:
      fd = b_fopenRead(argv[1]);

      if ( b_strcmp(argv[2], "-o") != 0 ) { die("missing '-o' flag for output file name"); }
      name = argv[3];
      break;


    default:
      die("incorrect structuring of arguments");
  }

  if (fd < 0) { die("could not open file"); }

  Arena* data = b_fread(fd);
  if (data == NULL) { die("could not read file"); }

  b_printString((char*)data + sizeof(Arena));
  b_free(data);

  //reset offset
  b_syscall_lseek(fd, 0, SEEK_SET);

  const Scanner* scanner = lex(fd);

  const Parser* parser = parse(scanner);

  Arena* codegenArena = generate(parser, name);

  //free arenas
  b_free(scanner->a);
  b_free(parser->a);
  b_free(codegenArena);

  return 0;
}

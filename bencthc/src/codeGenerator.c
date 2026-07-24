//
// Created by nick on 7/19/26.
//

#include "codeGenerator.h"
#include "bencthc/src/utils/file.h"
#include "utils/string.h"
#include "bencthc/src/parser.h"
#include "bencthc/src/utils/allocator.h"
#include "utils/exit.h"

int generate(const Parser* p) {

  const int fd = b_fopenWrite("bencthc/tests/out.s");
  if (fd < 0) { die("could not open output file"); }
  Arena* a = b_allocArena();

  //parser verified that entry point is called main
  b_fwrite(fd, "\t.globl\tmain\n", b_strlen("\t.globl\tmain\n"));
  b_fwrite(fd, "main:\n", b_strlen("main:\n"));
  b_fwrite(fd, "\tmovl\t$", b_strlen("\tmovl\t$"));

  const char* retNum = b_intToString(a, p->program->function->stmts[0]->returnStmt.value->literalExpr.value);
  b_fwrite(fd, retNum, b_strlen(retNum));

  b_fwrite(fd, ", %eax\n", b_strlen(", %eax\n"));
  b_fwrite(fd, "\tret\n", b_strlen("\tret\n"));

  return fd;
}

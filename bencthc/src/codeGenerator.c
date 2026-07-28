//
// Created by nick on 7/19/26.
//

#include "codeGenerator.h"
#include "bencthc/src/utils/file.h"
#include "utils/string.h"
#include "bencthc/src/parser.h"
#include "bencthc/src/utils/allocator.h"
#include "utils/exit.h"

static Arena* codegenArena = NULL;

void writeInstr(const int fd, const char* mnem, const char* ops) {

  //  mnem  ops
  //^^
  b_fwrite(fd, "\t", 1);

  //  mnem  ops
  //  ^^^^
  b_fwrite(fd, mnem, b_strlen(mnem));

  if (ops != NULL) {

    //  mnem  ops
    //      ^^
    b_fwrite(fd, "\t", 1);

    //  mnem  ops
    //        ^^^
    b_fwrite(fd, ops, b_strlen(ops));
  }

  //  mnem  ops
  //           ^
  b_fwrite(fd, "\n", 1);
}

void writeLabel(const int fd, const char* label) {

  //label:
  //^^^^^
  b_fwrite(fd, label, b_strlen(label));

  //label:
  //     ^^
  b_fwrite(fd, ":\n", 2);
}

void writeDirective(const int fd, const char* directive, const char* args) {

  //  .directive  args
  //^^^
  b_fwrite(fd, "\t.", 2);

  //  .directive  args
  //   ^^^^^^^^^
  b_fwrite(fd, directive, b_strlen(directive));

  //  .directive  args
  //            ^^
  b_fwrite(fd, "\t", 1);

  //  .directive  args
  //              ^^^^
  b_fwrite(fd, args, b_strlen(args));

  //  .directive  args
  //                  ^
  b_fwrite(fd, "\n", 1);
}

void writeComment(const int fd, const char* comment) {

  //#comment
  //^
  b_fwrite(fd, "#", 1);

  //#comment
  // ^^^^^^^
  b_fwrite(fd, comment, b_strlen(comment));

  //#comment
  //        ^
  b_fwrite(fd, "\n", 1);
}

char* generateExpr(const Expr* e) {

  switch (e->type) {

    case EXPR_BINARY:
      die("binary expressions not yet supported"); //return generateBinary(e->binary.left, e->binary.operator, e->binary.right);
      break;

    case EXPR_UNARY:
      die("unary expressions not yet supported"); //return generateUnary(e->unary.op, e->unary.operand);
      break;

    case EXPR_LITERAL:
      return b_concat(codegenArena, "$\0", b_intToString(codegenArena, e->literal.value));
      break;

    case EXPR_VARIABLE:
      die("variable expressions not yet supported"); //ts is going to fry me
      break;

    case EXPR_GROUPING:
      return generateExpr(e->grouping.inner);
      break;

    case EXPR_ASSIGN:
      die("assignment expressions not yet supported"); //ts is also going to fry me
      break;

    default:
      return NULL;
  }
}

void generateStatements(const int fd, const Function* f) {

  for (size_t i = 0; i < f->count; i++) {

    switch (f->stmts[i]->type) {

      case STMT_RETURN:
        writeInstr(fd, "movl", generateExpr(f->stmts[i]->returnStmt.expr));
        writeInstr(fd, "ret", NULL);
        break;

      case STMT_EXPR:
        //ex: foo = expr
        //1. need to find where foo is on stack
        //2. evaluate expression
        //3. check if that still fits in the stack there
        //4. write to stack (either in new or old spot based on 3)
        die("expressions not yet supported");
        break;

      case STMT_DECL:
        //ex: identifier bar = expr
        //0. identifier (should) be already checked for dupes
        //1. evaluate expression
        //2. push onto stack
        die("declarations not yet supported");
        break;

      default:
        break;
    }
  }
}

Arena* generate(const Parser* p) {

  const int fd = b_fopenWrite("bencthc/tests/out.s");
  if (fd < 0) { die("could not open output file"); }
  codegenArena = b_allocArena();

  //parser verified that entry point is called main
  writeDirective(fd, "globl", "main");
  writeLabel(fd, "main");

  generateStatements(fd, p->program->function);

  return codegenArena;
}

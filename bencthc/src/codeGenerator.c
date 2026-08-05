//
// Created by nick on 7/19/26.
//

#include "codeGenerator.h"

#include "bencthc/src/utils/file.h"
#include "bencthc/src/utils/string.h"
#include "bencthc/src/utils/allocator.h"
#include "bencthc/src/utils/exit.h"
#include "bencthc/src/structs.h"

char* generateExpr(const Expr* e);

static Arena* codegenArena = NULL;
static int fd = -1;
static int rspOffset = 0;

void writeInstr(const char* mnem, const int count, const char** ops) {

  //  mnem  ops
  //^^
  b_fwrite(fd, "\t", 1);

  //  mnem  ops
  //  ^^^^
  b_fwrite(fd, mnem, b_strlen(mnem));

  for (int i = 0; i < count; i++) {

    //write space after first op
    if (i == 0) {

      //  mnem  ops
      //      ^^
      b_fwrite(fd, " ", 1);
    }

    //write comma space after other ops
    else {

      //  mnem  op1, op2
      //           ^^
      b_fwrite(fd, ", ", 2);
    }

    //  mnem  ops
    //        ^^^
    b_fwrite(fd, ops[i], b_strlen(ops[i]));
  }

  //  mnem  ops
  //           ^
  b_fwrite(fd, "\n", 1);
}

void writeLabel(const char* label) {

  //label:
  //^^^^^
  b_fwrite(fd, label, b_strlen(label));

  //label:
  //     ^^
  b_fwrite(fd, ":\n", 2);
}

void writeDirective(const char* directive, const char* args) {

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

void writeComment(const char* comment) {

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

int stackAlloc(const int size) {

  const char* ops[] = { b_concat(codegenArena, "$", b_intToString(codegenArena, size)), "%rsp" };
  writeInstr("sub", 2, ops);
  rspOffset += size;
  return rspOffset;
}

Symbol* getSymbol(const SymbolTable* sT, const char* name) {

  for (Symbol* s = sT->head; s != NULL; s = s->next) {

    if (s->token->literal.b_string == name) { return s; }
  }

  if (sT->outerScope != NULL) {

    Symbol* s = getSymbol(sT->outerScope, name);
    if (s != NULL) { return s; }
  }

  //no outer scope
  return NULL;
}

char* getStackOffset(const Symbol* s) {

  return b_concat(codegenArena,
                  b_concat(codegenArena, "-", b_intToString(codegenArena, s->offset)), //-offset
                  "(%rbp)"); //-offset(%rbp)
}

char* generateBinary(const Expr* left, const TokenType op, const Expr* right) {

  switch (op) {

    case PLUS: {

      //move left into rax
      const char* ops1[] = { generateExpr(left), "%rax" };
      writeInstr("mov", 2, ops1);

      //add right to rax
      const char* ops2[] = { "%rax", generateExpr(right) };
      writeInstr("add", 2, ops2);

      return "%rax";
    }

    case MINUS: {

      //move left into rax
      const char* ops1[] = { generateExpr(left), "%rax" };
      writeInstr("mov", 2, ops1);

      //sub right from rax
      const char* ops2[] = { "%rax", generateExpr(right) };
      writeInstr("sub", 2, ops2);

      return "%rax";
    }

    case STAR: {

      //move left into rax
      const char* ops1[] = { generateExpr(left), "%rax" };
      writeInstr("mov", 2, ops1);

      //mult rax by right
      const char* ops2[] = { "%rax", generateExpr(right) };
      writeInstr("imul", 2, ops2);

      return "%rax";
    }

    case FORWARD_SLASH: {

      //move left into rax
      const char* ops1[] = { generateExpr(left), "%rax" };
      writeInstr("mov", 2, ops1);

      //zero rdx
      writeInstr("cqto", 0, NULL);

      //divide by right
      const char* ops2[] = { generateExpr(right) };
      writeInstr("idiv", 1, ops2);

      return "%rax";
    }

    default:
      die("expected binary operator");
      return NULL;

  }
}

char* generateExpr(const Expr* e) {

  switch (e->type) {

    case EXPR_BINARY:
      return generateBinary(e->binary.left, e->binary.operator, e->binary.right);

    case EXPR_UNARY:
      die("unary expressions not yet supported"); //return generateUnary(e->unary.op, e->unary.operand);
      return NULL;

    case EXPR_LITERAL:
      return b_concat(codegenArena, "$", b_intToString(codegenArena, e->literal.value));

    case EXPR_VARIABLE:
      die("variable expressions not yet supported"); //ts is going to fry me
      return NULL;

    case EXPR_GROUPING:
      return generateExpr(e->grouping.inner);

    case EXPR_ASSIGN:
      die("assignment expressions not yet supported"); //ts is also going to fry me
      return NULL;

    default:
      die("expected expression");
      return NULL;
  }
}

void generateStatements(const Function* f) {

  for (size_t i = 0; i < f->count; i++) {

    switch (f->stmts[i]->type) {

      case STMT_RETURN: {

        const char* ops[] = { generateExpr(f->stmts[i]->returnStmt.expr), "%rax" };
        writeInstr("mov", 2, ops);
        writeInstr("ret", 0, NULL);
        break;
      }

      case STMT_EXPR: {

        //ex: foo = expr
        //1. need to find where foo is on stack
        //2. evaluate expression
        //3. check if that still fits in the stack there
        //4. write to stack (either in new or old spot based on 3)
        Symbol* s = getSymbol(f->symbolTable, f->stmts[i]->exprStmt.identifier);
        if (s == NULL) { die("undefined symbol"); }

        const char* ops[] = { generateExpr(f->stmts[i]->declStmt.expr), getStackOffset(s) };
        writeInstr("mov", 2, ops);
        break;
      }

      case STMT_DECL: {

        //ex: identifier bar = expr
        //0. identifier (should (it is)) be already checked for dupes
        //1. evaluate expression
        //2. push onto stack
        //3. store offset in symbol
        Symbol* s = getSymbol(f->symbolTable, f->stmts[i]->declStmt.identifier);
        if (s == NULL) { die("undefined symbol"); }
        s->offset = stackAlloc(4);

        if (f->stmts[i]->declStmt.expr != NULL) {

          const char* ops[] = { generateExpr(f->stmts[i]->declStmt.expr), getStackOffset(s) };
          writeInstr("mov", 2, ops);
        }
        break;
      }

      default:
        die("expected return statement");
    }
  }
}

Arena* generate(const Parser* p) {

  fd = b_fopenWrite("bencthc/tests/out.s");
  if (fd < 0) { die("could not open output file"); }
  codegenArena = b_allocArena();

  //prologue
  const char* ops1[] = { "%rbp" };
  writeInstr("push", 1, ops1);

  const char* ops2[] = { "%rsp", "%rbp" };
  writeInstr("mov", 2, ops2);

  //parser verified that entry point is called main
  writeDirective("global", "main");
  writeLabel("main");

  generateStatements(p->program->function);

  //epilogue
  writeInstr("pop", 1, ops1);
  writeInstr("ret", 0, NULL);

  return codegenArena;
}

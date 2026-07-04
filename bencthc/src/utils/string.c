//
// Created by nick on 7/2/26.
//

#include "string.h"
#include "bencthc/src/utils/allocator.h"
#include "bencthc/src/scanner.h"

void b_lexemeToLiteral(Token* t) {

  //allocate space in scanner arena
  //hypothetically done right after the token struct,
  //as it's allocated during scanning
  t->literal.b_string = b_alloc(t->a, t->length + 1);

  for (size_t i = 0; i < t->length; i++) {

    t->literal.b_string[i] = t->lexeme[i];
  }

  t->literal.b_string[t->length] = '\0';
}
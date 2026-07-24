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

size_t b_strlen(const char* data) {

  size_t count = 0;
  while (*data++ != '\0') { count++; }
  return count;
}

int b_strcmp(const char* a, const char* b) {

  if (b_strlen(a) != b_strlen(b)) { return 1; }

  while (*a != '\0') {

    if (*a != *b) { return 1; }
    a++;
    b++;
  }

  return 0;
}

char* b_intToString(Arena* a, int i) {

  if (i == 0) {

    char* ret = b_alloc(a, 2);
    ret[0] = '0';
    ret[1] = '\0';
    return ret;
  }

  int negative = i < 0;
  i = negative ? -i : i;

  char temp[32];
  int n = 0;

  while (i > 0) {

    temp[n++] = (char)('0' + i % 10);
    i /= 10;
  }

  if (negative) { temp[n++] = '-'; }

  char* ret = b_alloc(a, n + 1);
  for (int j = 0; j < n; j++) { ret[j] = temp[n - 1 - j]; }
  ret[n] = '\0';
  return ret;
}
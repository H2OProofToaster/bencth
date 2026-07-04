//
// Created by nick on 6/27/26.
//

#include "bencthc/src/utils/file.h"
#include "bencthc/src/utils/exit.h"
#include "bencthc/src/utils/print.h"
#include "bencthc/src/scanner.h"
#include "utils/string.h"

int isDigit(const char c) { return c >= '0' && c <= '9'; }

int isAlpha(const char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

int isAlphaNumeric(const char c) { return isAlpha(c) || isDigit(c); }

//eat one char
char* advance(Scanner* s) { return s->curr++; }

char peek(const Scanner* s) { return s->curr[1]; }

Token* addToken(Scanner* s, const enum tokenType type, char* lexeme, const int line) {

  Token* t = &s->tokens[s->count++];
  t->type = type;
  t->lexeme = lexeme;
  t->line = line;

  return t;
}

void advanceNumber(Scanner* s, char* c) {

  Token* t = addToken(s, INTEGER, c, s->line);
  t->literal.b_integer = *c - '0';
  t->length = 1;

  while (isDigit(peek(s))) {

    t->literal.b_integer = t->literal.b_integer * 10 + ( *advance(s) - '0' );
    t->length++;
  }
}

void advanceIdentifier(Scanner* s, char* c) {

  Token* t = addToken(s, IDENTIFIER, c, s->line);

  while (isAlphaNumeric(peek(s))) {

    advance(s);
    t->length++;
  }

  b_lexemeToLiteral(t);
}

void scanToken(Scanner* s) {

  char* c = advance(s);
  Token* t;

  switch (*c) {

    //single characters
    case '=': t = addToken(s, EQUALS, c, s->line); t->literal.b_char = '='; break;
    case '+': t = addToken(s, PLUS, c, s->line); t->literal.b_char = '+'; break;
    case '(': t = addToken(s, LEFT_PAREN, c, s->line); t->literal.b_char = '('; break;
    case ')': t = addToken(s, RIGHT_PAREN, c, s->line); t->literal.b_char = ')'; break;
    case '{': t = addToken(s, LEFT_BRACE, c, s->line); t->literal.b_char = '{'; break;
    case '}': t = addToken(s, RIGHT_BRACE, c, s->line); t->literal.b_char = '}'; break;
    case ';': t = addToken(s, SEMICOLON, c, s->line); t->literal.b_char = ';'; break;

    //single OR double characters
    case '/': while (*c != '\n' && s->curr > s->source + s->length) c = advance(s); break; //only comments rn

    //keywords

    //ignore whitespace
    case ' ':
    case '\r':
    case '\t':
      break;

    //newline
    case '\n' : s->line++; break;

    //compile time cases
    default:

      //literals
      //integers
      if (isDigit(*c)) { advanceNumber(s, c); break; }

      //identifier and keywords
      if (isAlpha(*c)) { advanceIdentifier(s, c); break; }

      //error
      else { b_printString("unexpected character; continuing"); break; }
  }
}

Scanner* scan(const char* sourcePath) {

  Arena* a = b_allocArena();

  Scanner* s = b_alloc(a, sizeof(Scanner));
  s->a = a;

  //read from file
  const int f =  b_fopen(sourcePath);
  s->length = b_fsize(f);
  s->source = (char*)b_fread(f);

  //allocate token space with upper bound of s->length
  s->tokens = b_alloc(a, sizeof(Token) * s->length);

  //initialize scanning loop
  s->curr = s->source;
  while (s->curr < s->source + s->length) {

    scanToken(s);
  }

  addToken(s, EOF,"", s->line);

  return s;
}
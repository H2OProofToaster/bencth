//
// Created by nick on 6/27/26.
//

#include "bencthc/src/utils/file.h"
#include "bencthc/src/utils/exit.h"
#include "bencthc/src/utils/print.h"
#include "bencthc/src/scanner.h"

#include "utils/memory.h"
#include "utils/string.h"

int isDigit(const char c) { return c >= '0' && c <= '9'; }

int isAlpha(const char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

int isAlphaNumeric(const char c) { return isAlpha(c) || isDigit(c); }

int isAtEnd(const Scanner* s) { return s->curr >= s->source + s->length; }

//eat one char
static char* advance(Scanner* s) { return s->curr++; }

static char peek(const Scanner* s) { return s->curr[0]; }

char peekNext(const Scanner* s) { return s->curr[1]; }

Token* addToken(Scanner* s, const enum TokenType type, char* lexeme, const int line) {

  Token* t = &s->tokens[s->count++];
  t->a = s->a;
  t->type = type;
  t->lexeme = lexeme;
  t->line = line;

  return t;
}

//check if an identifier is a reserved keyword
//first does length check (idea gratefully from calude :)
//then uses (to be implemented) memory comparison
enum TokenType checkKeyword(const Token* t) {

  for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {

    if (t->length == keywords[i].length &&
        b_memcmp(t->lexeme, keywords[i].keyword, t->length) == 0) {

      return keywords[i].type;
    }
  }

  return IDENTIFIER;
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
  t->length = 1;

  while (isAlphaNumeric(peek(s))) {

    advance(s);
    t->length++;
  }

  //check for reserved keyword
  t->type = checkKeyword(t);

  if (t->type == IDENTIFIER) { b_lexemeToLiteral(t); }
}

void advanceString(Scanner* s, char* c) {

  Token* t = addToken(s, STRING, c + 1, s->line);
  t->length = 0;

  //allocate space for literal
  //upper bound of rest of source (ik that seems extra)
  char* literal = b_alloc(s->a, (s->source + s->length) - s->curr + 1);
  size_t length = 0;

  while (peek(s) != '"' && !isAtEnd(s)) {

    //check for splice
    if (peek(s) == '\\' && peekNext(s) == '\n') {

      advance(s); //eat backslash
      advance(s); //eat newline
      s->line++;
      continue; //don't count splice in length
    }

    literal[length++] = *advance(s);
  }

  //unterminated string
  if (isAtEnd(s)) { die("unterminated string"); }
  advance(s); //eat closing "

  literal[length] = '\0';
  t->length = length;
  t->literal.b_string = literal;
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

    //only comments rn
    case '/':
      if (peek(s) == '/') {

        while (peek(s) != '\n' && !isAtEnd(s)) { advance(s); }
        //continue because c is now pointing at the newline
        //just let the '\n' case handle it to increment s->line
      }
      else {

        //division
      }
      break;

    //string literals
    case '"': advanceString(s, c); break;

    //ignore whitespace
    case ' ':
    case '\r':
    case '\t':
      break;

    //newline
    case '\n' : s->line++; break;

    //compile time cases
    default:

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
  s->line = 1;

  //read from file
  const int f =  b_fopen(sourcePath);
  s->length = b_fsize(f);
  s->source = (char*)b_fread(f) + sizeof(Arena);

  //allocate token space with upper bound of s->length
  s->tokens = b_alloc(a, sizeof(Token) * (s->length + 1));

  //initialize scanning loop
  s->curr = s->source;
  while (s->curr < s->source + s->length) {

    scanToken(s);
  }

  addToken(s, B_EOF, "", s->line);

  return s;
}

void printLiteral(const Token* t) {

  switch (t->type) {

    case EQUALS: case PLUS: case LEFT_PAREN: case RIGHT_PAREN:
    case LEFT_BRACE: case RIGHT_BRACE: case SEMICOLON:
      b_printChar(t->literal.b_char);
      break;

    case INTEGER:
      b_printInt(t->literal.b_integer);
      break;

    case IDENTIFIER:
    case STRING:
      b_printString(t->literal.b_string);
      break;

    case RETURN:
    case INT:
      b_printString("kms"); // no real literal, print the keyword name
      break;

    case B_EOF:
      b_printString("EOF");
      break;
  }

  b_printChar('\n');
}

void printToken(const Token* t) {

  b_printString("Token type: ");
  b_printString("n/a");
  b_printString(" Literal: ");
  printLiteral(t); //yes this is hacky, idrc tho
  b_printString(" Line: ");
  b_printInt(t->line);
  b_printString("\n");
}

void printTokens(const Scanner* s) {

  for (size_t count = 0; count < s->count; count++) {

    printToken(s->tokens + count);
  }
}
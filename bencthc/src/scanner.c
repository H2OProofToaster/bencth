//
// Created by nick on 6/27/26.
//

#include "scanner.h"

#include "utils/file.h"
#include "utils/exit.h"
#include "utils/print.h"
#include "utils/memory.h"
#include "utils/string.h"

int isDigit(const char c) { return c >= '0' && c <= '9'; }

int isAlpha(const char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

int isAlphaNumeric(const char c) { return isAlpha(c) || isDigit(c); }

int isAtEnd(const Scanner* s) { return s->curr >= s->source + s->length; }

//eat one char
char* s_advance(Scanner* s) { return s->curr++; }

char s_peek(const Scanner* s) { return s->curr[0]; }

char s_peekNext(const Scanner* s) { return s->curr[1]; }

Token* addToken(Scanner* s) {

  Token* t = &s->tokens[s->count++];
  t->a = s->a;
  t->line = s->line;

  return t;
}

//check if an identifier is a reserved keyword
//first does length check (idea gratefully from calude :)
//then uses (to be cut from libc) memcmp
//pulls from a static list of keywords in structs.h
//(that's kinda wonky and I might fix eventually)
void checkKeyword(Token* t) {

  for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {

    if (t->length == keywords[i].length &&
        b_memcmp(t->lexeme, keywords[i].keyword, t->length) == 0) {

      t->length = keywords[i].length;
      t->type = keywords[i].type;
    }
  }
}

//these consume functions wrap all the token-specific work to do
//(I didn't want to clutter the switch in scanToken)
//they should be pretty self-explanatory
//don't hate on the spacing here, it's ORGANIZED

void consumeSingle(Scanner* s, const TokenType type, char* c) {

  Token* t = addToken(s);

  t->type = type;

  t->lexeme = c;

  t->length = 1;

  t->literal.b_char = *c;
}

void consumeDouble(Scanner* s, const TokenType type, char* c) {

  Token* t = addToken(s);

  t->type = type;

  t->lexeme = c;

  t->length = 2;

  t->literal.b_string = b_alloc(s->a, 3);
  t->literal.b_string[0] = *c;
  t->literal.b_string[1] = *s_advance(s);
  t->literal.b_string[2] = '\0';
}

void consumeTriple(Scanner* s, const TokenType type, char* c) {

  Token* t = addToken(s);

  t->type = type;

  t->lexeme = c;

  t->length = 3;

  t->literal.b_string = b_alloc(s->a, 4);
  t->literal.b_string[0] = *c;
  t->literal.b_string[1] = *s_advance(s);
  t->literal.b_string[2] = *s_advance(s);
  t->literal.b_string[3] = '\0';
}

void consumeNumber(Scanner* s, char* c) {

  Token* t = addToken(s);

  t->type = INTEGER;

  t->lexeme = c;

  t->length = 1;

  t->literal.b_integer = *c - '0';
  while (isDigit(s_peek(s))) {

    t->literal.b_integer = t->literal.b_integer * 10 + ( *s_advance(s) - '0' );
    t->length++;
  }
}

void consumeIdentifier(Scanner* s, char* c) {

  Token* t = addToken(s);

  t->type = IDENTIFIER;

  t->lexeme = c;

  t->length = 1;
  while (isAlphaNumeric(s_peek(s))) {

    s_advance(s);
    t->length++;
  }

  //check for reserved keyword;
  //changes type to that if so
  checkKeyword(t);
  b_lexemeToLiteral(t);
}

void consumeString(Scanner* s, char* c) {

  Token* t = addToken(s);

  t->type = STRING;

  //skip over '"' that c is pointing to
  t->lexeme = c + 1;

  //0 b/c the start (the '"') isn't included in the lexeme or literal
  t->length = 0;

  //allocate space for literal
  //upper bound as the rest of source to be safe (ik that seems extra)
  t->literal.b_string = b_alloc(s->a, s->source + s->length - s->curr + 1);
  while (s_peek(s) != '"' && !isAtEnd(s)) {

    //check for splice
    if (s_peek(s) == '\\' && s_peekNext(s) == '\n') {

      s_advance(s); //eat backslash
      s_advance(s); //eat newline
      s->line++;
      continue; //don't count splice in length
    }

    t->literal.b_string[t->length++] = *s_advance(s);
  }
  //unterminated string
  if (isAtEnd(s)) { die("unterminated string"); }
  s_advance(s); //eat closing "
  t->literal.b_string[t->length] = '\0';
}

void scanToken(Scanner* s) {

  char* c = s_advance(s);

  switch (*c) {

    //single characters
    case '=': consumeSingle(s, EQUALS, c); break;
    case '+': consumeSingle(s, PLUS, c); break;
    case '(': consumeSingle(s, LEFT_PAREN, c); break;
    case ')': consumeSingle(s, RIGHT_PAREN, c); break;
    case '{': consumeSingle(s, LEFT_BRACE, c); break;
    case '}': consumeSingle(s, RIGHT_BRACE, c); break;
    case ';': consumeSingle(s, SEMICOLON, c); break;
    case '-': consumeSingle(s, MINUS, c); break;
    case '*': consumeSingle(s, STAR, c); break;
    case '!': consumeSingle(s, EXCLAMATION, c); break;
    case '~': consumeSingle(s, TILDE, c); break;
    case '%': consumeSingle(s, PERCENT, c); break;
    case '&': consumeSingle(s, AMPERSAND, c); break;
    case ',': consumeSingle(s, COMMA, c); break;

    //single OR double characters

    //only comments rn
    //added division support 7/27/26
    case '/':

      //comment, b/c next is also a '/'
      if (s_peek(s) == '/') {

        while (s_peek(s) != '\n' && !isAtEnd(s)) { c = s_advance(s); }
        //continue because c(urr) is now pointing at the newline
        //just let the '\n' case handle it to increment s->line
        //don't do that, that's stupid if any of the next characters are in the comment 7/27/26
        if (*c == '\n') { s->line++; break; }
      }

      //division
      else { consumeSingle(s, FORWARD_SLASH, c); }
      break;

    //single OR triple characters
    case '.': consumeTriple(s, ELLIPSIS, c); break;

    //string literals
    case '"': consumeString(s, c); break;

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
      if (isDigit(*c)) { consumeNumber(s, c); break; }

      //identifier and keywords
      if (isAlpha(*c)) { consumeIdentifier(s, c); break; }

      //error
      else { b_printString("unexpected character; continuing"); break; }
  }
}

Scanner* scan(const int fd) {

  //setup scanner struct
  Arena* a = b_allocArena();
  Scanner* s = b_alloc(a, sizeof(Scanner));
  s->a = a;
  s->line = 1;

  //read from file
  const int f = fd;
  s->length = b_fsize(f);
  s->source = (char*)b_fread(f) + sizeof(Arena);

  //allocate token space with upper bound of s->length
  s->tokens = b_alloc(a, sizeof(Token) * (s->length + 1));

  //initialize scanning loop
  s->curr = s->source;
  while (s->curr < s->source + s->length) {

    scanToken(s);
  }

  //manually add EOF token bc idrc
  Token* t = &s->tokens[s->count++];

  t->type = B_EOF;

  //no lexeme or literal because EOF is not in source buffer

  t->literal.b_string = b_alloc(a, 6);
  t->literal.b_string = "B_EOF";

  return s;
}
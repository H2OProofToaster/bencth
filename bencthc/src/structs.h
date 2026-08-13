//
// Created by nick on 8/2/26.
//

#ifndef BENCTH_STRUCTS_H
#define BENCTH_STRUCTS_H

#include "utils/allocator.h"
#include "utils/iHateLibC.h"

//first used in scanning/lexing

//logical value that a token can have
typedef enum {

  //single characters
  EQUALS, PLUS, LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  SEMICOLON, MINUS, STAR,
  EXCLAMATION, TILDE, PERCENT, AMPERSAND, COMMA,

  //single OR double characters
  FORWARD_SLASH, DOUBLE_FORWARD_SLASH,

  //single OR triple characters
  ELLIPSIS,

  //literals
  IDENTIFIER, INTEGER, STRING,

  //keywords
  RETURN, INT,

  B_EOF,
} TokenType;

//holds operable literals for tokens
//i.e. the actual int you can pull from, or the string
typedef union {

  int b_integer;
  //idk why doubles were here in the first place;
  //nowhere near implementing them 8/2/2026
  //double b_double;
  char b_char;
  char* b_string;

} Literal;

//token created when lexing
typedef struct {

  //arena for token storage, freed after codegen
  Arena* a;

  //line in source
  int line;

  //above is handled in addToken()
  //------------------------------
  //below is handled in consumeX()

  TokenType type;

  //encodes token from buffer with pointer to location, and length
  char* lexeme;
  size_t length;

  //this should be guaranteed to exist
  //value for numbers, strings, chars, and identifiers, null terminated if it's a string (locked af)
  Literal literal;

} Token;

//created during scanning/lexing and passed off to parser
typedef struct {

  //arena
  Arena* a;

  //token storage
  Token* tokens;
  size_t count;

  //source storage
  char* source;
  size_t length;

  //source maneuvering
  char* curr;
  int line;
} Scanner;

//holds the info of a keyword for the static array
typedef struct {

  const char* keyword;
  size_t length;
  TokenType type;

} Keyword;

//I really don't like this
static const Keyword keywords[] = {
  {"return", 6, RETURN},
  {"int", 3, INT},
};

//first used in parsing

//symbols for lookup, stored in a linked list
//ik, but we aren't really caring about lookup time right now
typedef struct Symbol {

  Token* token;
  struct Symbol* next;

  int offset; //filled in at codegen for stack lookup
} Symbol;
typedef struct SymbolTable{

  Symbol* head;

  struct SymbolTable* outerScope;
} SymbolTable;

//encoding the grammar

//A.1.4 Constants
typedef struct {

  char* identifier;
} Identifier;

typedef enum { INTEGER_DECIMAL_CONSTANT } IntegerConstantType;
typedef struct {

  //There isn't any differing storage needed for decimal/octal/hexadecimal constants

  IntegerConstantType type;
  int value;
} IntegerConstant;

typedef enum { CONSTANT_INTEGER } ConstantType;
typedef struct {

  ConstantType type;
  union {
    struct { IntegerConstant* integerConstant; } integerConstant;
  };
} Constant;

//A.2.1 Expressions
typedef enum { PRIMARY_IDENTIFIER, PRIMARY_CONSTANT, PRIMARY_EXPRESSION } PrimaryExpressionType;
typedef struct {

  PrimaryExpressionType type;
  union {
    struct { Identifier* identifier; } identifier;
    struct { Constant* constant; } constant;
    struct { struct Expression* expression; } expression;
  };
} PrimaryExpression;

typedef enum { UNARY_PRIMARY, UNARY_OPERATOR } UnaryExpressionType;
typedef struct UnaryExpression {

  UnaryExpressionType type;
  union {
    struct { PrimaryExpression* primaryExpression; } primaryExpression;
    struct { TokenType operator; struct UnaryExpression* unaryExpression; } operator;
  };
} UnaryExpression;

typedef enum { MULTIPLICATIVE_UNARY, MULTIPLICATIVE_OPERATOR } MultiplicativeExpressionType;
typedef struct MultiplicativeExpression {

  MultiplicativeExpressionType type;
  union {
    struct { UnaryExpression* unaryExpression; } unaryExpression;
    struct { struct MultiplicativeExpression* multiplicativeExpression; TokenType operator; UnaryExpression* unaryExpression; } operator;
  };
} MultiplicativeExpression;

typedef enum { ADDITIVE_MULTIPLICATIVE, ADDITIVE_OPERATOR } AdditiveExpressionType;
typedef struct AdditiveExpression {

  AdditiveExpressionType type;
  union {
    struct { MultiplicativeExpression* multiplicativeExpression; } multiplicativeExpression;
    struct { struct AdditiveExpression* additiveExpression; TokenType operator; MultiplicativeExpression* multiplicativeExpression; } operator;
  };
} AdditiveExpression;

typedef enum { ASSIGNMENT_ADDITIVE, ASSIGNMENT_OPERATOR } AssignmentExpressionType;
typedef struct AssignmentExpression {

  AssignmentExpressionType type;
  union {
    struct { AdditiveExpression* additiveExpression; } additiveExpression;
    struct { UnaryExpression* unaryExpression; TokenType operator; struct AssignmentExpression* assignmentExpression; } operator;
  };
} AssignmentExpression;

typedef enum { EXPRESSION_ASSIGNMENT, EXPRESSION_LIST } ExpressionType;
typedef struct Expression {

  ExpressionType type;
  union {
    struct { AssignmentExpression* assignmentExpression; } assignmentExpression;
    struct { struct Expression* expression; AssignmentExpression* assignmentExpression; } expressionList;
  };
} Expression;

//A.2.2 Declarations
typedef struct {

  struct DeclarationSpecifiers* declarationSpecifiers;
  struct InitDeclaratorList* initDeclaratorList;
} Declaration;

typedef struct DeclarationSpecifiers {

  struct TypeSpecifier* typeSpecifier;
  struct DeclarationSpecifiers* declarationSpecifiers;
} DeclarationSpecifiers;

typedef struct InitDeclaratorList {

  struct InitDeclarator* initDeclarator;
  struct InitDeclaratorList* initDeclaratorList; //comma separated, with an optional trailing comma, i.e. { 1, 2, 3, }
} InitDeclaratorList;

typedef struct InitDeclarator {

  struct Declarator* declarator;
  struct Initializer* initializer;
} InitDeclarator;

typedef struct TypeSpecifier {

  TokenType type;
} TypeSpecifier;

typedef struct Declarator {

  struct DirectDeclarator* directDeclarator;
} Declarator;

typedef enum { DIRECT_DECLARATOR_IDENTIFIER, DIRECT_DECLARATOR_DECLARATOR, DIRECT_DECLARATOR_PARAMETER_TYPE_LIST, DIRECT_DECLARATOR_IDENTIFIER_LIST, DIRECT_DECLARATOR_EMPTY_IDENTIFIER_LIST } DirectDeclaratorType;
typedef struct DirectDeclarator {

  DirectDeclaratorType type;
  union {
    struct { Identifier* identifier; } identifier;
    struct { Declarator* declarator; } declarator;
    struct { struct DirectDeclarator* directDeclarator; struct ParameterTypeList* parameterTypeList; } parameterTypeList;
    struct { struct DirectDeclarator* directDeclarator; struct IdentifierList* identifierList; } identifierList;
  };
} DirectDeclarator;

typedef enum { PARAMETER_TYPE_LIST_PARAMETER_LIST, PARAMETER_TYPE_LIST_ELLIPSIS } ParameterTypeListType;
typedef struct ParameterTypeList {

  ParameterTypeListType type;
  struct ParameterList* parameterList;
} ParameterTypeList;

typedef struct ParameterList {

  struct ParameterList* parameterList;
  struct ParameterDeclaration* parameterDeclaration;
} ParameterList;

typedef struct ParameterDeclaration {

  DeclarationSpecifiers* declarationSpecifiers;
  Declarator* declarator;
} ParameterDeclaration;

typedef struct IdentifierList {

  struct IdentifierList* identifierList;
  Identifier* identifier;
} IdentifierList;

typedef enum { INITIALIZER_ASSIGNMENT, INITIALIZER_INITIALIZER_LIST } InitializerType;
typedef struct Initializer {

  InitializerType type;
  union {
    struct { AssignmentExpression* assignmentExpression; } assignment;
    struct { struct InitializerList* initializerList; } initializerList;
  };
} Initializer;

typedef struct InitializerList {

  struct InitializerList* initializerList;
  Initializer* initializer;
} InitializerList;

//A.2.3 Statements
typedef enum { STATEMENT_COMPOUND, STATEMENT_EXPRESSION, STATEMENT_JUMP } StatementType;
typedef struct {

  StatementType type;
  union {
    struct { struct CompoundStatement* compoundStatement; } compound;
    struct { struct ExpressionStatement* expressionStatement; } expression;
    struct { struct JumpStatement* jumpStatement; } jump;
  };
} Statement;

typedef struct CompoundStatement {

  struct DeclarationList* declarationList;
  struct StatementList* statementList;
} CompoundStatement;

typedef struct DeclarationList {

  struct DeclarationList* declarationList;
  Declaration* declaration;
} DeclarationList;

typedef struct StatementList {

  struct StatementList* statementList;
  Statement* statement;
} StatementList;

typedef struct ExpressionStatement {

  Expression* expression;
} ExpressionStatement;

typedef enum { JUMP_RETURN } JumpStatementType;
typedef struct JumpStatement {

  JumpStatementType type;
  union {
    struct { Expression* expression; } b_return;
  };
} JumpStatement;

//A.2.4 External Definitions
typedef struct {

  struct ExternalDeclaration* externalDeclaration;

  SymbolTable* symbolTable;
} TranslationUnit;

typedef struct ExternalDeclaration {

  struct FunctionDefinition* functionDefinition;
} ExternalDeclaration;

typedef struct FunctionDefinition {

  DeclarationSpecifiers* declarationSpecifiers;
  Declarator* declarator;
  DeclarationList* declarationList;
  CompoundStatement* compoundStatement;

  SymbolTable* symbolTable;
} FunctionDefinition;

typedef struct {

  //node storage
  Arena* a;

  //from scanner
  Token* tokens;
  size_t count;

  //next token index
  size_t current;

  //head of ast
  TranslationUnit* program;
} Parser;

#endif //BENCTH_STRUCTS_H
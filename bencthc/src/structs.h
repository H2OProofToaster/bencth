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
} g_Identifier;

typedef enum { INTEGER_DECIMAL_CONSTANT } g_IntegerConstantType;
typedef struct {

  //There isn't any differing storage needed for decimal/octal/hexadecimal constants

  g_IntegerConstantType type;
  int value;
} g_IntegerConstant;

typedef enum { CONSTANT_INTEGER } g_ConstantType;
typedef struct {

  g_ConstantType type;
  union {
    struct { g_IntegerConstant* integerConstant; } integerConstant;
  };
} g_Constant;

//A.2.1 Expressions
typedef enum { PRIMARY_IDENTIFIER, PRIMARY_CONSTANT, PRIMARY_EXPRESSION } g_PrimaryExpressionType;
typedef struct {

  g_PrimaryExpressionType type;
  union {
    struct { g_Identifier* identifier; } identifier;
    struct { g_Constant* constant; } constant;
    struct { struct g_Expression* expression; } expression;
  };
} g_PrimaryExpression;

typedef enum { UNARY_PRIMARY, UNARY_OPERATOR } g_UnaryExpressionType;
typedef struct g_UnaryExpression {

  g_UnaryExpressionType type;
  union {
    struct { g_PrimaryExpression* primaryExpression; } primaryExpression;
    struct { TokenType operator; struct g_UnaryExpression* unaryExpression; } operator;
  };
} g_UnaryExpression;

typedef enum { MULTIPLICATIVE_UNARY, MULTIPLICATIVE_OPERATOR } g_MultiplicativeExpressionType;
typedef struct g_MultiplicativeExpression {

  g_MultiplicativeExpressionType type;
  union {
    struct { g_UnaryExpression* unaryExpression; } unaryExpression;
    struct { struct g_MultiplicativeExpression* multiplicativeExpression; TokenType operator; g_UnaryExpression* unaryExpression; } operator;
  };
} g_MultiplicativeExpression;

typedef enum { ADDITIVE_MULTIPLICATIVE, ADDITIVE_OPERATOR } g_AdditiveExpressionType;
typedef struct g_AdditiveExpression {

  g_AdditiveExpressionType type;
  union {
    struct { g_MultiplicativeExpression* multiplicativeExpression; } multiplicativeExpression;
    struct { struct g_AdditiveExpression* additiveExpression; TokenType operator; g_MultiplicativeExpression* multiplicativeExpression; } operator;
  };
} g_AdditiveExpression;

typedef enum { ASSIGNMENT_ADDITIVE, ASSIGNMENT_OPERATOR } g_AssignmentExpressionType;
typedef struct g_AssignmentExpression {

  g_AssignmentExpressionType type;
  union {
    struct { g_AdditiveExpression* additiveExpression; } additiveExpression;
    struct { g_UnaryExpression* unaryExpression; TokenType operator; struct g_AssignmentExpression* assignmentExpression; } operator;
  };
} g_AssignmentExpression;

typedef enum { EXPRESSION_ASSIGNMENT, EXPRESSION_LIST } g_ExpressionType;
typedef struct g_Expression {

  g_ExpressionType type;
  union {
    struct { g_AssignmentExpression* assignmentExpression; } assignmentExpression;
    struct { struct g_Expression* expression; g_AssignmentExpression* assignmentExpression; } expressionList;
  };
} g_Expression;

//A.2.2 Declarations
typedef struct {

  struct g_DeclarationSpecifiers* declarationSpecifiers;
  struct g_InitDeclaratorList* initDeclaratorList;
} g_Declaration;

typedef struct g_DeclarationSpecifiers {

  struct g_TypeSpecifier* typeSpecifier;
  struct g_DeclarationSpecifiers* declarationSpecifiers;
} g_DeclarationSpecifiers;

typedef struct g_InitDeclaratorList {

  struct g_InitDeclarator* initDeclarator;
  struct g_InitDeclaratorList* initDeclaratorList; //comma separated, with an optional trailing comma, i.e. { 1, 2, 3, }
} g_InitDeclaratorList;

typedef struct g_InitDeclarator {

  struct g_Declarator* declarator;
  struct g_Initializer* initializer;
} g_InitDeclarator;

typedef struct g_TypeSpecifier {

  TokenType type;
} g_TypeSpecifier;

typedef struct g_Declarator {

  struct g_DirectDeclarator* directDeclarator;
} g_Declarator;

typedef enum { DIRECT_DECLARATOR_IDENTIFIER, DIRECT_DECLARATOR_DECLARATOR, DIRECT_DECLARATOR_PARAMETER_TYPE_LIST, DIRECT_DECLARATOR_IDENTIFIER_LIST, DIRECT_DECLARATOR_EMPTY_IDENTIFIER_LIST } g_DirectDeclaratorType;
typedef struct g_DirectDeclarator {

  g_DirectDeclaratorType type;
  union {
    struct { g_Identifier* identifier; } identifier;
    struct { g_Declarator* declarator; } declarator;
    struct { struct g_DirectDeclarator* directDeclarator; struct g_ParameterTypeList* parameterTypeList; } parameterTypeList;
    struct { struct g_DirectDeclarator* directDeclarator; struct g_IdentifierList* identifierList; } identifierList;
  };
} g_DirectDeclarator;

typedef enum { PARAMETER_TYPE_LIST_PARAMETER_LIST, PARAMETER_TYPE_LIST_ELLIPSIS } g_ParameterTypeListType;
typedef struct g_ParameterTypeList {

  g_ParameterTypeListType type;
  struct g_ParameterList* parameterList;
} g_ParameterTypeList;

typedef struct g_ParameterList {

  struct g_ParameterList* parameterList;
  struct g_ParameterDeclaration* parameterDeclaration;
} g_ParameterList;

typedef struct g_ParameterDeclaration {

  g_DeclarationSpecifiers* declarationSpecifiers;
  g_Declarator* declarator;
} g_ParameterDeclaration;

typedef struct g_IdentifierList {

  struct g_IdentifierList* identifierList;
  g_Identifier* identifier;
} g_IdentifierList;

typedef enum { INITIALIZER_ASSIGNMENT, INITIALIZER_INITIALIZER_LIST } g_InitializerType;
typedef struct g_Initializer {

  g_InitializerType type;
  union {
    struct { g_AssignmentExpression* assignmentExpression; } assignment;
    struct { struct g_InitializerList* initializerList; } initializerList;
  };
} g_Initializer;

typedef struct g_InitializerList {

  struct g_InitializerList* initializerList;
  g_Initializer* initializer;
} g_InitializerList;

//A.2.3 Statements
typedef enum { STATEMENT_COMPOUND, STATEMENT_EXPRESSION, STATEMENT_JUMP } g_StatementType;
typedef struct {

  g_StatementType type;
  union {
    struct { struct g_CompoundStatement* compoundStatement; } compound;
    struct { struct g_ExpressionStatement* expressionStatement; } expression;
    struct { struct g_JumpStatement* jumpStatement; } jump;
  };
} g_Statement;

typedef struct g_CompoundStatement {

  struct g_DeclarationList* declarationList;
  struct g_StatementList* statementList;
} g_CompoundStatement;

typedef struct g_DeclarationList {

  struct g_DeclarationList* declarationList;
  g_Declaration* declaration;
} g_DeclarationList;

typedef struct g_StatementList {

  struct g_StatementList* statementList;
  g_Statement* statement;
} g_StatementList;

typedef struct g_ExpressionStatement {

  g_Expression* expression;
} g_ExpressionStatement;

typedef enum { JUMP_RETURN } g_JumpStatementType;
typedef struct g_JumpStatement {

  g_JumpStatementType type;
  union {
    struct { g_Expression* expression; } b_return;
  };
} g_JumpStatement;

//A.2.4 External Definitions
typedef struct {

  struct g_ExternalDeclaration* externalDeclaration;

  SymbolTable* symbolTable;
} g_TranslationUnit;

typedef struct g_ExternalDeclaration {

  struct g_FunctionDefinition* functionDefinition;
} g_ExternalDeclaration;

typedef struct g_FunctionDefinition {

  g_DeclarationSpecifiers* declarationSpecifiers;
  g_Declarator* declarator;
  g_DeclarationList* declarationList;
  g_CompoundStatement* compoundStatement;

  SymbolTable* symbolTable;
} g_FunctionDefinition;

typedef struct {

  //node storage
  Arena* a;

  //from scanner
  Token* tokens;
  size_t count;

  //next token index
  size_t current;

  //head of ast
  g_TranslationUnit* program;
} Parser;

#endif //BENCTH_STRUCTS_H
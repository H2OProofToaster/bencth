//
// Created by nick on 7/13/26.
//

#include "parser.h"

#include "utils/exit.h"
#include "utils/allocator.h"
#include "utils/symbols.h"
#include "utils/string.h"
#include "structs.h"

//forward declarations
static Arena* blindParserArena;
static TokenStream* tokenStream;
static SymbolTable* symbolTable;

//prototypes
g_Expression* parseExpression();
g_DeclarationSpecifiers* parseDeclarationSpecifiers();
g_InitDeclaratorList* parseInitDeclaratorList();
g_InitDeclarator* parseInitDeclarator();
g_TypeSpecifier* parseTypeSpecifier();
g_Declarator* parseDeclarator();
g_DirectDeclarator* parseDirectDeclarator();
g_ParameterTypeList* parseParameterTypeList();
g_ParameterList* parseParameterList();
g_ParameterDeclaration* parseParameterDeclaration();
g_IdentifierList* parseIdentifierList();
g_Initializer* parseInitializer();
g_InitializerList* parseInitializerList();
g_CompoundStatement* parseCompoundStatement();
g_DeclarationList* parseDeclarationList();
g_StatementList* parseStatementList();
g_ExpressionStatement* parseExpressionStatement();
g_JumpStatement* parseJumpStatement();
g_ExternalDeclaration* parseExternalDeclaration();
g_FunctionDefinition* parseFunctionDefinition();

//helpers for tokenstream
Token* p_lookN(const size_t n) {

  if (tokenStream->current >= tokenStream->count) { die("end of tokenstream"); }

  return &tokenStream->tokens[tokenStream->current + n - 1];
}

Token* p_peek() {

  return p_lookN(1);
}

Token* p_peekNext() {

  return p_lookN(2);
}

Token* p_advance() {

  Token* t = &tokenStream->tokens[tokenStream->current];
  if (tokenStream->current < tokenStream->count) { tokenStream->current++; }
  return t;
}

int p_check(const TokenType type) {

  return p_peek()->type == type;
}

Token* p_consume(const TokenType type, const char* err) {

  if (p_check(type)) { return p_advance(); }

  die(err);
}

//helpers for parsing
int isTypeSpecifier() { return p_peek()->type == INT; }
int isDirectDeclarator() { return p_peek()->type == IDENTIFIER || p_peek()->type == LEFT_PAREN; }
int isDeclarator() { return isDirectDeclarator(); }
int isInitDeclarator() { return isDeclarator(); }
int isDeclarationSpecifiers() { return isTypeSpecifier(); }
int isParameterDeclaration() { return isDeclarationSpecifiers(); }
int isParameterList() { return isParameterDeclaration(); }
int isParameterTypeList() { return isParameterList(); }
int isIdentifier() { return p_peek()->type == IDENTIFIER; }
int isIdentifierList() { return isIdentifier(); }
int isConstant() { return p_peek()->type == INTEGER; }
int isPrimaryExpression() { return isIdentifier() || isConstant() || p_peek()->type == LEFT_PAREN; }
int isUnaryOperator() { return p_peek()->type == PLUS || p_peek()->type == MINUS || p_peek()->type == EXCLAMATION || p_peek()->type == TILDE || p_peek()->type == AMPERSAND; }
int isUnaryExpression() { return isPrimaryExpression() || isUnaryOperator(); }
int isMultiplicativeExpression() { return isUnaryExpression(); }
int isAdditiveExpression() { return isMultiplicativeExpression(); }
int isAssignmentExpression() { return isAdditiveExpression(); }
int isInitializer() { return p_peek()->type == RIGHT_BRACE || isAssignmentExpression(); }
int isInitializerList() { return isInitializer(); }
int isExpression() { return isAssignmentExpression(); }
int isDeclaration() { return isDeclarationSpecifiers(); }
int isDeclarationList() { return isDeclaration(); }
int isCompoundStatement() { return p_peek()->type == LEFT_BRACE; }
int isJumpStatement() { return p_peek()->type == RETURN; }
int isExpressionStatement() { return isExpression() || p_peek()->type == SEMICOLON; }
int isStatement() { return isCompoundStatement() || isJumpStatement() || isExpressionStatement(); }
int isStatementList() { return isStatement(); }
int isFunctionDefinition() { return isDeclarationSpecifiers() || isDeclarator(); }
int isExternalDeclaration() { return isFunctionDefinition() || isDeclaration(); }
int isTranslationUnit() { return isExternalDeclaration(); }

//A.1.4 Constants
g_Identifier* parseIdentifier() {

  const Token* t = p_consume(IDENTIFIER, "expected identifier");
  g_Identifier* curr = b_alloc(blindParserArena, sizeof(g_Identifier));

  /* add to symbol table */
  if (isDefined(symbolTable, t)) { die( 
                                   b_concat(blindParserArena, "symbol ", 
                                   b_concat(blindParserArena, t->literal.b_string, " already exists") ) ); }
  curr->identifier = insertSymbol(symbolTable, t);

  return curr;
}

g_IntegerConstant* parseIntegerConstant() {

  const Token* t = p_consume(INTEGER, "expected integer literal");
  g_IntegerConstant* curr = b_alloc(blindParserArena, sizeof(g_IntegerConstant));
  curr->type = INTEGER_DECIMAL_CONSTANT;
  curr->value = t->literal.b_integer;
  return curr;
}

g_Constant* parseConstant() {

  g_Constant* curr = b_alloc(blindParserArena, sizeof(g_Constant));
  curr->type = CONSTANT_INTEGER;
  curr->integerConstant.integerConstant = parseIntegerConstant();
  return curr;
}

//A.2.1 Expressions
g_PrimaryExpression* parsePrimaryExpression() {

  g_PrimaryExpression* curr = b_alloc(blindParserArena, sizeof(g_PrimaryExpression));
  switch (p_peek()->type) {

    case IDENTIFIER:
      curr->type = PRIMARY_IDENTIFIER;
      curr->identifier.identifier = parseIdentifier();
      return curr;

    case INTEGER:
      curr->type = PRIMARY_CONSTANT;
      curr->constant.constant = parseConstant();
      return curr;

    case LEFT_PAREN:
      curr->type = PRIMARY_EXPRESSION;
      p_advance(); //eat '('
      curr->expression.expression = parseExpression();
      p_consume(RIGHT_PAREN, "expected ')' after expression");
      return curr;

    default:
      die("expected primary expression");
  }
}

g_UnaryExpression* parseUnaryExpression() {

  g_UnaryExpression* curr = b_alloc(blindParserArena, sizeof(g_UnaryExpression));
  switch (p_peek()->type) {

    case PLUS:
    case MINUS:
    case EXCLAMATION:
    case TILDE:
    case AMPERSAND:
      curr->type = UNARY_OPERATOR;
      curr->operator.operator = p_advance()->type;
      curr->operator.unaryExpression = parseUnaryExpression();
      return curr;

    default:
      curr->type = UNARY_PRIMARY;
      curr->primaryExpression.primaryExpression = parsePrimaryExpression();
      return curr;
  }
}

g_BinaryExpression* parseMultiplicativeExpression() {

  g_BinaryExpression* left = b_alloc(blindParserArena, sizeof(g_BinaryExpression));
  left->type = BINARY_UNARY;
  left->unary.left = parseUnaryExpression();

  while (p_peek()->type == STAR || p_peek()->type == FORWARD_SLASH) {

    g_BinaryExpression* curr = b_alloc(blindParserArena, sizeof(g_BinaryExpression));
    curr->type = BINARY_MULTIPLICATIVE;
    curr->binary.left = left;
    curr->binary.operator = p_advance()->type; //eat operator
    curr->binary.right = parseMultiplicativeExpression();

    left = curr;
  }

  return left;
}

g_BinaryExpression* parseAdditiveExpression() {

  g_BinaryExpression* left = b_alloc(blindParserArena, sizeof(g_BinaryExpression));

  while (p_peek()->type == PLUS || p_peek()->type == MINUS) {

    g_BinaryExpression* curr = b_alloc(blindParserArena, sizeof(g_BinaryExpression));
    curr->type = BINARY_ADDITIVE;
    curr->binary.left = left;
    curr->binary.operator = p_advance()->type; //eat operator
    curr->binary.right = parseAdditiveExpression();

    left = curr;
  }

  return left;
}

g_BinaryExpression* parseAssignmentExpression() {

  //When conditionals are added this gets funky

  //because if this recurses the left has to be a unary, and if it doesn't recurse it can be a conditional,
  //assume it goes there and then in the recursive loop check that it is a binary expression that has the unary tag
  g_BinaryExpression* left = parseAdditiveExpression(); //parse left side as a conditional expression

  //check for an assignment-operator
  if (p_peek()->type == EQUALS) {

    g_BinaryExpression* curr = b_alloc(blindParserArena, sizeof(g_BinaryExpression));

    //build assignment-expression
    curr->type = BINARY_ASSIGNMENT;
    //make sure left is unary
    if (left->type != BINARY_UNARY) { die("expected unary expression left of an assignment expression"); }
    curr->binary.left = left;
    curr->binary.operator = p_advance()->type; //eat equals
    curr->binary.right = parseAssignmentExpression();
    return curr;
  }

  return left;
}

g_Expression* parseExpression() {

  g_Expression* left = b_alloc(blindParserArena, sizeof(g_Expression));
  left->binaryExpression = parseAssignmentExpression();
  if (left->binaryExpression->type != BINARY_ASSIGNMENT) { die("expected assignment expression as expression"); }

  while (p_peek()->type == COMMA) {

    g_Expression* curr = b_alloc(blindParserArena, sizeof(g_Expression));
    curr->expression = left;
    p_advance(); //eat comma
    curr->binaryExpression = parseAssignmentExpression();
    if (curr->binaryExpression->type != BINARY_ASSIGNMENT) { die("expected assignment expression as left side of expression list"); }

    left = curr;
  }

  return left;
}

//A.2.2 Declarations
g_Declaration* parseDeclaration() {

  g_Declaration* curr = b_alloc(blindParserArena, sizeof(g_Declaration));
  curr->declarationSpecifiers = parseDeclarationSpecifiers();
  if (p_peek()->type != SEMICOLON) { curr->initDeclaratorList = parseInitDeclaratorList(); }
  p_advance(); //eat ';'
  return curr;
}

g_DeclarationSpecifiers* parseDeclarationSpecifiers() {

  g_DeclarationSpecifiers* curr = b_alloc(blindParserArena, sizeof(g_DeclarationSpecifiers));
  curr->typeSpecifier = parseTypeSpecifier();
  if (isTypeSpecifier()) { curr->declarationSpecifiers = parseDeclarationSpecifiers(); }
  else { curr->declarationSpecifiers = NULL; }
  return curr;
}

g_InitDeclaratorList* parseInitDeclaratorList() {

  //This is weird because you need to be able to accept a trailing comma
  // { 1, 2, 3, } (the comma after the 3)

  g_InitDeclaratorList* curr = b_alloc(blindParserArena, sizeof(g_InitDeclaratorList));
  curr->initDeclarator = parseInitDeclarator();
  if (p_check(COMMA)) { p_advance(); } //eat ',', but don't die if it doesn't exist (that's why I'm not using p_consume)
  if (isInitDeclarator()) { curr->initDeclaratorList = parseInitDeclaratorList(); }
  return curr;
}

g_InitDeclarator* parseInitDeclarator() {

  g_InitDeclarator* curr = b_alloc(blindParserArena, sizeof(g_InitDeclarator));
  curr->declarator = parseDeclarator();
  if (p_peek()->type == EQUALS) {

    p_advance();
    curr->initializer = parseInitializer();
  }
  return curr;
}

g_TypeSpecifier* parseTypeSpecifier() {

  g_TypeSpecifier* curr = b_alloc(blindParserArena, sizeof(g_TypeSpecifier));
  if (isTypeSpecifier()) { curr->type = p_advance()->type; }
  return curr;
}

g_Declarator* parseDeclarator() {

  g_Declarator* curr = b_alloc(blindParserArena, sizeof(g_Declarator));
  curr->directDeclarator = parseDirectDeclarator();
  return curr;
}

g_DirectDeclarator* parseDirectDeclarator() {

  g_DirectDeclarator* left = b_alloc(blindParserArena, sizeof(g_DirectDeclarator));
  switch (p_peek()->type) {

    case IDENTIFIER:
      left->type = DIRECT_DECLARATOR_IDENTIFIER;
      left->identifier.identifier = parseIdentifier();
      return left;

    case LEFT_PAREN:
      left->type = DIRECT_DECLARATOR_DECLARATOR;
      left->declarator.declarator = parseDeclarator();
      p_consume(RIGHT_PAREN, "expected ')' after declarator");
      return left;

    default:
      die("expected identifier or '('");
  }

  while (p_peek()->type == LEFT_PAREN) {

    p_advance(); //eat '('
    g_DirectDeclarator* curr = b_alloc(blindParserArena, sizeof(g_DirectDeclarator));

    if (isParameterTypeList()) {

      curr->type = DIRECT_DECLARATOR_PARAMETER_TYPE_LIST;
      curr->parameterTypeList.parameterTypeList = parseParameterTypeList();
      curr->parameterTypeList.directDeclarator = left;
    }

    else if (isIdentifierList()) {

      curr->type = DIRECT_DECLARATOR_IDENTIFIER_LIST;
      curr->identifierList.identifierList = parseIdentifierList();
      curr->identifierList.directDeclarator = left;
    }

    else {

      curr->type = DIRECT_DECLARATOR_EMPTY_IDENTIFIER_LIST;
      curr->identifierList.directDeclarator = left;
    }

    p_consume(RIGHT_PAREN, "expected ')' after parameter or identifier list");
    left = curr;
  }

  return left;
}

g_ParameterTypeList* parseParameterTypeList() {

  g_ParameterTypeList* curr = b_alloc(blindParserArena, sizeof(g_ParameterTypeList));
  curr->parameterList = parseParameterList();

  if (p_peek()->type == COMMA) {

    p_advance();
    p_consume(ELLIPSIS, "expected '...' after parameter list");
    curr->type = PARAMETER_TYPE_LIST_ELLIPSIS;
  }

  else { curr->type = PARAMETER_TYPE_LIST_PARAMETER_LIST; }

  return curr;
}

g_ParameterList* parseParameterList() {

  g_ParameterList* left = b_alloc(blindParserArena, sizeof(g_ParameterList));
  left->parameterDeclaration = parseParameterDeclaration();

  while (p_peek()->type == COMMA) {

    p_advance(); //eat ','
    g_ParameterDeclaration* right = parseParameterDeclaration();

    g_ParameterList* new = b_alloc(blindParserArena, sizeof(g_ParameterList));
    new->parameterList = left;
    new->parameterDeclaration = right;
    left = new;
  }

  return left;
}

g_ParameterDeclaration* parseParameterDeclaration() {

  g_ParameterDeclaration* curr = b_alloc(blindParserArena, sizeof(g_ParameterDeclaration));
  curr->declarationSpecifiers = parseDeclarationSpecifiers();
  curr->declarator = parseDeclarator();
  return curr;
}

g_IdentifierList* parseIdentifierList() {

  g_IdentifierList* left = b_alloc(blindParserArena, sizeof(g_IdentifierList));
  left->identifier = parseIdentifier();

  while (p_peek()->type == COMMA) {

    p_advance(); //eat ','
    g_Identifier* right = parseIdentifier();

    g_IdentifierList* new = b_alloc(blindParserArena, sizeof(g_IdentifierList));
    new->identifierList = left;
    new->identifier = right;
    left = new;
  }

  return left;
}

g_Initializer* parseInitializer() {

  g_Initializer* curr = b_alloc(blindParserArena, sizeof(g_Initializer));

  if (p_peek()->type == LEFT_BRACE) {

    p_advance();
    curr->type = INITIALIZER_INITIALIZER_LIST;
    curr->initializerList.initializerList = parseInitializerList();
    if (p_peek()->type == COMMA) { p_advance(); } //eat optional trailing ','
    p_consume(RIGHT_BRACE, "expected '}' after initializer");
  }

  else {

    curr->type = INITIALIZER_ASSIGNMENT;
    curr->assignment.binaryExpression = parseAssignmentExpression();
    if (curr->assignment.binaryExpression->type != BINARY_ASSIGNMENT) { die("expected assignment expression as initializer"); }
  }

  return curr;
}

g_InitializerList* parseInitializerList() {

  g_InitializerList* left = b_alloc(blindParserArena, sizeof(g_InitializerList));
  left->initializer = parseInitializer();

  while (p_peek()->type == COMMA) {

    p_advance(); //eat ','
    g_Initializer* right = parseInitializer();

    g_InitializerList* new = b_alloc(blindParserArena, sizeof(g_InitializerList));
    new->initializerList = left;
    new->initializer = right;
    left = new;
  }

  return left;
}

//A.2.3
g_Statement* parseStatement() {

  g_Statement* curr = b_alloc(blindParserArena, sizeof(g_Statement));
  switch (p_peek()->type) {

    case LEFT_BRACE:
      curr->type = STATEMENT_COMPOUND;
      curr->compound.compoundStatement = parseCompoundStatement();
      return curr;

    case RETURN:
      curr->type = STATEMENT_JUMP;
      curr->jump.jumpStatement = parseJumpStatement();
      return curr;

    case SEMICOLON:
      curr->type = STATEMENT_EXPRESSION;
      //empty expression statement
      return curr;

    default:

      if (isExpression()) {

        curr->type = STATEMENT_EXPRESSION;
        curr->expression.expressionStatement = parseExpressionStatement();
      }
  }

  return curr;
}

g_CompoundStatement* parseCompoundStatement() {

  g_CompoundStatement* curr = b_alloc(blindParserArena, sizeof(g_CompoundStatement));
  p_consume(LEFT_BRACE, "expected '{'");

  /* push symbol table */
  symbolTable = newSymbolTable(symbolTable);
  curr->symbolTable = symbolTable;

  if (isDeclarationList()) { curr->declarationList = parseDeclarationList(); }
  else { curr->declarationList = NULL; }
  if (isStatementList()) { curr->statementList = parseStatementList(); }
  else { curr->statementList = NULL; }

  /* pop symbol table */
  symbolTable = symbolTable->outerScope;

  p_consume(RIGHT_BRACE, "expected '}'");
  return curr;
}

g_DeclarationList* parseDeclarationList() {

  g_DeclarationList* left = b_alloc(blindParserArena, sizeof(g_DeclarationList));
  left->declaration = parseDeclaration();

  while (isDeclaration()) {

    g_Declaration* right = parseDeclaration();

    g_DeclarationList* new = b_alloc(blindParserArena, sizeof(g_DeclarationList));
    new->declarationList = left;
    new->declaration = right;
    left = new;
  }

  left->declarationList = NULL;
  return left;
}

g_StatementList* parseStatementList() {

  g_StatementList* left = b_alloc(blindParserArena, sizeof(g_StatementList));
  left->statement = parseStatement();

  while (isStatement()) {

    g_Statement* right = parseStatement();

    g_StatementList* new = b_alloc(blindParserArena, sizeof(g_StatementList));
    new->statementList = left;
    new->statement = right;
    left = new;
  }

  left->statementList = NULL;
  return left;
}

g_ExpressionStatement* parseExpressionStatement() {

  g_ExpressionStatement* curr = b_alloc(blindParserArena, sizeof(g_ExpressionStatement));
  if (isExpression()) { curr->expression = parseExpression(); }
  p_consume(SEMICOLON, "expected ';' after expression");
  return curr;
}

g_JumpStatement* parseJumpStatement() {

  g_JumpStatement* curr = b_alloc(blindParserArena, sizeof(g_JumpStatement));

  switch (p_peek()->type) {

    case RETURN:
      curr->type = JUMP_RETURN;
      p_advance(); //eat 'return'
      if (isExpression()) { curr->b_return.expression = parseExpression(); }
      p_consume(SEMICOLON, "expected ';' after expression");
      return curr;

    default:
      die("expected return statement");
  }

  return curr;
}

//A.2.4 External Definitions
g_TranslationUnit* parseTranslationUnit() {

  g_TranslationUnit* left = b_alloc(blindParserArena, sizeof(g_TranslationUnit));
  left->translationUnit = NULL;
  left->externalDeclaration = parseExternalDeclaration();
  left->symbolTable = symbolTable; //take current symbol table

  while (isTranslationUnit()) {

    g_ExternalDeclaration* right = parseExternalDeclaration();
    g_TranslationUnit* new = b_alloc(blindParserArena, sizeof(g_TranslationUnit));
    new->translationUnit = left;
    new->externalDeclaration = right;
    new->symbolTable = symbolTable; //take current symbol table
    left = new;
  }

  return left;
}

g_ExternalDeclaration* parseExternalDeclaration() {

  g_ExternalDeclaration* curr = b_alloc(blindParserArena, sizeof(g_ExternalDeclaration));
  if (isFunctionDefinition()) {

    curr->type = EXTERNAL_DECLARATION_FUNCTION;
    curr->function.functionDefinition = parseFunctionDefinition();
  }
  else if (isDeclaration()) {

    curr->type = EXTERNAL_DECLARATION_DECLARATION;
    curr->declaration.declaration = parseDeclaration();
  }
  else { die("expected function definition or declaration"); }

  return curr;
}

g_FunctionDefinition* parseFunctionDefinition() {

  g_FunctionDefinition* curr = b_alloc(blindParserArena, sizeof(g_FunctionDefinition));
  if (isDeclarationSpecifiers()) { curr->declarationSpecifiers = parseDeclarationSpecifiers(); }
  else { curr->declarationSpecifiers = NULL; }
  curr->declarator = parseDeclarator();
  if (isDeclarationList()) { curr->declarationList = parseDeclarationList(); }
  curr->compoundStatement = parseCompoundStatement();
  return curr;
}

g_TranslationUnit* parse(const Scanner* s) {

  blindParserArena = b_allocArena();

  tokenStream = b_alloc(blindParserArena, sizeof(TokenStream));
  tokenStream->tokens = s->tokens;
  tokenStream->count = s->count;
  tokenStream->current = 0;

  symbolTable = newSymbolTable(symbolTable);

  g_TranslationUnit* tU = parseTranslationUnit();

  return tU;
}
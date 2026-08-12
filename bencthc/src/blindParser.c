//
// Created by nick on 7/13/26.
//

#include "blindParser.h"

#include "utils/exit.h"
#include "utils/string.h"
#include "structs.h"

//forward declarations
static Arena* parserArena = NULL;
static Parser* p = NULL;

//prototypes
Expression* parseExpression();
DeclarationSpecifiers* parseDeclarationSpecifiers();
InitDeclaratorList* parseInitDeclaratorList();
InitDeclarator* parseInitDeclarator();
TypeSpecifier* parseTypeSpecifier();
Declarator* parseDeclarator();
DirectDeclarator* parseDirectDeclarator();
ParameterTypeList* parseParameterTypeList();
ParameterList* parseParameterList();
ParameterDeclaration* parseParameterDeclaration();
IdentifierList* parseIdentifierList();
Initializer* parseInitializer();
InitializerList* parseInitializerList();
CompoundStatement* parseCompoundStatement();
DeclarationList* parseDeclarationList();
StatementList* parseStatementList();
ExpressionStatement* parseExpressionStatement();
JumpStatement* parseJumpStatement();
ExternalDeclaration* parseExternalDeclaration();
FunctionDefinition* parseFunctionDefinition();

//helpers for tokenstream
Token* p_peek() {

  if (p->current >= p->count) { die("end of tokenstream"); }

  return &p->tokens[p->current];
}

Token* p_peekNext() {

  return &p->tokens[p->current + 1];
}

Token* p_advance() {

  Token* t = &p->tokens[p->current];
  if (p->current < p->count) { p->current++; }
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
int isTypeSpecifier() {

  if (p_peek()->type == INT) { return 1; }
  return 0;
}

int isDirectDeclarator() {

  size_t i = p->current;
  Token* curr = &p->tokens[i];

  while (curr->type != IDENTIFIER && curr->type != LEFT_PAREN) {

    if (i >= p->count - 1) { return 0; }

    curr = &p->tokens[++i];
  }

  return 1;
}

int isDeclarator() {

  return isDirectDeclarator();
}

int isInitDeclarator() {

  return isDeclarator();
}

int isDeclarationSpecifiers() {

  return isTypeSpecifier();
}

int isParameterDeclaration() {

  return isDeclarationSpecifiers();
}

int isParameterList() {

  return isParameterDeclaration();
}

int isParameterTypeList() {

  return isParameterList();
}

int isIdentifier() {

  return p_peek()->type == IDENTIFIER;
}

int isIdentifierList() {

  return isIdentifier();
}

int isConstant() {

  return p_peek()->type == INTEGER;
}

int isPrimaryExpression() {

  return isIdentifier() || isConstant() || p_peek()->type == LEFT_PAREN;
}

int isUnaryOperator() {

  return p_peek()->type == PLUS || p_peek()->type == MINUS || p_peek()->type == EXCLAMATION || p_peek()->type == TILDE || p_peek()->type == AMPERSAND;
}

int isUnaryExpression() {

  return isPrimaryExpression() || isUnaryOperator();
}

int isMultiplicativeExpression() {

  return isUnaryExpression();
}

int isAdditiveExpression() {

  return isMultiplicativeExpression();
}

int isAssignmentExpression() {

  return isAdditiveExpression();
}

int isInitializer() {

  return p_peek()->type == RIGHT_BRACE || isAssignmentExpression();
}

int isInitializerList() {

  return isInitializer();
}

int isExpression() {

  return isAssignmentExpression();
}

int isDeclaration() {

  return isDeclarationSpecifiers();
}

int isDeclarationList() {

  return isDeclaration();
}

int isCompoundStatement() {

  return p_peek()->type == LEFT_BRACE;
}

int isJumpStatement() {

  return p_peek()->type == RETURN;
}

int isExpressionStatement() {

  return isExpression() || p_peek()->type == SEMICOLON;
}

int isStatement() {

  return isCompoundStatement() || isJumpStatement() || isExpressionStatement();
}

int isStatementList() {

  return isStatement();
}

//symbol table manipulation
SymbolTable* initSymbolTable(SymbolTable* outer) {

  SymbolTable* sT = b_alloc(parserArena, sizeof(SymbolTable));
  sT->head = NULL;
  sT->outerScope = outer;
  return sT;
}

int isDefined(const SymbolTable* sT, const char* name) {

  for (const Symbol* s = sT->head; s != NULL; s = s->next) {

    if (b_strcmp(s->token->literal.b_string, name) == 0) { return 1; }
  }

  return 0;
}

void addSymbol(SymbolTable* sT, Token* token) {

  if (isDefined(sT, token->literal.b_string)) { die("Symbol already exists"); }

  Symbol* s = b_alloc(parserArena, sizeof(Symbol));
  s->token = token;
  s->next = sT->head;
  sT->head = s;
}

//A.1.4 Constants
Identifier* parseIdentifier() {

  const Token* t = p_consume(IDENTIFIER, "expected identifier");
  Identifier* curr = b_alloc(parserArena, sizeof(Identifier));
  curr->identifier = t->literal.b_string;
  return curr;
}

IntegerConstant* parseIntegerConstant() {

  const Token* t = p_consume(INTEGER, "expected integer literal");
  IntegerConstant* curr = b_alloc(parserArena, sizeof(IntegerConstant));
  curr->type = INTEGER_DECIMAL_CONSTANT;
  curr->value = t->literal.b_integer;
  return curr;
}

Constant* parseConstant() {

  Constant* curr = b_alloc(parserArena, sizeof(Constant));
  curr->type = CONSTANT_INTEGER;
  curr->integerConstant.integerConstant = parseIntegerConstant();
  return curr;
}

//A.2.1 Expressions
PrimaryExpression* parsePrimaryExpression() {

  PrimaryExpression* curr = b_alloc(parserArena, sizeof(PrimaryExpression));
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

UnaryExpression* parseUnaryExpression() {

  UnaryExpression* curr = b_alloc(parserArena, sizeof(UnaryExpression));
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

MultiplicativeExpression* parseMultiplicativeExpression() {

  MultiplicativeExpression* curr = b_alloc(parserArena, sizeof(MultiplicativeExpression));
  switch (p_peekNext()->type) {

    case STAR:
    case FORWARD_SLASH:
      curr->type = MULTIPLICATIVE_OPERATOR;
      curr->operator.multiplicativeExpression = parseMultiplicativeExpression();
      curr->operator.operator = p_advance()->type;
      curr->operator.unaryExpression = parseUnaryExpression();
      return curr;

    default:
      curr->type = MULTIPLICATIVE_UNARY;
      curr->unaryExpression.unaryExpression = parseUnaryExpression();
      return curr;
  }
}

AdditiveExpression* parseAdditiveExpression() {

  AdditiveExpression* curr = b_alloc(parserArena, sizeof(AdditiveExpression));
  switch (p_peekNext()->type) {

    case PLUS:
    case MINUS:
      curr->type = ADDITIVE_OPERATOR;
      curr->operator.additiveExpression = parseAdditiveExpression();
      curr->operator.operator = p_advance()->type;
      curr->operator.multiplicativeExpression = parseMultiplicativeExpression();
      return curr;

    default:
      curr->type = ADDITIVE_MULTIPLICATIVE;
      curr->multiplicativeExpression.multiplicativeExpression = parseMultiplicativeExpression();
      return curr;
  }
}

AssignmentExpression* parseAssignmentExpression() {

  AssignmentExpression* curr = b_alloc(parserArena, sizeof(AssignmentExpression));
  switch (p_peekNext()->type) {

    case EQUALS:
      curr->type = ASSIGNMENT_OPERATOR;
      curr->operator.unaryExpression = parseUnaryExpression();
      curr->operator.operator = p_advance()->type;
      curr->operator.assignmentExpression = parseAssignmentExpression();
      return curr;

    default:
      curr->type = ASSIGNMENT_ADDITIVE;
      curr->additiveExpression.additiveExpression = parseAdditiveExpression();
      return curr;
  }
}

Expression* parseExpression() {

  Expression* curr = b_alloc(parserArena, sizeof(Expression));
  switch (p_peekNext()->type) {

    case COMMA:
      curr->type = EXPRESSION_LIST;
      curr->expressionList.expression = parseExpression();
      p_advance(); //eat ','
      curr->expressionList.assignmentExpression = parseAssignmentExpression();
      return curr;

    default:
      curr->type = EXPRESSION_ASSIGNMENT;
      curr->assignmentExpression.assignmentExpression = parseAssignmentExpression();
      return curr;
  }
}

//A.2.2 Declarations
Declaration* parseDeclaration() {

  Declaration* curr = b_alloc(parserArena, sizeof(Declaration));
  curr->declarationSpecifiers = parseDeclarationSpecifiers();
  if (p_peek()->type != SEMICOLON) { curr->initDeclaratorList = parseInitDeclaratorList(); }
  p_advance(); //eat ';'
  return curr;
}

DeclarationSpecifiers* parseDeclarationSpecifiers() {

  DeclarationSpecifiers* curr = b_alloc(parserArena, sizeof(DeclarationSpecifiers));
  curr->typeSpecifier = parseTypeSpecifier();
  if (isTypeSpecifier()) { curr->declarationSpecifiers = parseDeclarationSpecifiers(); }
  return curr;
}

InitDeclaratorList* parseInitDeclaratorList() {

  //This is weird because you need to be able to accept a trailing comma
  // { 1, 2, 3, } (the comma after the 3)

  InitDeclaratorList* curr = b_alloc(parserArena, sizeof(InitDeclaratorList));
  curr->initDeclarator = parseInitDeclarator();
  if (p_check(COMMA)) { p_advance(); } //eat ',', but don't die if it doesn't exist (that's why I'm not using p_consume)
  if (isInitDeclarator()) { curr->initDeclaratorList = parseInitDeclaratorList(); }
  return curr;
}

InitDeclarator* parseInitDeclarator() {

  InitDeclarator* curr = b_alloc(parserArena, sizeof(InitDeclarator));
  curr->declarator = parseDeclarator();
  if (p_peek()->type == EQUALS) {

    p_advance();
    curr->initializer = parseInitializer();
  }
  return curr;
}

TypeSpecifier* parseTypeSpecifier() {

  TypeSpecifier* curr = b_alloc(parserArena, sizeof(TypeSpecifier));
  if (isTypeSpecifier()) { curr->type = p_advance()->type; }
  return curr;
}

Declarator* parseDeclarator() {

  Declarator* curr = b_alloc(parserArena, sizeof(Declarator));
  curr->directDeclarator = parseDirectDeclarator();
  return curr;
}

DirectDeclarator* parseDirectDeclarator() {

  DirectDeclarator* curr = b_alloc(parserArena, sizeof(DirectDeclarator));
  switch (p_peek()->type) {

    case IDENTIFIER:
      curr->type = DIRECT_DECLARATOR_IDENTIFIER;
      curr->identifier.identifier = parseIdentifier();

    case LEFT_PAREN:
      curr->type = DIRECT_DECLARATOR_DECLARATOR;
      curr->declarator.declarator = parseDeclarator();

    default:

      if (isDirectDeclarator()) {

        DirectDeclarator* temp = parseDirectDeclarator(); //I need to store this before I consume the '('
        p_consume(LEFT_PAREN, "expected '('");

        if (isParameterTypeList()) {

          curr->type = DIRECT_DECLARATOR_PARAMETER_TYPE_LIST;
          curr->parameterTypeList.directDeclarator = temp;
          curr->parameterTypeList.parameterTypeList = parseParameterTypeList();
        }

        else if (isIdentifierList()) {

          curr->type = DIRECT_DECLARATOR_IDENTIFIER_LIST;
          curr->identifierList.directDeclarator = temp;
          curr->identifierList.identifierList = parseIdentifierList();
        }

        else {

          curr->type = DIRECT_DECLARATOR_EMPTY_IDENTIFIER_LIST;
          curr->identifierList.directDeclarator = temp;
        }
      }
  }

  return curr;
}

ParameterTypeList* parseParameterTypeList() {

  ParameterTypeList* curr = b_alloc(parserArena, sizeof(ParameterTypeList));
  curr->parameterList = parseParameterList();

  if (p_peek()->type == COMMA) {

    p_advance();
    p_consume(ELLIPSIS, "expected '...' after parameter list");
    curr->type = PARAMETER_TYPE_LIST_ELLIPSIS;
  }

  else { curr->type = PARAMETER_TYPE_LIST_PARAMETER_LIST; }

  return curr;
}

ParameterList* parseParameterList() {

  ParameterList* curr = b_alloc(parserArena, sizeof(ParameterList));
  if (isParameterList()) {

    curr->parameterList = parseParameterList();
    p_consume(COMMA, "expected ',' after parameter list");
    curr->parameterDeclaration = parseParameterDeclaration();
  }

  else { curr->parameterDeclaration = parseParameterDeclaration(); }

  return curr;
}

ParameterDeclaration* parseParameterDeclaration() {

  ParameterDeclaration* curr = b_alloc(parserArena, sizeof(ParameterDeclaration));
  curr->declarationSpecifiers = parseDeclarationSpecifiers();
  curr->declarator = parseDeclarator();
  return curr;
}

IdentifierList* parseIdentifierList() {

  IdentifierList* curr = b_alloc(parserArena, sizeof(IdentifierList));
  if (isIdentifierList()) {

    curr->identifierList = parseIdentifierList();
    p_consume(COMMA, "expected ',' after identifier list");
    curr->identifier = parseIdentifier();
  }

  else { curr->identifier = parseIdentifier(); }

  return curr;
}

Initializer* parseInitializer() {

  Initializer* curr = b_alloc(parserArena, sizeof(Initializer));

  if (p_peek()->type == RIGHT_BRACE) {

    p_advance();
    curr->type = INITIALIZER_INITIALIZER_LIST;
    curr->initializerList.initializerList = parseInitializerList();
  }

  else {

    curr->type = INITIALIZER_ASSIGNMENT;
    curr->assignment.assignmentExpression = parseAssignmentExpression();
  }

  return curr;
}

InitializerList* parseInitializerList() {

  InitializerList* curr = b_alloc(parserArena, sizeof(InitializerList));

  if (isInitializerList()) {

    curr->initializerList = parseInitializerList();
    p_consume(COMMA, "expected ',' after initializer list");
    curr->initializer = parseInitializer();
  }

  else { curr->initializer = parseInitializer(); }

  return curr;
}

//A.2.3
Statement* parseStatement() {

  Statement* curr = b_alloc(parserArena, sizeof(Statement));
  switch (p_peek()->type) {

    case LEFT_BRACE:
      curr->type = STATEMENT_COMPOUND;
      curr->compound.compoundStatement = parseCompoundStatement();

    case RETURN:
      curr->type = STATEMENT_JUMP;
      curr->jump.jumpStatement = parseJumpStatement();

    case SEMICOLON:
      curr->type = STATEMENT_EXPRESSION;
      //empty expression statement

    default:

      if (isExpression()) {

        curr->type = STATEMENT_EXPRESSION;
        curr->expression.expressionStatement = parseExpressionStatement();
      }
  }

  return curr;
}

CompoundStatement* parseCompoundStatement() {

  CompoundStatement* curr = b_alloc(parserArena, sizeof(CompoundStatement));
  p_consume(LEFT_BRACE, "expected '{'");
  if (isDeclarationList()) { curr->declarationList = parseDeclarationList(); }
  if (isStatementList()) { curr->statementList = parseStatementList(); }
  p_consume(RIGHT_BRACE, "expected '}'");
  return curr;
}

DeclarationList* parseDeclarationList() {

  DeclarationList* curr = b_alloc(parserArena, sizeof(DeclarationList));

  if (isDeclarationList()) {

    curr->declarationList = parseDeclarationList();
    curr->declaration = parseDeclaration();
  }

  else { curr->declaration = parseDeclaration(); }

  return curr;
}

StatementList* parseStatementList() {

  StatementList* curr = b_alloc(parserArena, sizeof(StatementList));

  if (isStatementList()) {

    curr->statementList = parseStatementList();
    curr->statement = parseStatement();
  }

  else { curr->statement = parseStatement(); }

  return curr;
}

ExpressionStatement* parseExpressionStatement() {

  ExpressionStatement* curr = b_alloc(parserArena, sizeof(ExpressionStatement));
  if (isExpression()) { curr->expression = parseExpression(); }
  p_consume(SEMICOLON, "expected ';' after expression");
  return curr;
}

JumpStatement* parseJumpStatement() {

  JumpStatement* curr = b_alloc(parserArena, sizeof(JumpStatement));

  switch (p_peek()->type) {

    case RETURN:
      curr->type = JUMP_RETURN;
      p_advance(); //eat 'return'
      if (isExpression()) { curr->b_return.expression = parseExpression(); }
      p_consume(SEMICOLON, "expected ';' after expression");

    default:
      die("expected return statement");
  }

  return curr;
}

//A.2.4 External Definitions
TranslationUnit* parseTranslationUnit() {

  TranslationUnit* curr = b_alloc(parserArena, sizeof(TranslationUnit));
  curr->externalDeclaration = parseExternalDeclaration();
  return curr;
}

ExternalDeclaration* parseExternalDeclaration() {

  ExternalDeclaration* curr = b_alloc(parserArena, sizeof(ExternalDeclaration));
  curr->functionDefinition = parseFunctionDefinition();
  return curr;
}

FunctionDefinition* parseFunctionDefinition() {

  FunctionDefinition* curr = b_alloc(parserArena, sizeof(FunctionDefinition));
  if (isDeclarationSpecifiers()) { curr->declarationSpecifiers = parseDeclarationSpecifiers(); }
  curr->declarator = parseDeclarator();
  if (isDeclarationList()) { curr->declarationList = parseDeclarationList(); }
  curr->compoundStatement = parseCompoundStatement();
  return curr;
}

Parser* parse(const Scanner* s) {

  parserArena = b_allocArena();

  p = b_alloc(parserArena, sizeof(Parser));
  p->a = parserArena;
  p->tokens = s->tokens;
  p->count = s->count;
  p->current = 0;

  p->program = parseTranslationUnit();

  //check that entry is called main
  if (b_strcmp(p->program->externalDeclaration->functionDefinition->declarator->directDeclarator->identifier.identifier->identifier,
               "main") != 0)
    { die("entry function not called main"); }

  return p;
}
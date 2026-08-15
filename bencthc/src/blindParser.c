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

  if (p->current >= p->count) { die("end of tokenstream"); }

  return &p->tokens[p->current + n - 1];
}

Token* p_peek() {

  return p_lookN(1);
}

Token* p_peekNext() {

  return p_lookN(2);
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

  return p_peek()->type == IDENTIFIER || p_peek()->type == LEFT_PAREN;
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
g_Identifier* parseIdentifier() {

  const Token* t = p_consume(IDENTIFIER, "expected identifier");
  g_Identifier* curr = b_alloc(parserArena, sizeof(g_Identifier));
  curr->identifier = t->literal.b_string;
  return curr;
}

g_IntegerConstant* parseIntegerConstant() {

  const Token* t = p_consume(INTEGER, "expected integer literal");
  g_IntegerConstant* curr = b_alloc(parserArena, sizeof(g_IntegerConstant));
  curr->type = INTEGER_DECIMAL_CONSTANT;
  curr->value = t->literal.b_integer;
  return curr;
}

g_Constant* parseConstant() {

  g_Constant* curr = b_alloc(parserArena, sizeof(g_Constant));
  curr->type = CONSTANT_INTEGER;
  curr->integerConstant.integerConstant = parseIntegerConstant();
  return curr;
}

//A.2.1 Expressions
g_PrimaryExpression* parsePrimaryExpression() {

  g_PrimaryExpression* curr = b_alloc(parserArena, sizeof(g_PrimaryExpression));
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

  g_UnaryExpression* curr = b_alloc(parserArena, sizeof(g_UnaryExpression));
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

g_MultiplicativeExpression* parseMultiplicativeExpression() {

  g_MultiplicativeExpression* left = b_alloc(parserArena, sizeof(g_MultiplicativeExpression));
  left->type = MULTIPLICATIVE_UNARY;
  left->unaryExpression.unaryExpression = parseUnaryExpression();

  while (p_peek()->type == STAR || p_peek()->type == FORWARD_SLASH) {

    const TokenType operator = p_advance()->type; //eat operator
    g_UnaryExpression* right = parseUnaryExpression();

    g_MultiplicativeExpression* new = b_alloc(parserArena, sizeof(g_MultiplicativeExpression));
    new->type = MULTIPLICATIVE_OPERATOR;
    new->operator.multiplicativeExpression = left;
    new->operator.operator = operator;
    new->operator.unaryExpression = right;

    left = new;
  }

  return left;
}

g_AdditiveExpression* parseAdditiveExpression() {

  g_AdditiveExpression* left = b_alloc(parserArena, sizeof(g_AdditiveExpression));
  left->type = ADDITIVE_MULTIPLICATIVE;
  left->multiplicativeExpression.multiplicativeExpression = parseMultiplicativeExpression();

  while (p_peek()->type == PLUS || p_peek()->type == MINUS) {

    const TokenType operator = p_advance()->type; //eat operator
    g_MultiplicativeExpression* right = parseMultiplicativeExpression();

    g_AdditiveExpression* new = b_alloc(parserArena, sizeof(g_AdditiveExpression));
    new->type = ADDITIVE_OPERATOR;
    new->operator.additiveExpression = left;
    new->operator.operator = operator;
    new->operator.multiplicativeExpression = right;

    left = new;
  }

  return left;
}

g_AssignmentExpression* parseAssignmentExpression() {

  g_AssignmentExpression* curr = b_alloc(parserArena, sizeof(g_AssignmentExpression));

  g_AdditiveExpression* left = parseAdditiveExpression(); //parse left side up to an additive expression

  if (p_peek()->type == EQUALS) {

    //if equals, then left has to be unary
    const int isValidLeft = left->type == ADDITIVE_MULTIPLICATIVE && left->multiplicativeExpression.multiplicativeExpression->type == MULTIPLICATIVE_UNARY;
    if (!isValidLeft) { die("expected unary expression"); }

    curr->type = ASSIGNMENT_OPERATOR;
    curr->operator.unaryExpression = left->multiplicativeExpression.multiplicativeExpression->unaryExpression.unaryExpression;
    curr->operator.operator = p_advance()->type;
    curr->operator.assignmentExpression = parseAssignmentExpression();
    return curr;
  }

  curr->type = ASSIGNMENT_ADDITIVE;
  curr->additiveExpression.additiveExpression = left;
  return curr;
}

g_Expression* parseExpression() {

  g_Expression* left = b_alloc(parserArena, sizeof(g_Expression));
  left->type = EXPRESSION_ASSIGNMENT;
  left->assignmentExpression.assignmentExpression = parseAssignmentExpression();

  while (p_peek()->type == COMMA) {

    p_advance(); //eat ','
    g_AssignmentExpression* right = parseAssignmentExpression();

    g_Expression* new = b_alloc(parserArena, sizeof(g_Expression));
    new->type = EXPRESSION_LIST;
    new->expressionList.expression = left;
    new->expressionList.assignmentExpression = right;
    left = new;
  }

  return left;
}

//A.2.2 Declarations
g_Declaration* parseDeclaration() {

  g_Declaration* curr = b_alloc(parserArena, sizeof(g_Declaration));
  curr->declarationSpecifiers = parseDeclarationSpecifiers();
  if (p_peek()->type != SEMICOLON) { curr->initDeclaratorList = parseInitDeclaratorList(); }
  p_advance(); //eat ';'
  return curr;
}

g_DeclarationSpecifiers* parseDeclarationSpecifiers() {

  g_DeclarationSpecifiers* curr = b_alloc(parserArena, sizeof(g_DeclarationSpecifiers));
  curr->typeSpecifier = parseTypeSpecifier();
  if (isTypeSpecifier()) { curr->declarationSpecifiers = parseDeclarationSpecifiers(); }
  return curr;
}

g_InitDeclaratorList* parseInitDeclaratorList() {

  //This is weird because you need to be able to accept a trailing comma
  // { 1, 2, 3, } (the comma after the 3)

  g_InitDeclaratorList* curr = b_alloc(parserArena, sizeof(g_InitDeclaratorList));
  curr->initDeclarator = parseInitDeclarator();
  if (p_check(COMMA)) { p_advance(); } //eat ',', but don't die if it doesn't exist (that's why I'm not using p_consume)
  if (isInitDeclarator()) { curr->initDeclaratorList = parseInitDeclaratorList(); }
  return curr;
}

g_InitDeclarator* parseInitDeclarator() {

  g_InitDeclarator* curr = b_alloc(parserArena, sizeof(g_InitDeclarator));
  curr->declarator = parseDeclarator();
  if (p_peek()->type == EQUALS) {

    p_advance();
    curr->initializer = parseInitializer();
  }
  return curr;
}

g_TypeSpecifier* parseTypeSpecifier() {

  g_TypeSpecifier* curr = b_alloc(parserArena, sizeof(g_TypeSpecifier));
  if (isTypeSpecifier()) { curr->type = p_advance()->type; }
  return curr;
}

g_Declarator* parseDeclarator() {

  g_Declarator* curr = b_alloc(parserArena, sizeof(g_Declarator));
  curr->directDeclarator = parseDirectDeclarator();
  return curr;
}

g_DirectDeclarator* parseDirectDeclarator() {

  g_DirectDeclarator* left = b_alloc(parserArena, sizeof(g_DirectDeclarator));
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
    g_DirectDeclarator* curr = b_alloc(parserArena, sizeof(g_DirectDeclarator));

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

  g_ParameterTypeList* curr = b_alloc(parserArena, sizeof(g_ParameterTypeList));
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

  g_ParameterList* left = b_alloc(parserArena, sizeof(g_ParameterList));
  left->parameterDeclaration = parseParameterDeclaration();

  while (p_peek()->type == COMMA) {

    p_advance(); //eat ','
    g_ParameterDeclaration* right = parseParameterDeclaration();

    g_ParameterList* new = b_alloc(parserArena, sizeof(g_ParameterList));
    new->parameterList = left;
    new->parameterDeclaration = right;
    left = new;
  }

  return left;
}

g_ParameterDeclaration* parseParameterDeclaration() {

  g_ParameterDeclaration* curr = b_alloc(parserArena, sizeof(g_ParameterDeclaration));
  curr->declarationSpecifiers = parseDeclarationSpecifiers();
  curr->declarator = parseDeclarator();
  return curr;
}

g_IdentifierList* parseIdentifierList() {

  g_IdentifierList* left = b_alloc(parserArena, sizeof(g_IdentifierList));
  left->identifier = parseIdentifier();

  while (p_peek()->type == COMMA) {

    p_advance(); //eat ','
    g_Identifier* right = parseIdentifier();

    g_IdentifierList* new = b_alloc(parserArena, sizeof(g_IdentifierList));
    new->identifierList = left;
    new->identifier = right;
    left = new;
  }

  return left;
}

g_Initializer* parseInitializer() {

  g_Initializer* curr = b_alloc(parserArena, sizeof(g_Initializer));

  if (p_peek()->type == LEFT_BRACE) {

    p_advance();
    curr->type = INITIALIZER_INITIALIZER_LIST;
    curr->initializerList.initializerList = parseInitializerList();
    if (p_peek()->type == COMMA) { p_advance(); } //eat optional trailing ','
    p_consume(RIGHT_BRACE, "expected '}' after initializer");
  }

  else {

    curr->type = INITIALIZER_ASSIGNMENT;
    curr->assignment.assignmentExpression = parseAssignmentExpression();
  }

  return curr;
}

g_InitializerList* parseInitializerList() {

  g_InitializerList* left = b_alloc(parserArena, sizeof(g_InitializerList));
  left->initializer = parseInitializer();

  while (p_peek()->type == COMMA) {

    p_advance(); //eat ','
    g_Initializer* right = parseInitializer();

    g_InitializerList* new = b_alloc(parserArena, sizeof(g_InitializerList));
    new->initializerList = left;
    new->initializer = right;
    left = new;
  }

  return left;
}

//A.2.3
g_Statement* parseStatement() {

  g_Statement* curr = b_alloc(parserArena, sizeof(g_Statement));
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

  g_CompoundStatement* curr = b_alloc(parserArena, sizeof(g_CompoundStatement));
  p_consume(LEFT_BRACE, "expected '{'");
  if (isDeclarationList()) { curr->declarationList = parseDeclarationList(); }
  if (isStatementList()) { curr->statementList = parseStatementList(); }
  p_consume(RIGHT_BRACE, "expected '}'");
  return curr;
}

g_DeclarationList* parseDeclarationList() {

  g_DeclarationList* left = b_alloc(parserArena, sizeof(g_DeclarationList));
  left->declaration = parseDeclaration();

  while (isDeclaration()) {

    g_Declaration* right = parseDeclaration();

    g_DeclarationList* new = b_alloc(parserArena, sizeof(g_DeclarationList));
    new->declarationList = left;
    new->declaration = right;
    left = new;
  }

  return left;
}

g_StatementList* parseStatementList() {

  g_StatementList* left = b_alloc(parserArena, sizeof(g_StatementList));
  left->statement = parseStatement();

  while (isStatement()) {

    g_Statement* right = parseStatement();

    g_StatementList* new = b_alloc(parserArena, sizeof(g_StatementList));
    new->statementList = left;
    new->statement = right;
    left = new;
  }

  return left;
}

g_ExpressionStatement* parseExpressionStatement() {

  g_ExpressionStatement* curr = b_alloc(parserArena, sizeof(g_ExpressionStatement));
  if (isExpression()) { curr->expression = parseExpression(); }
  p_consume(SEMICOLON, "expected ';' after expression");
  return curr;
}

g_JumpStatement* parseJumpStatement() {

  g_JumpStatement* curr = b_alloc(parserArena, sizeof(g_JumpStatement));

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

  g_TranslationUnit* curr = b_alloc(parserArena, sizeof(g_TranslationUnit));
  curr->externalDeclaration = parseExternalDeclaration();
  return curr;
}

g_ExternalDeclaration* parseExternalDeclaration() {

  g_ExternalDeclaration* curr = b_alloc(parserArena, sizeof(g_ExternalDeclaration));
  curr->functionDefinition = parseFunctionDefinition();
  return curr;
}

g_FunctionDefinition* parseFunctionDefinition() {

  g_FunctionDefinition* curr = b_alloc(parserArena, sizeof(g_FunctionDefinition));
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
  //this may kill itself soon
  if (b_strcmp(p->program->externalDeclaration->functionDefinition->declarator->directDeclarator->identifier.identifier->identifier,
               "main") != 0)
    { die("entry function not called main"); }

  return p;
}
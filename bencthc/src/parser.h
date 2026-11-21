//
// Created by nick on 7/13/26.
//

#ifndef BENCTH_PARSER_H
#define BENCTH_PARSER_H

#include "lexer.h"
#include "symbols.h"

//context

typedef struct Context {

    SymbolTable* symbolTable;

    struct Function* currentFunction;
    Token* returnType;
    struct Statement* returnStatement;
} Context;

//AST nodes

typedef enum ExpressionType { EXPRESSION_BINARY, EXPRESSION_IDENTIFIER, EXPRESSION_CONSTANT } ExpressionType;
typedef struct Expression {

    ExpressionType type;
    union {

        struct binary { struct Expression* left; TokenType operator; struct Expression* right; } binary;
        struct identifier { char* identifier; } identifier;
        struct constant { int constant; } constant;
    };
} Expression;

typedef struct Initializer {

    Expression* assignmentExpression;
} Initializer;

typedef struct Declaration {

    Token* typeSpecifier;
    Token* identifier;
    Initializer* initializer;

    //used to collapse declaration-list into a linked list
    struct Declaration* next;
} Declaration;

typedef enum StatementType { STATEMENT_EXPRESSION, STATEMENT_JUMP } StatementType;
typedef struct Statement {

    StatementType type;
    union {

        struct expression { Expression* expression; } expression;
        struct jump { Keyword keyword; Expression* expression; } jump;
    };

    //used to collapse statement-list into a linked list
    struct Statement* next;
} Statement;

typedef struct Function {

    SymbolTable* symbolTable;

    Token* returnType;
    Token* identifier;
    Declaration* declarations;
    Statement* statements;
} Function;

typedef struct Source {

    Function* main;
} Source;

Source* parse(Scanner* s);

#endif //BENCTH_PARSER_H

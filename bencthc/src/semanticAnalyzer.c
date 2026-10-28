//
// Created by nick on 9/11/26.
//

#include "semanticAnalyzer.h"
#include "structs.h"

//A.2.3 Statements
void verifyStatement(const g_Statement* statement) {

  /* # 3.6 Statements */
  /* ## Semantics */
  /* A *statement* specifies an action to be performed. Except as indicated, statements are executed in sequence. */
  /* A *full expression* is an expression that is not part of another expression. Each of the following is a full expression: an intializer;
   * the expression in an expression statement; the controlling expression of a selection statement (`if` or `switch`); the controlling expression of a 
   * `while` or `do` statement; each of the three (optional) expression of a `for` statement; the (optional) expression in a `return` statement. The end of a
   * full expression is a sequence point.
   */
  /* **Forward references:** expression and null statements (3.6.3), selection statements (3.6.4), iteration statements (3.6.5), the `return` statement (3.6.6.4). */

  switch (statement->type) {

    case STATEMENT_COMPOUND: verifyCompoundStatement(statement->compound.compoundStatement); break;
    case STATEMENT_EXPRESSION: verifyExpressionStatement(statement->expression.expressionStatement); break;
    case STATEMENT_JUMP: verifyJumpStatement(statement->jump.jumpStatement); break;
  }
}

//A.2.4 External Definitions
void verifyTranslationUnit(const g_TranslationUnit* translationUnit) {

  /* # 3.7 External Definitions */
  /* ## Constraints */
  /* The storage class specifiers `auto` and `register` shall not appear in the declaration specifiers in an external declaration. */
  /* There shall be no more than one external definition for each identifier declared with internal linkage in a translation unit.
   * Moreover, if an identifier declared with internal linkage is used in an expression (other than as a part of the operand of a 'sizeof' operator,
   * there shall be exactly one external definition for the identifier in the translation unit.
   *
   * NON C STANDARD:
   * 1. One file can't have two `static` storage allocating identifiers, i.e.
   *    static int foo;
   *    static float foo;
   *    foo = 1; //valid, because foo only is allocating storage once statically
   *
   *    static int foo = 1;
   *    static float foo = 1.0f; //invalid, because foo has two allocated storage space
   * 2. Referenced `static` variables need to be initialized, i.e.
   *    static int foo;
   *    print(foo); //invalid, because foo isn't initialized, and `static` variables can be externally linked
   *
   *    static int foo;
   *    sizeof(foo); //exception, because `sizeof()` doesn't need its address or value, just type
   */

   /* ## Semantics */
   /* As discussed in 2.1.1.1, the unit of program text after preprocessing is a translation unit, which consists of a sequence of external declarations.
    * These are described as "external" because they appear outside of any function (and hence have file scope). As discussed in 3.5, a declaration that
    * also causess storage to be reserved for an object or function named by the identifier is a definition.
    */
   /* An *external definition* is an external declaration that is also a definition of a function or an object. If an identifier declared with 
    * external linkage is used in an expression (other than as part of the operand of a `sizeof` operator), somewhere in the entire program there shall
    * be exactly one external definition for the identifier; otherwise, there shall be no more than one. (Footnote 78: Thus if an identifier declared
    * with external linkage is not used in an expression, there need be no external definition for it.)
    */

  verifyExternalDeclaration(translationUnit->externalDeclaration);
}

void verifyExternalDeclaration(const g_ExternalDeclaration* externalDeclaration) {

  switch (externalDeclaration->type) {

    case EXTERNAL_DECLARATION_DECLARATION: verifyDeclaration(externalDeclaration->declaration.declaration); break;
    case EXTERNAL_DECLARATION_FUNCTION: verifyFunctionDefinition(externalDeclaration->function.functionDefinition); break;
  }
}

void verifyFunctionDefinition(const g_FunctionDefinition* functionDefinition) {

  /* # 3.7.1 Function Definitions */
  /* ## Constraints */
	/* The identifier delcared in a function definition (which is the name of the function) shall have a function type, as specified by the declarator
   * portion of the function definition (Footnote 79: The intent is that the type category in a function definition cannot be inherited from a typedef:
   * ```
   * typedef int F(void);         // type `F` is "function of no arguments returning `int`" //
   * F f, g;                      // `f` and `g` both have type compatible with `F` //
   * F f { //...// }              // WRONG: syntax/constraint error //
   * F g() { //...// }            // WRONG: declares that `g` returns a function //
   * int f(void) { //...// }      // RIGHT: `f` has type compatible with `F` //
   * int g() { //...// }          // RIGHT: `g` has type compatible with `F` //
   * F *e(void) { //...// }       // `e` returns a poitner to a function //
   * F *((e)) (void) { //...// }  // same: parentheses irrelevant //
   * int (*fp) (void);            // `fp` points to a function that has type `F` //
   * F *Fp;                       // `Fp` points to a function that has type `F` //
   * ```
   * )
   */
	/* The return type of a function shall be `void` or an object type other than array */
	/* If the declarator includes a parameter type list, the declaration of each parameter shall include an identifier (except for the special case of a 
   * parameter list consisting of a single parameter of type `void`, in which ther shall not be an identifier). No declaration list shall follow
   */
	/* If the declarator includes an identifier list, each declaration in the declaration list shall have at least one declarator, and those declarators
   * shall delcare only identifiers from the identifier list. An identifier declared as a typedef name shall not be redeclared as a parameter. The declarations
   * in the declaration list shall contain no storage-class specifier other than `register` and no initializations
   */

  /* ## Semantics */
	/* The declarator in a function definition specifies the name of the function being defined and the identifiers of its parameters. If the declarator
   * includes a parameter type list, the list also specifies the types of all the parameters; such a declarator also serves as a function prototype for 
   * later calls to the same function in the same translation unit. If the declarator includes an identifier list,(Footnote 80: See "future language directions" (3.9.5)) 
   * the types of the parameters may be declared in a following declaration list. Any parameter that is not declared has type `int`.
   */
	/* If a function that accepts a variable number of arguments is defined without a parameter type list that ends with the ellipsis notation, the behavior is undefined */	
  /* On entry to the function the value of each argument expression shall be converted to the type of its corresponding parameter, as if by assignment to the parameter.
   * Array expression and function designators as argumnets are converted to pointers before the call. A declaration of a parmeter as "array of *type*" shall be adjusted
   * to "pointer to *type,*" and a declaration of a parameter as "function returning *type*" shall be adjusted to "pointer to function returning *type,*" as in 3.2.2.1.
   * The resulting parameter type shall be an object type.*/
  /* Each parameter has automatic storage duration. Its identifier is an lvalue (Footnote 81: A parameter is in effect declared at the head of the compound statment that constitutes the function body, and therefore may not be redeclared in the function body
   * (except in an enclosed block)). The layout of the storage for parameters is unspecified.*/

  /* ## Examples */
  /* ```
   * extern int max(int a, int b)
   * {
   *    return a > b ? a : b;
   * }
   * ```
   * Here `extern` is the storage class specifier and `int` is the type specifier (each of which may be omitted as those are the defaults); `max(int a, in b)`
   * is the function declarator; and
   * ```
   * { return a > b ? a : b; }
   * ```
   * is the function body. The following similar definition uses the identifier-list form for the parameter declartions:
   * ```
   * extern int max(a, b)
   * int a, b;
   * {
   *    return a > b ? a : b;
   * }
   * ```
   * Here `int a, b;` is the declaration list for the parameters, which may be omitted because those are the defaults. The difference between these two definitions is that
   * the first form acts as a prototype declaration that forces conversion of the arguments of subsequent calls to the function, whereas the second form may not.
   * Note that `f` must be declared explicitly in the calling function, as its appearance in the expression `g(f)` was not followed by `(`. Then the definition of `g`
   * might read
   * ```
   * g(int (*funcp)(void))
   * {
   *    //...// (*funcp)() // or funcp() ... //
   * }
   * ```
   * or, equivalently,
   * ```
   * g(int func(void))
   * {
   *   //...// func() // or (*func)() ... //}
   * ```
   */
}

void semanticAnalysis(const g_TranslationUnit* translationUnit) {

  verifyTranslationUnit(translationUnit);
}

//
// Created by nick on 9/11/26.
//

#include "semanticAnalyzer.h"
#include "structs.h"

/* # 3.5.7 Initialization */

/* ## Constraints */
/* There shall be no more initializers in an initializer list than there are objects to be initialized. */
/* The type of the entity to be inititalized shall be an object type or an array of unknown size. */
/* All the expressions in an initializer for an object that has static storage duration or in an initializer
 * list for an object that has aggregate or union type shall be constant expressions.
 */
/* If the declaration of an identifier has block scope and the identifier has external or internal linkage,
 * the declaration shall have no initializer for the identifier.
 */

/* ## Semantics */
/* An initializer specifies the initial value stored in an object. */
/* All unnamed structure or union members are ignored during initialization. */
/* If an object that has automatic storage duration is not initialized explicitly, its value is indeterminate.
 * (Footnote 74: Unlike in the base document, any automatic duration object may be initialized.) If an object that has
 * static storage duration is not initialized explicitly, it is initialized implicityly as if every member that has arithmatic type
 * were assigned 0 and every member that has pointer type were assigned a null pointer constant.
 */
/* The initializer for a scalar shall be single expression, optionally enclosed in braces. The initial value of the object is that
 * of the expression; the same type constraints and conversiosn as for simple assignment apply, taking the type of the scalar to be
 * the unqualified version of its declared type.
 */
/* A brace-enclosed inititalizer for a union object initializes the member that appears first in the delcaration list of the union type. */
/* The initializer for a structure or union object that has automatic storage duration either shall be an initializer list as described
 * below, or shall be a single expression that has compatible structure or union type. In the latter case, the initial value of the object
 * is that of the expression.
 */
/* The rest of this section deals with initializers for objects that have aggregate or union type*/
/* An array of character type may be initialized by a character string literal, optionally enclosed in braces. Successive characters of
 * the character string literal (including the terminating null character if there is room or if the array is of unknown size) initialize
 * the elements of the array.
 */
/* An array with element type compatible with `wchar_t` may be initialized by a wide string literal, optionally enclosed in braces.
 * Successive codes of the wide string literal (including the terminating zero-valued code if there is room or if the array is of unknown
 * size) initialize the elements of the array.
 */
/* Otherwise, the initializer for an object that has aggregate type shall be a barce enclosed list of initializers for the members of the
 * aggregate, written in increasing subscript or member order; and the initializer for an object that has union type shall be a brace-enclosed
 * initializer for the first member of the union.
 */
/* If the aggregate contains members that are aggregates or unions, or if the first member of a union is an aggregate or union, the rules apply
 * recursivley to the subaggregates or contained unions. If the initializer of a subaggregate or contained union begins with a left brace,
 * the initializers enclosed by that brace and its matching right brace initialize the members of the subaggregate or the first member of the
 * contained union. Otherwise, only enough initializers form the list are taken into account for the members of the subaggregate or the first
 * member of the contained union; any remaining initializers are left to initialize the next member of the aggregate of which the current 
 * subaggregate or contained union is a part.
 */
/* If there are fewer initializers in a brace-enclosed list than there are members of the aggregate, the remainder of the aggregate shall be
 * initialized implicitly the same object that have static storage duration.
 */
/* If an array of unknown size if initialized, its size is determined by the number of initializers provided for its elements. At the end of
 * its initializer list, the array no longer has incomplete type.
 */

/* ## Examples */
/* The declaration
 * ```
 * int x[] = { 1, 3, 5 };
 * ```
 * defines and initializes `x` as a one-dimensional array object that has three elements, as no size was specified and there are three initializers.
 */
/* ```
 * float y[4][3] = {
 *  { 1, 3, 5 },
 *  { 2, 4, 6 },
 *  { 3, 5, 7 },
 * };
 * ```
 * is a definition with a fully bracketed initialization: 1, 3, and 5 initialize the first row of `y` (the array object `y[0]`), namely `y[0][0]`,
 * `y[0][1]`, and `y[0][2]`. Likewise the next two lines initialize `y[1]` and `y[2]`. The initializer ends early, so `y[3]` is initialized with zeros.
 * Precisely the same effect could have been achieved by
 * ```
 * float y[4][3] = {
 *  1, 3, 5, 2, 4, 6, 3, 5, 7
 * };
 * ```
 * The initializer for `y[0]` does not begin with a left brace, so three items from the list are used. Likewise the next three are taken successively
 * for `y[1]` and `y[2]`. Also
 * ```
 * float z[4][3] = {
 *  { 1 } , { 2 }, { 3 }, { 4 }
 * };
 * initializes the first column of `z` as specified and initializes the rest with zeros.
 */
/* ```
 * struct { int a[3], b; } w[] = { { 1 }, 2 };
 * ```
 * is a definition with an inconsistently bracketed initialization. It defines an array with two element structures: `w[0].a[0]` is 1 and `w[1].a[0]`
 * is 2; all the other elements are zero.
 */
/* The declaration
 * ```
 * short q[4][3][2] = {
 *  { 1 },
 *  { 2, 3 },
 *  { 4, 5, 6 }
 * };
 * ```
 * contains an incompletely but consistently bracketed initialization. It defines a three dimensional array object: `q[0][0][0]` is 1, `q[1][0][0]` is 2,
 * `q[1][0][1]` is 3, and 4, 5, and 6 initialize `q[2][0][0]`, respectively; all the rest are zero. The initializer for `q[0][0]` does not being with a left brace,
 * so up to six items from the current list may be used. There is only one, so the values for the remaining five elements are initialized with zero.
 * Likewise, the initializers for `q[1][0]` and `q[2][0]` do not begin with a left brace, so each uses up to six items, initializing their respective
 * two-dimensional subaggregates. If there had been more than six items in any of the lists, a diagnostic message would have been issued. 
 * The same initialization result could have been achieved by:
 * ```
 * short q[4][3][2] = {
 *  1, 0, 0, 0, 0, 0, 0,
 *  2, 3, 0, 0, 0, 0, 0,
 *  4, 5, 6
 * };
 * ```
 * or by:
 * ```
 * short q[4][3][2] = {
 *  {
 *    { 1 },
 *  },
 *  {
 *    { 2, 3 },
 *  },
 *  {
 *    { 4, 5 },
 *    { 6 },
 *  }
 * };
 * ```
 * is a fully bracketed form.
 */
/* Note that the fully bracketed an minimally bracketed forms of initialization are, in general, less likely to cause confusion. */
/* One form of initialization that completes array types involves typedef names. Given the declaration
 * ```
 * typedef int A[];
 * ```
 * the declaration
 * ```
 * A a = {1, 2}, b = {3, 4, 5};
 * ```
 * is identical to
 * ```
 * int a[] = {1, 2}, b[] = {3, 4, 5};
 * ```
 * due to the rules for incomplete types.
 */
/* Finally, the declaration
 * ```
 * char s[] = "abc", t[3] = "abc";
 * ```
 * defines "plain" `char` array objects `s` and `t` whose elements are initialized with character string literals. This delcaration is identical to
 * ```
 * char s[] = { 'a', 'b', 'c', '\0' },
 *      t[] = { 'a', 'b', 'c' };
 * ```
 * The contents of the arrays are modifiable. On the other hand, the declaration
 * ```
 * char *p = "abc";
 * ```
 * defines `p` with type "pointer to `char`" that is initialized to point to an object with type "array of `char`" with length 4 whose elements
 * are initialized with a character string literal. If an attempt is made to use `p`  to modify the contents of the array, the behavior is undefined.
 */
/* **Forward references:** common definitions `<stddef.h>` (4.1.5). */

void verifyInitializer(const g_Initializer* initializer) {

}
void verifyInitializerList(const g_InitializerList* initializerList) {

}

void verifyCompoundStatement(const g_CompoundStatement* compoundStatement);
void verifyExpressionStatement(const g_ExpressionStatement* expressionStatement);
void verifyJumpStatement(const g_JumpStatement* jumpStatement);
void verifyStatement(const g_Statement* statement) {

  /* # 3.6 Statements */
  /* ## Semantics */
  /* A *statement* specifies an action to be performed. Except as indicated, statements are executed in sequence. */
  /* A *full expression* is an expression that is not part of another expression. Each of the following is a full expression: an intializer;
   * the expression in an expression statement; the controlling expression of a selection statement (`if` or `switch`); the controlling expression of a 
   * `while` or `do` statement; each of the three (optional) expressions of a `for` statement; the (optional) expression in a `return` statement. The end of a
   * full expression is a sequence point.
   */
  /* **Forward references:** expression and null statements (3.6.3), selection statements (3.6.4), iteration statements (3.6.5), the `return` statement (3.6.6.4). */

  switch (statement->type) {

    case STATEMENT_COMPOUND: verifyCompoundStatement(statement->compound.compoundStatement); break;
    case STATEMENT_EXPRESSION: verifyExpressionStatement(statement->expression.expressionStatement); break;
    case STATEMENT_JUMP: verifyJumpStatement(statement->jump.jumpStatement); break;
  }
}

void verifyDeclarationList(const g_DeclarationList* declarationList);
void verifyStatementList(const g_StatementList* statementList);
void verifyCompoundStatement(const g_CompoundStatement* compoundStatement) {

  /* # 3.6.2 Compound Statement, or Block */
  /* ## Semantics */
  /* A *compound statement* (also called a *block*) allows a set of statements to be grouped into one syntactic unit, which may have its own set of declarations and initializations
   * (as discuessed in 3.1.2.4). The inititalizers of objects that have automatic storage duration are evaluated and the values are stored in the objects in the order their
   * declarators appear in the translation unit.
   */

  if (compoundStatement->declarationList) { verifyDeclarationList(compoundStatement->declarationList); }
  if (compoundStatement->statementList) { verifyStatementList(compoundStatement->statementList); }
}

void verifyStatement(const g_Statement* statement);
void verifyStatementList(const g_StatementList* statementList) {

  if (statementList->statementList) { verifyStatementList(statementList->statementList); }
  else { verifyStatement(statementList->statement); }
}

void verifyDeclaration(const g_Declaration*);
void verifyDeclarationList(const g_DeclarationList* declarationList) {

  if (declarationList->declarationList) { verifyDeclarationList(declarationList->declarationList); }
  else { verifyDeclaration(declarationList->declaration); }
}

void verifyExpressionStatement(const g_ExpressionStatement* expressionStatement) {

  /* # 3.6.3 Expression and Null Statements */
  /* ## Semantics */
  /* The expression in an expression statement is evaluated as a void expression for it side effects. (Footnote 75: Such as assignments, and function calls which have side effects.)*/
  /* A *null statement* (consisting of just a semicolon) performs no operations. */

  /* ## Examples */
  /* If a function call is evalutaed as an expression statement for its side effects only, the discarding of its value may be made explicit by converting the expression to a
   * void expression by means of a cast:
   * ```
   * int p(int);
   * //...//
   * (void)p(0);
   * ```
   * In the program fragment
   * ```
   * char *s ;
   * //...//
   * while (*s++ != '\0')
   *       ;
   * ```
   * a null statement is used to supply an empty loop body to the iteration statement.
   */
  /* A null statement may also be used to carry a label just before the closing `}` of a compound statement.
   * ```
   * while (loop1) {
   *       //...//
   *       while (loop2) {
   *             //...//
   *             if (want_out)
   *                   goto end_loop1;
   *             //...//
   *       }
   *       //...//
   * end_loop1: ;
   * }
   * ```
   */
  /* **Forward References:** iteration statements (3.6.5). */

  if (expressionStatement->expression) { verifyExpression(expressionStatement->expression); }
}

void verifyJumpStatement(/* @OMIT g_Context context, */const g_JumpStatement* jumpStatement) {

  /* # 3.6.6 Jump Statements */
  /* ## Semantics */
  /* A jump statement causes and unconditional jump to another place. */

  switch (jumpStatement->type) {

    case JUMP_RETURN :

    /* # 3.6.6.4 The `return` Statement */
    /* ## Constraints */
    /* A `return` statement with an expression shall not appear in a function whose return type is `void`. */
    /* @OMIT if(// check return type is void // && jumpStatement->b_return.expression) { die("return expression in function returning void"); } */

    /* ## Semantics */
    /* A `return` statement terminates execution of the current function and returns control to its caller.
     * A function may have any number of `return` statements, with and without expressions.
     */
    /* If a `return` statement with an expression is executed, the value of the expression is returned to the caller
     * as the value of the function call expression. If the expression has a type different from that of the function
     * in which it appears, it is converted as if it were assigned to an object of that type.
     */  
    /* If a `return` statement without an expression is executed, and the value of the function call is used by the caller,
     * the behvaior is undefined. Reaching the `}` that terminates a function is equivalent to executing a return statement
     * without an expression.
     */

    verifyExpression(jumpStatement->b_return.expression);
  }
}

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

void verifyExternalDeclaration(const g_ExternalDeclaration* externalDeclaration);
void verifyTranslationUnit(const g_TranslationUnit* translationUnit) {

  if (translationUnit->translationUnit) { verifyTranslationUnit(translationUnit->translationUnit); }
  else { verifyExternalDeclaration(translationUnit->externalDeclaration); }
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
   * parameter list consisting of a single parameter of type `void`, in which there shall not be an identifier). No declaration list shall follow.
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
  /* Each parameter has automatic storage duration. Its identifier is an lvalue (Footnote 81: A parameter is in effect declared at the head of the compound statment that constitutes the function body, 
   *  and therefore may not be redeclared in the function body (except in an enclosed block)). The layout of the storage for parameters is unspecified.
   */

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
   * 
   * To pass one function to another, one might say
   * ```
   * int f(void);
   * // ... //
   * g(f);
   * ```
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

  if (functionDefinition->declarationSpecifiers) { verifyDeclarationSpecifiers(functionDefinition->declarationSpecifiers); }
  if (!functionDefinition->declarator) { die("Expected declarator for function definition"); }
  else { verifyDeclarator(functionDefinition->declarator); }
  if (functionDefinition->declarationList) { verifyDeclarationList(functionDefinition->declarationList); }
  if (!functionDefinition->compoundStatement) { die("Expected compound statement for function definition"); }
  else { verifyCompoundStatement(functionDefinition->compoundStatement); }
}

void verifyExternalDeclaration(const g_ExternalDeclaration* externalDeclaration) {

  /* # 3.7.2 External Object Definitions */
  
  /* ## Semantics */
  /* If the declaration of an identifier for an object has file scope and an initializer, the declaration is an external definition for the identifier. */
  /* A declaration of an identifier for an object that has file scope without an initializer, and without a storage-class specifier or with the storage-class
   * specifier `static`, constitutes a *tentative definition*. If a translation unit contains one or more tentative definitions for an identifier, and the 
   * translation unit contains no external definition for that identifier, then the behavior is exactly as if the translation unit contains file scope declaration
   * of that identifier, with the composite type as of the end of the translation unit, with an initializer equal to 0.
   */
  /* If the declaration of an identifier for an object is a tentative definition and has internal linkage, the declared type shall not be an incomplete type. */

  /* ## Examples */
  /* ```
   * int i1 = 1;        // definition, external linkage //
   * static int i2 = 2; // definition, internal linkage //
   * extern int i3 = 3; // definition, external linkage //
   * int i4;            // tentative definition, external linkage //
   * static int i5;     // tentative definition, internal linkage //
   * 
   * int i1;  // valid tentative definition, refers to previous //
   * int i2;  // 3.1.2.2 renders undefined, linkage disagreement //
   * int i3;  // valid tentative definition, refers to previous //
   * int i4;  // valid tentative definition, refers to previous //
   * int i5;  // 3.1.2.2 renders undefined, linkage disagreement //
   * 
   * extern int i1; // refers to previous, whose linkage is external //
   * extern int i2; // refers to previous, whose linkage is internal //
   * extern int i3; // refers to previous, whose linkage is external //
   * extern int i4; // refers to previous, whose linkage is external //
   * extern int i5; // refers to previous, whose linkage is internal //
  */

  switch (externalDeclaration->type) {

    case EXTERNAL_DECLARATION_DECLARATION: verifyDeclaration(externalDeclaration->declaration.declaration); break;
    case EXTERNAL_DECLARATION_FUNCTION: verifyFunctionDefinition(externalDeclaration->function.functionDefinition); break;
  }
}

void semanticAnalysis(const g_TranslationUnit* translationUnit) {

  verifyTranslationUnit(translationUnit);
}

//
// Created by nick on 9/11/26.
//

#include "semanticAnalyzer.h"
#include "structs.h"

void verifyFunctionDefinition(const g_FunctionDefinition* functionDefinition) {

	/* The identifier delcared in a function definition (which is the name of the function) shall have a function type, as specified by the declarator
   * portion of the function definition
   */
	/* The return type of a function shall be 'void' or an object type other than array */
	/* If the declarator includes a parameter type list, the declaration of each parameter shall include an identifier (except for the special case of a 
   * parameter list consisting of a single parameter of type 'void', in which ther shall not be an identifier). No declaration list shall follow
   */
	/* If the declarator includes an identifier list, each declaration in the declaration list shall have at least one declarator, and those declarators
   * shall delcare only identifiers from the identifier list. An identifier declared as a typedef name shall not be redeclared as a parameter. The declarations
   * in the declaration list shall contain no storage-class specifier other than 'register' and no initializations
   */
	/* The declarator in a function definition specifies the name of the function being defined and the identifiers of its parameters. If the declarator
   * includes a parameter type list, the list also specifies the types of all the parameters; such a declarator also serves as a function prototype for 
   * later calls to the same function in the same translation unit. If the declarator includes an identifier list, the types of the parameters may be declared
   * in a following declaration list. Any parameter that is not declared has type 'int'
   */
	/* If a function that accepts a variable number of arguments is defined without a parameter type list that ends with the ellipsis notation, the behavior is undefined *	

}

void verifyExternalDeclaration(const g_ExternalDeclaration* externalDeclaration) {

  /* The storage class specifiers 'auto' and 'register' shall not appear in the declaration specifiers in an external declaration */
  /* There shall be no more than one external definition for each identifier declared with internal linkage in a translation unit.
   * Moreover, if an identifier declared with internal linkage is used in an expression (other than as a part of the operand of a 'sizeof' operator,
   * there shall be exactly one external definition for the identifier in the translation
   *
   * 1. One file can't have two 'static' storage allocating identifiers, i.e.
   *    static int foo;
   *    static float foo;
   *    foo = 1; //valid, because foo only is allocating storage once statically
   *
   *    static int foo = 1;
   *    static float foo = 1.0f; //invalid, because foo has two allocated storage space
   * 2. Referenced 'static' variables need to be initialized, i.e.
   *    static int foo;
   *    print(foo); //invalid, because foo isn't initialized, and 'static' variables can be externally linked
   *
   *    static int foo;
   *    sizeof(foo); //exception, because 'sizeof()' doesn't need its address or value, just type
   */

  switch (externalDeclaration->type) {

    case EXTERNAL_DECLARATION_DECLARATION: verifyDeclaration(externalDeclaration->declaration.declaration); break;
    case EXTERNAL_DECLARATION_FUNCTION: verifyFunctionDefinition(externalDeclaration->function.functionDefinition); break;
  }
}

void verifyTranslationUnit(const g_TranslationUnit* translationUnit) {

  verifyExternalDeclaration(translationUnit->externalDeclaration);
}

void semanticAnalysis(const g_TranslationUnit* translationUnit) {

  verifyTranslationUnit(translationUnit);
}

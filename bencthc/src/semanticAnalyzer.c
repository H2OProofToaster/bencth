//
// Created by nick on 9/11/26.
//

#include "semanticAnalyzer.h"
#include "structs.h"

void verifyFunctionDefinition(const g_FunctionDefinition* functionDefinition) {


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
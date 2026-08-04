//
// Created by nick on 7/13/26.
//

#ifndef BENCTH_PARSER_H
#define BENCTH_PARSER_H

#include "bencthc/src/scanner.h"

Parser* parse(const Scanner* s);
void printProgram(const Program* p);

#endif //BENCTH_PARSER_H
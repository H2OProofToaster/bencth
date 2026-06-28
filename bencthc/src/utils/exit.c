//
// Created by nick on 6/27/26.
//

#include "exit.h"
#include <stdio.h>
#include <stdlib.h>

void die(const char* err) {

  fprintf(stderr, "%s", err);
  exit(-1);
}
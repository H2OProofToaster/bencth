//
// Created by nick on 6/26/26.
//

#include "bencthc/src/utils/file.h"
#include "bencthc/src/utils/print.h"

#include <stdlib.h>

int main() {

  const int f = b_fopen("bencthc/tests/hello.txt");

  char* data = b_fread(f);

  b_fclose(f);

  b_printString(data);

  free(data);

  return 0;
}

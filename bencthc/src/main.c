//
// Created by nick on 6/26/26.
//

#include "bencthc/src/utils/file.h"
#include "bencthc/src/utils/print.h"
#include "bencthc/src/utils/exit.h"
#include "bencthc/src/utils/allocator.h"

#include <stdlib.h>

int main() {

  Arena* a = b_allocArena();

  const int f = b_fopen("bencthc/tests/hello.txt");

  Arena* data = b_fread(f);

  b_fclose(f);

  b_printString((char*)data);

  free(data);

  return 0;
}

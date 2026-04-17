#include <stdio.h>

#include "../project.h"

// wrapper
#include "shop.c"
// database
#include "bst.c"
#include "revenue.c"
// io
#include "io_binary.c"
#include "io_plain.c"
// other
#include "debug.c"
#include "legacy.c"

int main(void) {
  ErrorCode err;
  Shop* shop = read_plaintext("../pelit", &err);
  if (!shop) {
    printError(err);
    return 1;
  }

  printf("Loaded shop:\n");
  printShop(shop);

  printf("Revenue list:\n");
  printRevenue(shop);

  freeShop(shop);
  return 0;
}
#include <stdio.h>

#include "../project.h"

// wrapper
#include "../sub_files/shop.c"
// database
#include "../sub_files/bst.c"
#include "../sub_files/revenue.c"
// io
#include "../sub_files/io_binary.c"
#include "../sub_files/io_plain.c"
// other
#include "../sub_files/debug.c"
#include "../sub_files/legacy.c"

int main(void) {
  ErrorCode err;
  Shop* shop = io_text_read("../pelit", &err);
  if (!shop) {
    printError(err);
    return 1;
  }

  printf("Loaded shop:\n");
  print_shop(shop);

  printf("Revenue list:\n");
  print_revenue(shop);

  free_shop(shop);
  return 0;
}
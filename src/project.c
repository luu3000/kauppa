#include "project.h"

// wrapper
#include "sub_files/shop.c"
// database
#include "sub_files/bst.c"
#include "sub_files/revenue.c"
// io
#include "sub_files/io_binary.c"
#include "sub_files/io_plain.c"
// other
#include "sub_files/debug.c"
#include "sub_files/legacy.c"

int main(void) {
  Shop* shop = (Shop*)malloc(sizeof(Shop));
  if (shop == NULL) {
    printf("ERROR: Failed to allocate database.\n");
    return 1;
  }
  initShop(shop);

  int loop = true;
  char* command = malloc(1000);  // guideline 1.1.I
  char filename[100];
  ErrorCode err;

  while (loop) {
    if (!fgets(command, 1000, stdin)) {
      break;
    }
    switch (command[0]) {
      case 'A': {
        char* name = malloc(1000);
        double price;
        if (sscanf(command + 1, " %999s %lf", name, &price) != 2) {
          printf("Invalid command A\n");
          free(name);
          break;
        }
        name = strcat(name, ".txt");
        err = addGame(shop, name, price);
        if (err != SUCCESS) {
          printError(err);
        } else {
          printf("SUCCESS\n");
        }
        free(name);
        break;
      }
      case 'B': {
        char* name = malloc(1000);
        int count;
        if (sscanf(command + 1, " %99s %d", name, &count) != 2) {
          printf("Invalid command B\n");
          free(name);
          break;
        }
        name = strcat(name, ".txt");
        err = buyGame(shop, name, count);
        if (err != SUCCESS) {
          printError(err);
        } else {
          printf("SUCCESS\n");
        }
        free(name);
        break;
      }
      case 'L':
        printRevenue(shop);
        printf("SUCCESS\n");
        break;
      case 'W':
        if (sscanf(command + 1, " %99s", filename) != 1) {
          printf("Invalid command W\n");
          break;
        }
        err = write_plaintext(filename, shop);
        if (err != SUCCESS) {
          printError(err);
        } else {
          printf("SUCCESS\n");
        }
        break;
      case 'O': {
        if (sscanf(command + 1, " %99s", filename) != 1) {
          printf("Invalid command O\n");
          break;
        }
        Shop* new_shop = read_plaintext(filename, &err);
        if (err != SUCCESS || new_shop == NULL) {
          printError(err);
          break;
        }
        freeShop(shop);
        shop = new_shop;
        printf("SUCCESS\n");
        break;
      }
      case 'Q':
        loop = false;
        break;
    }
  }

  freeShop(shop);
  free(command);
  printf("SUCCESS\n");
  return 0;
}

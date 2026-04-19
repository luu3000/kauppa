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
  init_shop(shop);

  int loop = true;
  char command[MAX_STRING_LENGTH];  // guideline 1.1.I
  char filename[MAX_STRING_LENGTH];
  ErrorCode err;

  while (loop) {
    if (!fgets(command, MAX_STRING_LENGTH, stdin)) {
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
        err = shop_add_game(shop, name, price);
        if (err != SUCCESS) {
          debug_print_error(err);
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
        err = shop_buy_game(shop, name, count);
        if (err != SUCCESS) {
          debug_print_error(err);
        } else {
          printf("SUCCESS\n");
        }
        free(name);
        break;
      }
      case 'L':
        print_revenue(shop);
        printf("SUCCESS\n");
        break;
      case 'W':
        if (sscanf(command + 1, " %99s", filename) != 1) {
          printf("Invalid command W\n");
          break;
        }
        strcat(filename, ".txt");
        err = io_text_write(filename, shop);
        if (err != SUCCESS) {
          debug_print_error(err);
        } else {
          printf("SUCCESS\n");
        }
        break;
      case 'O': {
        if (sscanf(command + 1, " %99s", filename) != 1) {
          printf("Invalid command O\n");
          break;
        }
        strcat(filename, ".txt");
        Shop* new_shop = io_text_read(filename, &err);
        if (err != SUCCESS || new_shop == NULL) {
          debug_print_error(err);
          break;
        }
        free_shop(shop);
        shop = new_shop;
        printf("SUCCESS\n");
        break;
      }
      case 'Q':
        loop = false;
        break;
    }
  }

  free_shop(shop);
  printf("SUCCESS\n");
  return 0;
}

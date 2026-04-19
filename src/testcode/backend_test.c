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
  printf("=== Testing Shop Implementations ===\n");

  // Test 1: Basic shop operations
  printf("\n1. Basic Shop Initialization and Game Addition\n");
  Shop* shop = (Shop*)malloc(sizeof(Shop));
  if (shop == NULL) {
    printf("ERROR: Failed to allocate shop\n");
    return 1;
  }
  init_shop(shop);

  // Add games
  ErrorCode add_err;
  add_err = shop_add_game(shop, "Tetris", 9.99);
  printf("Add Tetris: %s\n", add_err == SUCCESS ? "SUCCESS" : "FAILED");

  add_err = shop_add_game(shop, "Wormgame", 4.50);
  printf("Add Wormgame: %s\n", add_err == SUCCESS ? "SUCCESS" : "FAILED");

  // Test duplicate addition
  add_err = shop_add_game(shop, "Tetris", 9.99);  // Should fail
  printf("Add duplicate Tetris: %s (expected failure)\n",
         add_err == GAME_IN_SYSTEM ? "SUCCESS (duplicate rejected)" : "FAILED");

  // Test buying games
  printf("\n2. Buying Games and Revenue Updates\n");
  ErrorCode buy_err;
  buy_err = shop_buy_game(shop, "Tetris", 3);
  printf("Buy Tetris x3: %s\n", buy_err == SUCCESS ? "SUCCESS" : "FAILED");

  buy_err = shop_buy_game(shop, "NonExistent", 1);  // Should fail
  printf("Buy non-existent game: %s (expected failure)\n",
         buy_err == NOT_FOUND ? "SUCCESS (not found)" : "FAILED");

  // Print current state
  printf("\n3. Printing Functions\n");
  printf("Shop contents:\n");
  debug_print_shop(shop);
  printf("\nRevenue list:\n");
  print_revenue(shop);

  // Test I/O operations
  printf("\n4. Plaintext I/O\n");
  ErrorCode write_err = io_text_write("kauppa.txt", shop);
  printf("Write plaintext: %s\n", write_err == SUCCESS ? "SUCCESS" : "FAILED");

  free_shop(shop);

  ErrorCode read_err;
  shop = io_text_read("kauppa.txt", &read_err);
  printf("Read plaintext: %s\n", read_err == SUCCESS ? "SUCCESS" : "FAILED");
  debug_print_error(read_err);

  if (shop) {
    printf("Re-read shop contents:\n");
    debug_print_shop(shop);
    printf("\nRe-read revenue:\n");
    print_revenue(shop);
  }

  free_shop(shop);

  shop = io_binary_read("kauppa", &read_err);
  printf("Read binary: %s\n", read_err == SUCCESS ? "SUCCESS" : "FAILED");

  if (shop) {
    printf("Binary-read shop contents:\n");
    debug_print_shop(shop);
    printf("\nBinary-read revenue:\n");
    print_revenue(shop);
  }
  // Test edge cases
  printf("\n6. Edge Cases\n");
  // Empty shop
  Shop* empty_shop = (Shop*)malloc(sizeof(Shop));
  init_shop(empty_shop);
  printf("Empty shop print:\n");
  debug_print_shop(empty_shop);
  printf("Empty shop revenue:\n");
  print_revenue(empty_shop);
  free_shop(empty_shop);

  // Test individual functions
  printf("\n7. Individual Function Tests\n");
  Game* test_game = create_game("TestGame", 10.0, &add_err);
  if (test_game) {
    printf("Created game: %s $%.2f\n", test_game->name, test_game->price);
    free_game(test_game);
    printf("Freed game: SUCCESS\n");
  } else {
    printf("Create game: FAILED\n");
  }

  // Test tree size
  if (shop) {
    size_t size = bst_size(shop->root);
    printf("Tree size: %zu nodes\n", size);
  }

  // Cleanup
  if (shop) free_shop(shop);

  printf("\n=== All Tests Completed ===\n");
  return 0;
}

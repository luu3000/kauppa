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

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Test macros
#define TEST_START(name) printf("\n=== Testing %s ===\n", name);

#define TEST_ASSERT(condition, msg)  \
  do {                               \
    tests_run++;                     \
    if (condition) {                 \
      printf("  ✓ PASS: %s\n", msg); \
      tests_passed++;                \
    } else {                         \
      printf("  ✗ FAIL: %s\n", msg); \
      tests_failed++;                \
    }                                \
  } while (0)

#define TEST_ASSERT_ERR(expected, actual, msg)                              \
  do {                                                                      \
    tests_run++;                                                            \
    if (expected == actual) {                                               \
      printf("  ✓ PASS: %s (err=%d)\n", msg, actual);                       \
      tests_passed++;                                                       \
    } else {                                                                \
      printf("  ✗ FAIL: %s - expected err=%d, got err=%d\n", msg, expected, \
             actual);                                                       \
      tests_failed++;                                                       \
    }                                                                       \
  } while (0)

#define TEST_SUMMARY()                                                     \
  printf("\n=== Test Summary ===\n");                                      \
  printf("Total: %d | Passed: %d | Failed: %d\n", tests_run, tests_passed, \
         tests_failed);

// Helper function to count games in shop
static int count_games_in_shop(const Shop* shop) {
  return bst_size(shop->root);
}

// Helper function to count nodes in revenue list
static int count_revenue_nodes(const Node* head) {
  int count = 0;
  while (head) {
    count++;
    head = head->next;
  }
  return count;
}

// Helper to find a game in BST
static Game* find_game_in_bst(Vertex* root, const char* name) {
  if (!root) return NULL;
  int cmp = strcmp(name, root->game->name);
  if (cmp == 0) return root->game;
  if (cmp < 0) return find_game_in_bst(root->left, name);
  return find_game_in_bst(root->right, name);
}

void validate_bst(Vertex* root) {
  if (!root) return;

  if (!root->game) {
    printf("ERROR: Vertex at %p has NULL game\n", root);
    return;
  }

  if (!root->game->name) {
    printf("ERROR: Game at %p has NULL name\n", root->game);
    return;
  }

  printf("Valid node: '%s'\n", root->game->name);
  validate_bst(root->left);
  validate_bst(root->right);
}

// Test 1: Game creation and destruction
void test_game_creation(void) {
  TEST_START("Game Creation");

  ErrorCode err;

  // Valid game
  Game* game = create_game("TestGame", 19.99, &err);
  TEST_ASSERT(game != NULL, "Create valid game");
  TEST_ASSERT(err == SUCCESS, "Error code is SUCCESS");
  TEST_ASSERT(strcmp(game->name, "TestGame") == 0, "Game name correct");
  TEST_ASSERT(game->price == 19.99, "Game price correct");
  TEST_ASSERT(game->revenue == 0.0, "Game revenue initialized to 0");

  free_game(game);

  // Invalid: NULL name
  game = create_game(NULL, 10.0, &err);
  TEST_ASSERT(game == NULL, "Create game with NULL name fails");
  TEST_ASSERT_ERR(INVALID_ARGUMENT, err, "Returns INVALID_ARGUMENT");

  // Invalid: name too long
  char long_name[MAX_STRING_LENGTH + 10];
  memset(long_name, 'A', MAX_STRING_LENGTH + 5);
  long_name[MAX_STRING_LENGTH + 5] = '\0';
  game = create_game(long_name, 10.0, &err);
  TEST_ASSERT(game == NULL, "Create game with too long name fails");
  TEST_ASSERT_ERR(INVALID_ARGUMENT, err, "Returns INVALID_ARGUMENT");
}

// Test 2: Shop initialization
void test_shop_init(void) {
  TEST_START("Shop Initialization");

  Shop* shop = malloc(sizeof(Shop));
  TEST_ASSERT(shop != NULL, "Allocate shop");

  init_shop(shop);
  TEST_ASSERT(shop->root == NULL, "Shop root is NULL");
  TEST_ASSERT(shop->revenue == NULL, "Shop revenue is NULL");

  free_shop(shop);
}

// Test 3: Adding games
void test_add_games(void) {
  TEST_START("Adding Games");

  Shop* shop = malloc(sizeof(Shop));
  init_shop(shop);
  ErrorCode err;

  // Add first game
  err = shop_add_game(shop, "Zelda", 59.99);
  TEST_ASSERT_ERR(SUCCESS, err, "Add first game");
  TEST_ASSERT(count_games_in_shop(shop) == 1, "Shop has 1 game");
  TEST_ASSERT(count_revenue_nodes(shop->revenue) == 1,
              "Revenue list has 1 node");

  // Add second game
  err = shop_add_game(shop, "Mario", 49.99);
  TEST_ASSERT_ERR(SUCCESS, err, "Add second game");
  TEST_ASSERT(count_games_in_shop(shop) == 2, "Shop has 2 games");
  TEST_ASSERT(count_revenue_nodes(shop->revenue) == 2,
              "Revenue list has 2 nodes");

  // Add third game
  err = shop_add_game(shop, "Sonic", 39.99);
  TEST_ASSERT_ERR(SUCCESS, err, "Add third game");
  TEST_ASSERT(count_games_in_shop(shop) == 3, "Shop has 3 games");

  // Try to add duplicate
  err = shop_add_game(shop, "Zelda", 59.99);
  TEST_ASSERT_ERR(GAME_IN_SYSTEM, err, "Add duplicate game fails");
  TEST_ASSERT(count_games_in_shop(shop) == 3, "Shop still has 3 games");

  free_shop(shop);
}

// Test 4: Buying games
void test_buy_games(void) {
  TEST_START("Buying Games");

  Shop* shop = malloc(sizeof(Shop));
  init_shop(shop);
  ErrorCode err;

  // Setup
  shop_add_game(shop, "Game1", 10.0);
  shop_add_game(shop, "Game2", 20.0);
  shop_add_game(shop, "Game3", 30.0);

  // Call before search
  printf("BST contents:\n");
  validate_bst(shop->root);

  // Buy existing game
  err = shop_buy_game(shop, "Game1", 3);
  TEST_ASSERT_ERR(SUCCESS, err, "Buy existing game");

  Game* game1 = find_game_in_bst(shop->root, "Game1");
  TEST_ASSERT(game1 != NULL, "Found Game1");
  TEST_ASSERT(game1->revenue == 30.0, "Game1 revenue updated to 30.0");

  // Buy another game
  err = shop_buy_game(shop, "Game2", 5);
  TEST_ASSERT_ERR(SUCCESS, err, "Buy Game2");
  Game* game2 = find_game_in_bst(shop->root, "Game2");
  TEST_ASSERT(game2->revenue == 100.0, "Game2 revenue updated to 100.0");

  // Buy non-existent game
  err = shop_buy_game(shop, "NonExistent", 1);
  TEST_ASSERT_ERR(NOT_FOUND, err, "Buy non-existent game fails");

  // Buy with multiple purchases
  err = shop_buy_game(shop, "Game1", 2);
  TEST_ASSERT_ERR(SUCCESS, err, "Buy Game1 again");
  TEST_ASSERT(game1->revenue == 50.0, "Game1 revenue accumulated to 50.0");

  // Verify revenue list order (highest revenue first)
  TEST_ASSERT(shop->revenue->game == game2, "Highest revenue game is first");
  TEST_ASSERT(shop->revenue->next->game == game1, "Second highest is next");

  free_shop(shop);
}

// Test 5: Plaintext I/O
void test_plaintext_io(void) {
  TEST_START("Plaintext I/O");

  const char* filename = "test_plaintext.txt";
  ErrorCode err;

  // Create and populate shop
  Shop* shop1 = malloc(sizeof(Shop));
  init_shop(shop1);
  shop_add_game(shop1, "Alpha", 10.0);
  shop_add_game(shop1, "Beta", 20.0);
  shop_add_game(shop1, "Gamma", 30.0);
  shop_buy_game(shop1, "Alpha", 5);  // revenue = 50
  shop_buy_game(shop1, "Beta", 2);   // revenue = 40

  // Write to file
  err = io_text_write(filename, shop1);
  TEST_ASSERT_ERR(SUCCESS, err, "Write shop to plaintext file");

  // Read back
  Shop* shop2 = io_text_read(filename, &err);
  TEST_ASSERT_ERR(SUCCESS, err, "Read shop from plaintext file");
  TEST_ASSERT(shop2 != NULL, "Shop read successfully");

  // Verify contents
  TEST_ASSERT(count_games_in_shop(shop2) == 3, "Read shop has 3 games");

  Game* alpha = find_game_in_bst(shop2->root, "Alpha");
  TEST_ASSERT(alpha != NULL, "Alpha found in read shop");
  TEST_ASSERT(alpha->price == 10.0, "Alpha price correct");
  TEST_ASSERT(alpha->revenue == 50.0, "Alpha revenue correct");

  Game* beta = find_game_in_bst(shop2->root, "Beta");
  TEST_ASSERT(beta != NULL, "Beta found in read shop");
  TEST_ASSERT(beta->revenue == 40.0, "Beta revenue correct");

  // Verify revenue list order
  TEST_ASSERT(count_revenue_nodes(shop2->revenue) == 3,
              "Revenue list has 3 nodes");

  TEST_ASSERT(
      shop2->revenue->game->revenue >= shop2->revenue->next->game->revenue,
      "Revenue list is sorted");

  // Cleanup
  free_shop(shop1);
  free_shop(shop2);
  remove(filename);
}

// Test 6: Binary I/O
void test_binary_io(void) {
  TEST_START("Binary I/O");

  const char* filename = "test_binary.dat";
  ErrorCode err;

  // Create and populate shop
  Shop* shop1 = malloc(sizeof(Shop));
  init_shop(shop1);
  shop_add_game(shop1, "Delta", 15.0);
  shop_add_game(shop1, "Epsilon", 25.0);
  shop_add_game(shop1, "Zeta", 35.0);
  shop_buy_game(shop1, "Delta", 4);    // revenue = 60
  shop_buy_game(shop1, "Epsilon", 3);  // revenue = 75

  // Write to file
  err = io_binary_write(filename, shop1);
  TEST_ASSERT_ERR(SUCCESS, err, "Write shop to binary file");

  // Read back
  Shop* shop2 = io_binary_read(filename, &err);
  TEST_ASSERT_ERR(SUCCESS, err, "Read shop from binary file");
  TEST_ASSERT(shop2 != NULL, "Shop read successfully");

  // Verify contents
  TEST_ASSERT(count_games_in_shop(shop2) == 3, "Read shop has 3 games");

  Game* delta = find_game_in_bst(shop2->root, "Delta");
  TEST_ASSERT(delta != NULL, "Delta found in read shop");
  TEST_ASSERT(delta->price == 15.0, "Delta price correct");
  TEST_ASSERT(delta->revenue == 60.0, "Delta revenue correct");

  Game* epsilon = find_game_in_bst(shop2->root, "Epsilon");
  TEST_ASSERT(epsilon != NULL, "Epsilon found in read shop");
  TEST_ASSERT(epsilon->revenue == 75.0, "Epsilon revenue correct");

  // Verify revenue list order TODO:
  // TEST_ASSERT(shop2->revenue->game == epsilon, "Epsilon (75) is first");
  // TEST_ASSERT(shop2->revenue->next->game == delta, "Delta (60) is second");

  // Cleanup
  free_shop(shop1);
  free_shop(shop2);
  remove(filename);
}

// Test 7: Empty shop operations
void test_empty_shop(void) {
  TEST_START("Empty Shop Operations");

  Shop* shop = malloc(sizeof(Shop));
  init_shop(shop);
  ErrorCode err;

  // Buy from empty shop
  err = shop_buy_game(shop, "Anything", 1);
  TEST_ASSERT_ERR(NOT_FOUND, err, "Buy from empty shop fails");

  // Print empty shop (should not crash)
  printf("  Empty shop output:\n");
  print_revenue(shop);

  // I/O with empty shop
  err = io_text_write("empty_test.txt", shop);
  TEST_ASSERT_ERR(SUCCESS, err, "Write empty shop");

  Shop* shop2 = io_text_read("empty_test.txt", &err);
  TEST_ASSERT_ERR(SUCCESS, err, "Read empty shop");
  TEST_ASSERT(shop2 != NULL, "Empty shop read successfully");
  TEST_ASSERT(count_games_in_shop(shop2) == 0, "Read shop is empty");

  free_shop(shop);
  free_shop(shop2);
  remove("empty_test.txt");
}

// Test 8: Error handling
void test_error_handling(void) {
  TEST_START("Error Handling");

  ErrorCode err;

  // Read non-existent file
  Shop* shop = io_text_read("nonexistent_file_12345.txt", &err);
  TEST_ASSERT(shop == NULL, "Read non-existent file returns NULL");
  TEST_ASSERT_ERR(FILE_ERROR, err, "Returns FILE_ERROR");

  shop = io_binary_read("nonexistent_file_12345.dat", &err);
  TEST_ASSERT(shop == NULL, "Read non-existent binary file returns NULL");
  TEST_ASSERT_ERR(FILE_ERROR, err, "Returns FILE_ERROR");
}

// Test 9: Revenue list ordering
void test_revenue_ordering(void) {
  TEST_START("Revenue List Ordering");

  Shop* shop = malloc(sizeof(Shop));
  init_shop(shop);

  // Add games and buy in different order
  shop_add_game(shop, "GameA", 10.0);
  shop_add_game(shop, "GameB", 20.0);
  shop_add_game(shop, "GameC", 30.0);

  // Buy to create specific revenues: C=90, A=50, B=40
  shop_buy_game(shop, "GameC", 3);  // 90
  shop_buy_game(shop, "GameA", 5);  // 50
  shop_buy_game(shop, "GameB", 2);  // 40

  // Verify order (descending by revenue)
  Node* curr = shop->revenue;
  TEST_ASSERT(curr->game->revenue == 90.0, "First has highest revenue");
  curr = curr->next;
  TEST_ASSERT(curr->game->revenue == 50.0, "Second has middle revenue");
  curr = curr->next;
  TEST_ASSERT(curr->game->revenue == 40.0, "Third has lowest revenue");
  TEST_ASSERT(curr->next == NULL, "End of list reached");

  // Update revenue to change order
  shop_buy_game(shop, "GameB", 3);  // 40 + 60 = 100 (now highest)

  // Verify new order
  curr = shop->revenue;
  TEST_ASSERT(curr->game->revenue == 100.0, "Updated game is now highest");
  curr = curr->next;
  TEST_ASSERT(curr->game->revenue == 90.0, "Previous highest is now second");

  free_shop(shop);
}

// Test 10: Stress test
void test_stress(void) {
  TEST_START("Stress Test");

  Shop* shop = malloc(sizeof(Shop));
  init_shop(shop);
  ErrorCode err;

  const int num_games = 100;
  char name[20];

  // Add many games
  for (int i = 0; i < num_games; i++) {
    sprintf(name, "Game%03d", i);
    err = shop_add_game(shop, name, 10.0 + i);
    TEST_ASSERT(err == SUCCESS, "Add game in stress test");
  }

  TEST_ASSERT(count_games_in_shop(shop) == num_games, "All games added");

  // Buy some games
  for (int i = 0; i < num_games; i += 10) {
    sprintf(name, "Game%03d", i);
    err = shop_buy_game(shop, name, (i / 10) + 1);
    TEST_ASSERT(err == SUCCESS, "Buy game in stress test");
  }

  // Test I/O with many games
  err = io_text_write("stress_test.txt", shop);
  TEST_ASSERT_ERR(SUCCESS, err, "Write many games to file");

  Shop* shop2 = io_text_read("stress_test.txt", &err);
  TEST_ASSERT_ERR(SUCCESS, err, "Read many games from file");
  TEST_ASSERT(count_games_in_shop(shop2) == num_games, "All games read back");

  free_shop(shop);
  free_shop(shop2);
  remove("stress_test.txt");
}

// Main test runner
int main(void) {
  printf("╔══════════════════════════════════════╗\n");
  printf("║     Shop System Test Suite v1.0       ║\n");
  printf("╚══════════════════════════════════════╝\n");

  test_game_creation();
  test_shop_init();
  test_add_games();
  test_buy_games();
  test_plaintext_io();
  test_binary_io();
  test_empty_shop();
  test_error_handling();
  test_revenue_ordering();
  test_stress();

  TEST_SUMMARY();

  return tests_failed > 0 ? 1 : 0;
}
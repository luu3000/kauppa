#ifndef PROJECT_H
#define PROJECT_H

#define MAX_STRING_LENGTH 1000  // guidline 1.1.1

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file project.h
 * @brief Shop system implemented using a binary search tree and a
 * revenue-sorted linked list.
 *
 * This system stores games in a BST ordered by name and maintains a second
 * linked list sorted by revenue. The revenue list borrows Game pointers from
 * the BST and does not own them independently.
 *
 * @section data Data structures
 * - `Game` represents a game with a name, price, and revenue.
 * - `Shop` owns the BST root and the revenue list head.
 * - `Vertex` owns its `Game` object.
 * - `Node` borrows `Game*` pointers from the BST.
 *
 * @section structure File structure in a non-monolithic design previous the
 * combination used to submit to A+.
 * - `shop.c` contains shop management functions.
 * - `revenue.c` contains revenue list management functions.
 * - `bst.c` contains BST management functions.
 * - `io_plain.c` contains plaintext I/O functions.
 * - `io_binary.c` contains binary I/O functions.
 * - `legacy.c` contains legacy functions for comparison.
 * - `debug.c` contains debugging functions.
 *
 * @section error Error handling
 * Functions return `ErrorCode` to indicate success or specific failure modes.
 * If returning a pointer, NULL indicates failure and an `ErrorCode` is set via
 * an output parameter.
 *
 * on the design choices:
 *
 * The use of linked List for revenue sorting is a design choice to make easy
 * listing possible. The list is rebuilt from the BST after loading from file,
 * so it does not need to be stored persistently. The list is not saved to file.
 *
 * The BST is not self-balancing, so the order of insertion affects its shape.
 * The `build_balanced()` function is used to create a balanced BST when loading
 * from file, ensuring efficiency for search and insertion operations after
 * loading.
 *
 * The BST was used to allow easy insertion and searching by name.
 * The revenue list was used to allow efficient listing of games by revenue
 * without needing to sort the entire BST on demand.
 *
 * The linked list can be removed if desired, using functions in legacy.c.
 * Eliminating the binary search tree would require a different data structure
 * for efficient searching by name, such as a hash table, but the author did not
 * implement this due to perceived implementation complexity.
 *
 *
 */

/**
 * @brief Represents a game in the shop.
 */
typedef struct Game {
  char* name;     /**< Heap-allocated game name */
  double price;   /**< Price of the game */
  double revenue; /**< Accumulated revenue from purchases */
} Game;

/**
 * @brief Vertex of the binary search tree.
 */
typedef struct Vertex {
  Game* game;           /**< Pointer to owned Game */
  struct Vertex* left;  /**< Left subtree */
  struct Vertex* right; /**< Right subtree */
  struct Node* node;    /**< Pointer to the corresponding List node in the
                                revenue list for efficient updates */
} Vertex;

/**
 * @brief Node of the revenue-sorted linked list.
 */
typedef struct Node {
  Game* game;        /**< Borrowed pointer to Game */
  struct Node* next; /**< Next list node */
  struct Node* prev; /**< Previous list node (for easier removal) */
} Node;

/**
 * @brief Shop container for BST and revenue list.
 *
 * Dependencies:
 * - `root` points to the BST containing owned `Game` objects.
 * - `revenue` points to a secondary list ordered by `Game.revenue`.
 */
typedef struct Shop {
  Vertex* root;  /**< Root of the BST */
  Node* revenue; /**< Head of the revenue-sorted list */
} Shop;

typedef enum ErrorCode {
  SUCCESS,
  OUT_OF_MEMORY,
  FILE_ERROR,
  IO_EOF,
  NOT_FOUND,
  READ_ERROR,
  PARSE_ERROR,
  GAME_IN_SYSTEM,
  INVALID_OBJECT,
  INVALID_ARGUMENT
} ErrorCode;

/* ------------------------------------------------------------------------- */
/** @file shop.c containing the implementation of shop management functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Initialize an empty Shop.
 *
 * @param shop Pointer to a Shop object allocated by the caller.
 *
 *
 */
void init_shop(Shop* shop);

/**
 * @brief Free all memory owned by the Shop.
 *
 * @param shop Shop to free.
 *
 * Frees the BST, revenue list nodes, owned Game objects, and the Shop itself.
 */
void free_shop(Shop* shop);

/**
 * @brief Free all nodes in the revenue-sorted linked list.
 *
 * @param head Head of the list.
 *
 * The revenue list does not own Game objects.
 */
void free_revenue_list(Node* head);

/**
 * @brief Free a Game object.
 *
 * @param game Game to free.
 */
void free_game(Game* game);

/**
 * @brief Recursively free the binary search tree.
 *
 * @param node Root of the subtree.
 */
void free_bst(Vertex* node);

/**
 * @brief Create a new Game object.
 *
 * @param name Name of the game.
 * @param price Price of the game.
 * @return Pointer to allocated Game, or NULL on memory error.
 */
Game* create_game(const char* name, double price, ErrorCode* err);

/**
 * @brief Create a new BST Vertex.
 *
 * @param game Game pointer for the node.
 * @return Pointer to allocated Vertex, or NULL on memory error.
 */
Vertex* create_vertex(Game* game, ErrorCode* err);

/**
 * @brief Create a new list node for the revenue-sorted list.
 *
 * @param game Game pointer for the node.
 * @return Pointer to allocated List node, or NULL on memory error.
 */
Node* create_node(Game* game, ErrorCode* err);

/* ------------------------------------------------------------------------- */
/** @file revenue.c containing the implementation of revenue list management
 * functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Insert a Game into the revenue-sorted list.
 *
 * @param head Address of the list head pointer.
 * @param game Game to insert.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return Pointer to the inserted List node, or NULL on failure.
 *
 */
Node* list_insert_node(Node** head, Game* game, ErrorCode* err);

/**
 * @brief Remove a Game from the revenue-sorted list.
 *
 * @param head Address of the list head pointer.
 * @param node Node containing the Game to remove.
 * @param err Pointer to an ErrorCode variable to receive error status.
 *
 *
 */
ErrorCode list_remove_node(Node** head, Vertex* node);

/**
 * @brief Rebuild the revenue list from the BST.
 *
 * @param node Root of the BST.
 * @param head Address of the revenue list head pointer.
 * @param err Pointer to an ErrorCode variable to receive error status.
 */
ErrorCode list_rebuild_from_bst(const Vertex* node, Node** head);

/**
 * @brief Attach list nodes to the corresponding BST vertices after rebuilding
 * the revenue list.
 * @param root Root of the BST.
 * @param arr Array of Game pointers corresponding to the BST vertices in sorted
 * order.
 * @param list_nodes Array of Vertex pointers corresponding to the list nodes in
 * sorted order.
 * @param index Pointer to the current index in the arrays.
 *
 *
 */

void attachListNodes(Vertex* root, Game** arr, Vertex** list_nodes,
                     size_t* index);

/**
 * @brief Print all games sorted by revenue.
 *
 * @param shop Shop containing the revenue list.
 *
 */
void print_revenue(const Shop* shop);

/**
 * @brief Print a single game record.
 *
 * @param game Game to print.
 *
 */
ErrorCode print_game(const Game* game);

/**
 * @brief Flatten the BST into a Game pointer array.
 *
 * @param root BST root.
 * @param arr Output array of Game pointers.
 * @param index Index pointer for insertion.
 *
 */
void bst_make_array(const Vertex* root, Game** arr, size_t* index);

/**
 * @brief Free an array of Game objects.
 * @param array Array of Game objects.
 * @param count Number of Game objects in the array.
 *
 */
void free_game_array(Game* array, int count);

/**
 * @brief Compare Games by revenue for qsort.
 *
 * @param a Pointer to Game*.
 * @param b Pointer to Game*.
 * @return Negative if a > b, positive if a < b, zero if equal.
 *
 */
int list_revenue_compare(const void* a, const void* b);

/* ------------------------------------------------------------------------- */
/** @file bst.c containing the implementation of BST management functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Add a new game to the shop.
 *
 * @param shop Shop to modify.
 * @param name Name of the game.
 * @param price Price of the game.
 * @return ErrorCode indicating success or failure of the addition.
 */
ErrorCode shop_add_game(Shop* shop, const char* name, double price);

/**
 * @brief Recursively find a node in the BST.
 *
 * @param root Address of the subtree root pointer.
 * @param next Vertex to add.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return Pointer to the found node, or NULL if not found.
 *
 */
Vertex* bst_add_game_vertex(Vertex* root, Vertex* next, ErrorCode* err);

/**
 * @brief Buy a game and update its revenue.
 *
 * @param shop Shop to modify.
 * @param name Name of the game.
 * @param count Number of copies purchased.
 * @return ErrorCode indicating success or failure of the purchase.
 */
ErrorCode shop_buy_game(Shop* shop, const char* name, int count);

/**
 * @brief Recursively search the BST and find the game to buy, then update its
 * revenue and the revenue list.
 *
 * @param root Current subtree root.
 * @param name Name of the game.
 * @param err Pointer to an ErrorCode variable to receive error status.
 *
 * @return Pointer to the node of the found game, or NULL if not found.
 */
Vertex* bst_buy_game_vertex(Vertex* root, const char* name, ErrorCode* err);

/**
 * @brief Compute the number of nodes in the BST.
 *
 * @param root BST root.
 * @return Number of nodes.
 */
size_t bst_size(const Vertex* root);

/**
 * @brief Build a balanced BST from a sorted Game array.
 *
 * @param arr Sorted Game array.
 * @param start Starting index.
 * @param end Ending index.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return Root of the balanced subtree.
 */
Vertex* bst_build_from_sorted_array(Game** arr, int start, int end,
                                    ErrorCode* err);

/* ------------------------------------------------------------------------- */
/** @file io_plain.c containing the implementation of plaintext I/O functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Write a Game to plaintext.
 *
 * @param game Game to write.
 * @param file_ptr Output file.
 * @return SUCCESS on success, FILE_ERROR on failure.
 *
 */
ErrorCode io_text_write_game(const Game* game, FILE* file_ptr);

/**
 * @brief Recursively write the BST as plaintext using in-order traversal.
 *
 * @param node Current BST node.
 * @param file_ptr Output file.
 * @return SUCCESS on success, FILE_ERROR on failure.
 */
ErrorCode io_text_write_bst(Vertex* node, FILE* file_ptr);

/**
 * @brief Write the shop to a plaintext file.
 *
 * @param filename Output file name.
 * @param shop Shop to write.
 * @return ErrorCode indicating success or failure of the write operation.
 */
ErrorCode io_text_write(const char* filename, const Shop* shop);

/**
 * @brief Read a shop from a plaintext file.
 *
 * @param filename Input file name.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return Newly allocated Shop*, or NULL on failure.
 *
 */
Shop* io_text_read(const char* filename, ErrorCode* err);

/**
 * @brief Compare two Game structures by name.
 * @param a Pointer to the first Game structure.
 * @param b Pointer to the second Game structure.
 * @return qsort-compatible comparison result.
 *
 */
int cmp_game_name(const void* a, const void* b);

/* ------------------------------------------------------------------------- */
/** @file io_binary.c containing the implementation of binary I/O functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Write a Game structure to a binary file.
 *
 * @param game Game to write.
 * @param file_ptr Output file.
 * @return SUCCESS on success, FILE_ERROR on failure.
 */
ErrorCode io_binary_write_game(const Game* game, FILE* file_ptr);

/**
 * @brief Recursively write the BST to a binary file.
 *
 * @param node Current BST node.
 * @param file_ptr Output file.
 * @return ERROR_CODE on success or failure.
 */
ErrorCode io_binary_write_bst(Vertex* node, FILE* file_ptr);

/**
 * @brief Write the shop to a binary file.
 *
 * @param filename Output file name.
 * @param shop Shop to write.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return ErrorCode indicating success or failure of the write operation.
 */
ErrorCode io_binary_write(const char* filename, const Shop* shop);

/**
 * @brief Read a single Game from a binary file.
 *
 * @param file_ptr Input file.
 * @param err Error code pointer.
 * @return Heap-allocated Game*, or NULL on failure.
 */
Game* io_binary_read_game(FILE* file_ptr, ErrorCode* err);

/**
 * @brief Read a shop from a binary file.
 *
 * @param filename Input file name.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return Newly allocated Shop*, or NULL on failure.
 */
Shop* io_binary_read(const char* filename, ErrorCode* err);

/* ------------------------------------------------------------------------- */
/** @file legacy.c containing the implementation of legacy functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Print revenue ordering using an array instead of linked list.
 *
 * @param shop Shop to print.
 *
 */
void legacy_print_revenue(const Shop* shop);

/**
 * @brief Remove a Game from the revenue-sorted list.
 *
 * @param head Address of the list head pointer.
 * @param game Game to remove.
 * @return ErrorCode indicating success or failure of the removal.
 *
 */
ErrorCode legacy_remove_list(Node** head, Game* game);

/**
 * @brief Find the previous Game in the revenue list.
 *
 * @param head Head of the revenue list.
 * @param game Game whose predecessor is sought.
 * @return Pointer to the previous Game, or NULL if none.
 *
 */
Game* legacy_find_previous(Node* head, Game* game);

/* ------------------------------------------------------------------------- */
/** @file debug.c containing the implementation of debugging functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Print the full shop data structure.
 *
 * @param shop Shop to print.
 */
void debug_print_shop(const Shop* shop);

/**
 * @brief Print indentation tabs.
 *
 * @param num Number of tabs.
 */
void debug_print_tabs(int num);

/**
 * @brief gives pointer to previous game in linked list
 *
 */
Game* find_previous(Vertex* vertex);

/**
 * @brief Print the BST for debugging.
 *
 * @param shop Shop containing the revenue list.
 * @param vertex Current vertex.
 * @param depth Indentation depth.
 *
 */
ErrorCode debug_print_bst(const Shop* shop, Vertex* vertex, int depth);

/**
 * @brief Print an human-readable error message corresponding to an ErrorCode.
 *
 * @param err Error code to print.
 *
 */
void debug_print_error(ErrorCode err);

#endif /* PROJECT_H */

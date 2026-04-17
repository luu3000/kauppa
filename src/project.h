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
 * - `Node` owns its `Game` object.
 * - `List` borrows `Game*` pointers from the BST.
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
 * Memory management is carefully handled to avoid leaks, with clear ownership
 * semantics for each data structure.
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
 * @brief Node of the binary search tree.
 */
typedef struct Node {
  Game* game;             /**< Pointer to owned Game */
  struct Node* left;      /**< Left subtree */
  struct Node* right;     /**< Right subtree */
  struct List* list_node; /**< Pointer to the corresponding List node in the
                             revenue list for efficient updates */
} Node;

/**
 * @brief Node of the revenue-sorted linked list.
 *
 * The revenue list does not own Game objects; it only maintains pointers
 * to games stored in the BST.
 */
typedef struct List {
  Game* game;        /**< Borrowed pointer to Game */
  struct List* next; /**< Next list node */
  struct List* prev; /**< Previous list node (for easier removal) */
} List;

/**
 * @brief Shop container for BST and revenue list.
 *
 * Dependencies:
 * - `root` points to the BST containing owned `Game` objects.
 * - `revenue` points to a secondary list ordered by `Game.revenue`.
 */
typedef struct Shop {
  Node* root;    /**< Root of the BST */
  List* revenue; /**< Head of the revenue-sorted list */
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
 */
void initShop(Shop* shop);

/**
 * @brief Free all memory owned by the Shop.
 *
 * @param shop Shop to free.
 *
 * Frees the BST, revenue list nodes, owned Game objects, and the Shop itself.
 */
void freeShop(Shop* shop);

/**
 * @brief Free all nodes in the revenue-sorted linked list.
 *
 * @param head Head of the list.
 *
 * The revenue list does not own Game objects.
 */
void freeRevenueList(List* head);

/**
 * @brief Create a new Game object.
 *
 * @param name Name of the game.
 * @param price Price of the game.
 * @return Pointer to allocated Game, or NULL on memory error.
 */
Game* createGame(const char* name, double price, ErrorCode* err);

/**
 * @brief Free a Game object.
 *
 * @param game Game to free.
 */
void freeGame(Game* game);

/**
 * @brief Create a new BST Node.
 *
 * @param game Game pointer for the node.
 * @return Pointer to allocated Node, or NULL on memory error.
 */
Node* createNode(Game* game, ErrorCode* err);

/**
 * @brief Create a new list node for the revenue-sorted list.
 *
 * @param game Game pointer for the node.
 * @return Pointer to allocated List node, or NULL on memory error.
 */
List* createListNode(Game* game, ErrorCode* err);

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
 * This function inserts a Game into the revenue-sorted linked list in the
 * correct position based on its revenue.
 */
List* insertRevenueList(List** head, Game* game, ErrorCode* err);

/**
 * @brief Remove a Game from the revenue-sorted list.
 *
 * @param head Address of the list head pointer.
 * @param node Node containing the Game to remove.
 * @param err Pointer to an ErrorCode variable to receive error status.
 *
 * This function removes a Game from the revenue-sorted linked list.
 *
 * The `node` parameter is a pointer to the List node that contains the Game to
 * be removed. This allows for efficient removal without needing to search the
 * list by Game pointer, since the caller (e.g., buyGame()) can keep track of
 * the List node corresponding to the Game being updated. The function updates
 * the list pointers to bypass the removed node and frees the node's memory. It
 * returns SUCCESS on success, or an appropriate error code if the input is
 * invalid or if memory management fails during the process.
 *
 */
void removeRevenueList(List** head, Node* node, ErrorCode* err);

/**
 * @brief Rebuild the revenue list from the BST.
 *
 * @param node Root of the BST.
 * @param head Address of the revenue list head pointer.
 * @param err Pointer to an ErrorCode variable to receive error status.
 *
 *  This function traverses the BST and inserts each Game into the
 * revenue-sorted linked list. It can be used after loading from file to
 * reconstruct the revenue list based on the games stored in the BST. The
 * function handles memory allocation for the list nodes and returns an
 * appropriate error code if memory allocation fails during the process.
 *  */
void buildRevenueListFromTree(Node* node, List** head, ErrorCode* err);

/**
 * @brief Print all games sorted by revenue.
 *
 * @param shop Shop containing the revenue list.
 *
 * This function traverses the revenue-sorted linked list and prints each Game's
 * name and revenue. The list is sorted in descending order of revenue, so the
 * most profitable games are printed first.
 */
void printRevenue(const Shop* shop);

/**
 * @brief Print a single game record.
 *
 * @param game Game to print.
 *
 * This function prints the name and revenue of a single Game. It is used by
 * printRevenue() to print each game in the revenue list. The output format is
 * consistent with the requirements for listing games by revenue.
 *
 */
void printGame(Game* game);

/**
 * @brief Flatten the BST into a Game pointer array.
 *
 * @param root BST root.
 * @param arr Output array of Game pointers.
 * @param index Index pointer for insertion.
 *
 * This function is used to create an array of Game pointers from the BST by
 * in-order traversal, which can then be sorted by revenue.
 *
 */
void makeList(Node* root, Game** arr, size_t* index);

/**
 * @brief Compare Games by revenue for qsort.
 *
 * @param a Pointer to Game*.
 * @param b Pointer to Game*.
 * @return Negative if a > b, positive if a < b, zero if equal.
 *
 * This function is used as a comparison function for qsort to sort an array of
 * Game pointers by their revenue field in descending order.
 *
 */
int cmprevenue(const void* a, const void* b);

/* ------------------------------------------------------------------------- */
/** @file bst.c containing the implementation of BST management functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Add a new game to the shop.
 *
 * @param shop Shop to modify.
 * @param name Name of the game.
 * @param price Price of the game.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return SUCCESS if successful, NOT_FOUND if not.
 *
 * This function creates a new Game and inserts it into the BST. It also updates
 * the revenue list accordingly. It returns SUCCESS on success, or an
 * appropriate error code on failure.
 */
ErrorCode addGame(Shop* shop, const char* name, double price);

/**
 * @brief Recursively find a node in the BST.
 *
 * @param root Address of the subtree root pointer.
 * @param next Node to add.
 * @return Pointer to the found node, or NULL if not found.
 *
 * This function is used to implement the addGame() function, which needs to
 * find the correct location in the BST to insert a new game. The function
 * returns the found Node pointer for addGame() to use for insertion, or NULL if
 * the correct location is already occupied.
 */
Node* addGameNode(Node* root, Node* next, ErrorCode* err);

/**
 * @brief Buy a game and update its revenue.
 *
 * @param shop Shop to modify.
 * @param name Name of the game.
 * @param count Number of copies purchased.
 * @return SUCCESS if successful, NOT_FOUND if game not found.
 *
 * This function finds the game in the BST, updates its revenue, and then
 * updates the revenue list accordingly. It returns SUCCESS on success, or
 * NOT_FOUND if the game is not found in the BST.
 */
ErrorCode buyGame(Shop* shop, const char* name, int count);

/**
 * @brief Recursively search the BST and find the game to buy, then update its
 * revenue and the revenue list.
 *
 * @param shop Shop owning the revenue list.
 * @param root Current subtree root.
 * @param name Name of the game.
 * @param err Pointer to an ErrorCode variable to receive error status.
 *
 *
 * @return Pointer to the node of the found game, or NULL if not found.
 * This function is used to implement the buyGame() function, which needs to
 * find the game in the BST, update its revenue, and then update the revenue
 * list accordingly. The function returns the found Game pointer for buyGame()
 * to use for revenue updates, or NULL if the game is not found in the BST.
 */
Node* buyGameNode(Node* root, const char* name, ErrorCode* err);

/**
 * @brief Compute the number of nodes in the BST.
 *
 * @param root BST root.
 * @return Number of nodes.
 * This function is used to determine the size of the BST for building a
 * balanced tree when loading from file.
 */
size_t treeSize(const Node* root);

/**
 * @brief Build a balanced BST from a sorted Game array.
 *
 * @param arr Sorted Game array.
 * @param start Starting index.
 * @param end Ending index.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return Root of the balanced subtree.
 * This function is used to create a balanced BST when loading from file,
 * ensuring efficiency for search and insertion operations after loading.
 */

Node* build_balanced(Game** arr, int start, int end, ErrorCode* err);

/**
 * @brief Recursively free the binary search tree.
 *
 * @param node Root of the subtree.
 * Frees all nodes and their owned Game objects.
 */
void freeTree(Node* node);

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
 * This function is used to write a single Game record to a plaintext file in a
 * specific format. It is called by write_tree_plaintext() during the in-order
 * traversal of the BST to save the entire shop to a file. The function formats
 * the Game's name, price, and revenue as a line of text, and checks for errors
 * during writing to ensure data integrity in the output file.
 */
ErrorCode write_game_plaintext(Game* game, FILE* file_ptr);

/**
 * @brief Recursively write the BST as plaintext using in-order traversal.
 *
 * @param node Current BST node.
 * @param file_ptr Output file.
 * @return SUCCESS on success, FILE_ERROR on failure.
 *
 * This function performs an in-order traversal of the BST, writing each Game to
 * the output file using write_game_plaintext(). It ensures that the games are
 * written in sorted order by name. The function checks for errors during
 * writing and propagates any error codes back to the caller, allowing for
 * robust error handling during the save process.
 */
ErrorCode write_tree_plaintext(Node* node, FILE* file_ptr);

/**
 * @brief Write the shop to a plaintext file.
 *
 * @param filename Output file name.
 * @param shop Shop to write.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return 0 on success, 1 on failure.
 *
 * This function saves the entire shop to a plaintext file by performing an
 * in-order traversal of the BST and writing each Game record to the file. It
 * handles file opening, error checking during writing, and ensures that all
 * resources are properly released in case of an error. The function returns
 * SUCCESS on success, or an appropriate error code on failure, allowing the
 * caller to understand the reason for any failure that occurs during the save
 * process. The expected format of the output file is one game per line, with
 * the name in quotes, followed by the price and revenue, e.g.: "Game
 * Name" 19.99 1000.00
 */
ErrorCode write_plaintext(const char* filename, const Shop* shop);

/**
 * @brief Read a shop from a plaintext file.
 *
 * @param filename Input file name.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return Newly allocated Shop*, or NULL on failure.
 *
 * This function reads game records from a plaintext file, creates Game objects,
 * and builds a balanced BST to store them in the Shop. It also rebuilds the
 * revenue list from the BST after loading. The function handles various error
 * conditions, such as file errors, memory allocation failures, and parsing
 * errors, and ensures that all allocated resources are properly freed in case
 * of an error to avoid memory leaks. The function returns a pointer to the
 * newly created Shop on success, or NULL on failure, with the error code set in
 * the provided pointer. The expected format of the input file is one game per
 * line, with the name in quotes, followed by the price and revenue, e.g.: "Game
 * Name" 19.99 1000.00
 *
 */
Shop* read_plaintext(const char* filename, ErrorCode* err);

/*
 * @brief Compare two Game structures by name.
 * @param a Pointer to the first Game structure.
 * @param b Pointer to the second Game structure.
 * @return Negative value if a < b, zero if a == b, positive value if a > b.
 *
 * This function is used for sorting an array of Game pointers by their name
 * field using qsort. This is used when building a balanced BST from an array of
 * Game pointers, ensuring that the games are inserted into the BST in sorted
 * order by name, which is necessary for the BST properties to hold.
 */
static int cmp_game_name(const void* a, const void* b);

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
ErrorCode write_game_binary(Game* game, FILE* file_ptr);

/**
 * @brief Recursively write the BST to a binary file.
 *
 * @param node Current BST node.
 * @param file_ptr Output file.
 * @return SUCCESS on success, FILE_ERROR on failure.
 */
ErrorCode write_tree_binary(Node* node, FILE* file_ptr);

/**
 * @brief Write the shop to a binary file.
 *
 * @param filename Output file name.
 * @param shop Shop to write.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return 0 on success, 1 on failure.
 */
ErrorCode write_binary(const char* filename, const Shop* shop);

/**
 * @brief Read a single Game from a binary file.
 *
 * @param file_ptr Input file.
 * @param err Error code pointer.
 * @return Heap-allocated Game*, or NULL on failure.
 */
Game* read_game_binary(FILE* file_ptr, ErrorCode* err);

/**
 * @brief Read a shop from a binary file.
 *
 * @param filename Input file name.
 * @param err Pointer to an ErrorCode variable to receive error status.
 * @return Newly allocated Shop*, or NULL on failure.
 */
Shop* read_binary(const char* filename, ErrorCode* err);

/**
 * @brief Free an array of Game objects.
 * @param array Array of Game objects.
 * @param count Number of Game objects in the array.
 *
 * This function is used to free the temporary array of Game objects created
 * when reading from a binary file. It frees the name strings and the array
 * itself to avoid memory leaks in the case of an error during the loading
 * process. The function iterates through the array, freeing each name string,
 * and then frees the array pointer itself. This is necessary because the Game
 * objects
 */
static void free_game_array(Game* array, int count);

/* ------------------------------------------------------------------------- */
/** @file legacy.c containing the implementation of legacy functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Print revenue ordering using an array instead of linked list.
 *
 * @param shop Shop to print.
 *
 * This function is a legacy implementation that prints the games in order of
 * revenue by first flattening the BST into an array of Game pointers, then
 * sorting that array by revenue, and finally printing the games in sorted
 * order. This approach is less efficient than using a linked list for revenue
 * sorting, but it serves as a comparison to show the benefits of the linked
 * list design. The function uses the treeSize() function to determine the size
 * of the BST for allocating the array, and then uses qsort with a custom
 * comparison function to sort the games by revenue before printing them.
 *
 */
void printRevenueViaArray(const Shop* shop);

/**
 * @brief Remove a Game from the revenue-sorted list.
 *
 * @param head Address of the list head pointer.
 * @param game Game to remove.
 * @param err Pointer to an ErrorCode variable to receive error status.
 *
 * This function removes a Game from the revenue-sorted linked list.
 * Uses only one linked list, so it traverses the list to find the game to
 * remove.
 */
void removeRevenueListLegacy(List** head, Game* game, ErrorCode* err);
/* ------------------------------------------------------------------------- */
/** @file debug.c containing the implementation of debugging functions */
/* ------------------------------------------------------------------------- */

/**
 * @brief Print the full shop data structure.
 *
 * @param shop Shop to print.
 * Prints the BST structure and the revenue list connections for debugging
 * purposes.
 */
void printShop(const Shop* shop);

/**
 * @brief Print indentation tabs.
 *
 * @param num Number of tabs.
 * Used for visualizing the BST structure in printShop().
 */
void printTab(int num);

/**
 * @brief Print the BST for debugging.
 *
 * @param shop Shop containing the revenue list.
 * @param node Current node.
 * @param depth Indentation depth.
 * Prints the BST structure and the revenue list connections for debugging
 * purposes.
 */
void printTree(const Shop* shop, Node* node, int depth);

/**
 * @brief Find the previous Game in the revenue list.
 *
 * @param head Head of the revenue list.
 * @param game Game whose predecessor is sought.
 * @return Pointer to the previous Game, or NULL if none.
 *
 * This function is used for debugging purposes to show the connections in the
 * revenue list when printing the shop structure. It traverses the revenue list
 * to find the Game that comes before the specified Game, which helps visualize
 * the ordering of the revenue list in relation to the BST structure.
 */
Game* findPrevious(List* head, Game* game);

/**
 * @brief Print an human-readable error message corresponding to an ErrorCode.
 *
 * @param err Error code to print.
 *
 * This function is used to convert ErrorCode values into human-readable
 * messages for debugging and user feedback purposes. It takes an ErrorCode as
 * input and prints a corresponding message to standard output, allowing
 * developers and users to understand the nature of errors that occur during the
 * execution of shop operations.
 */
void printError(ErrorCode err);

#endif /* PROJECT_H */

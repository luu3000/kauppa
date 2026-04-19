#include "../project.h"

ErrorCode shop_add_game(Shop* shop, const char* name, double price) {
  // Create the Game object
  ErrorCode err;
  Game* game = create_game(name, price, &err);
  if (!game) {
    return err;
  }
  // Create the BST vertex for the game
  Vertex* vertex = create_vertex(game, &err);
  if (!vertex) {
    free_game(game);
    return err;
  }
  // Insert the vertex into the BST
  Vertex* result = bst_add_game_vertex(shop->root, vertex, &err);
  shop->root = result;
  if (err != SUCCESS) {
    free_game(game);
    free_vertex(vertex);
    return err;
  }
  // Insert the game into the revenue list
  Node* node = list_insert_node(&shop->revenue, game, &err);
  if (node) {
    vertex->node = node;
  }
  return err;
}

Vertex* bst_add_game_vertex(Vertex* root, Vertex* next, ErrorCode* err) {
  // Base case: found the insertion point
  if (root == NULL) {
    return next;
  } else {  // Compare game names to determine direction
    int comp = strcmp(next->game->name, root->game->name);
    if (comp < 0) {
      root->left = bst_add_game_vertex(root->left, next, err);  // go left
    } else if (comp > 0) {
      root->right = bst_add_game_vertex(root->right, next, err);  // go right
    } else {
      if (err) *err = GAME_IN_SYSTEM;
      return root;  // duplicate name
    }
    return root;
  }
}

ErrorCode shop_buy_game(Shop* shop, const char* name, int count) {
  // Find the game in the BST
  ErrorCode err;
  Vertex* vertex = bst_buy_game_vertex(shop->root, name, &err);
  if (!vertex) {
    return err;
  }
  // Update the revenue list: remove old entry...
  err = list_remove_node(&shop->revenue, vertex->node);
  vertex->node = NULL;
  if (err != SUCCESS) {
    return err;
  }
  Game* game = vertex->game;
  // ...update revenue...
  game->revenue += game->price * count;
  // ...and reinsert into the revenue list
  vertex->node = list_insert_node(&shop->revenue, game, &err);
  if (err != SUCCESS) {
    return err;
  }
  return SUCCESS;
}

Vertex* bst_buy_game_vertex(Vertex* root, const char* name, ErrorCode* err) {
  if (root == NULL) {
    if (err) *err = NOT_FOUND;
    return NULL;  // game not found
  } else {        // Compare game names to determine direction
    int comp = strcmp(name, root->game->name);
    if (comp < 0) {
      return bst_buy_game_vertex(root->left, name, err);  // go left
    } else if (comp > 0) {
      return bst_buy_game_vertex(root->right, name, err);  // go right
    } else {
      return root;  // found the node of the game
    }
  }
}

size_t bst_size(const Vertex* root) {
  if (root == NULL) {  // base case: empty subtree
    return 0;
  } else {  // recursive case: count this node + left subtree + right subtree
    return bst_size(root->left) + bst_size(root->right) + 1;
  }
}

Vertex* bst_build_from_sorted_array(Game** arr, int start, int end,
                                    ErrorCode* err) {
  if (start > end) return NULL;  // base case: empty subtree

  int mid =
      (start + end) / 2;  // middle element becomes the root of this subtree

  // Create a vertex for the middle game
  Vertex* vertex = create_vertex(arr[mid], err);
  if (!vertex) {
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }
  // Recursively build left and right subtrees
  vertex->left = bst_build_from_sorted_array(arr, start, mid - 1, err);
  vertex->right = bst_build_from_sorted_array(arr, mid + 1, end, err);
  // If either subtree failed to build, free the current vertex and return NULL
  if ((mid > start && !vertex->left) || (mid < end && !vertex->right)) {
    freeTree(vertex);
    return NULL;
  }
  if (err) *err = SUCCESS;
  return vertex;
}

void bst_free(Vertex* vertex) {
  // base case: empty subtree
  if (vertex == NULL) return;
  // post-order traversal to free children before parent
  freeTree(vertex->left);
  freeTree(vertex->right);
  // Free the Game object owned by this vertex
  free(vertex->game->name);
  free(vertex->game);
  free(vertex);
}

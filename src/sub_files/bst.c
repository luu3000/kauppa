#include "../project.h"

Node* addGameNode(Node* root, Node* next, ErrorCode* err) {
  if (root == NULL) {
    return next;
  } else {
    int comp = strcmp(next->game->name, root->game->name);
    if (comp < 0) {
      root->left = addGameNode(root->left, next, err);  // go left
    } else if (comp > 0) {
      root->right = addGameNode(root->right, next, err);  // go right
    } else {
      if (err) *err = GAME_IN_SYSTEM;
      return root;  // duplicate name
    }
    return root;
  }
}

ErrorCode addGame(Shop* shop, const char* name, double price) {
  ErrorCode err;
  Game* game = createGame(name, price, &err);  // make the game
  if (!game) {
    return err;
  }

  Node* node = createNode(game, &err);  // give it to a BST node
  if (!node) {
    freeGame(game);
    return err;
  }

  Node* result = addGameNode(shop->root, node, &err);  // try to insert into BST
  shop->root = result;  // update root in case it was NULL

  if (err != SUCCESS) {
    freeGame(game);
    free(node);
    return err;
  }

  List* revenue_node = insertRevenueList(&shop->revenue, game, &err);
  if (revenue_node) {
    node->list_node = revenue_node;
  }
  return err;
}

Node* buyGameNode(Node* root, const char* name, ErrorCode* err) {
  if (root == NULL) {
    if (err) *err = NOT_FOUND;
    return NULL;  // game not found
  } else {
    int comp = strcmp(name, root->game->name);
    if (comp < 0) {
      return buyGameNode(root->left, name, err);  // go left
    } else if (comp > 0) {
      return buyGameNode(root->right, name, err);  // go right
    } else {
      return root;  // found the node of the game
    }
  }
}

ErrorCode buyGame(Shop* shop, const char* name, int count) {
  // Find the game in the BST
  ErrorCode err;
  Node* node = buyGameNode(shop->root, name, &err);
  if (!node) {
    return err;
  }
  // Update the revenue list: remove old entry...
  removeRevenueList(&shop->revenue, node, &err);
  node->list_node = NULL;

  if (err != SUCCESS) {
    return err;
  }
  Game* game = node->game;
  // ...update revenue...
  game->revenue += game->price * count;
  // ...and reinsert into the revenue list
  node->list_node = insertRevenueList(&shop->revenue, game, &err);

  if (err != SUCCESS) {
    return err;
  }
  return SUCCESS;
}

size_t treeSize(const Node* root) {
  if (root == NULL) {
    return 0;
  } else {
    return treeSize(root->left) + treeSize(root->right) + 1;
  }
}

Node* build_balanced(Game** arr, int start, int end, ErrorCode* err) {
  if (start > end) return NULL;

  int mid = (start + end) / 2;

  Node* node = createNode(arr[mid], err);
  if (!node) {
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }

  node->left = build_balanced(arr, start, mid - 1, err);
  node->right = build_balanced(arr, mid + 1, end, err);

  if ((mid > start && !node->left) || (mid < end && !node->right)) {
    freeTree(node);
    return NULL;
  }

  if (err) *err = SUCCESS;
  return node;
}

void freeTree(Node* node) {
  if (node == NULL) return;
  freeTree(node->left);
  freeTree(node->right);
  free(node->game->name);
  free(node->game);
  free(node);
}

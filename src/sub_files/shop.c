#include "../project.h"

void initShop(Shop* shop) {
  shop->root = NULL;
  shop->revenue = NULL;
}

void freeShop(Shop* shop) {
  freeRevenueList(shop->revenue);
  freeTree(shop->root);
  free(shop);
}

void freeRevenueList(List* head) {
  while (head) {
    List* tmp = head;
    head = head->next;
    free(tmp);
  }
}

Game* createGame(const char* name, double price, ErrorCode* err) {
  if (!name || strlen(name) >= MAX_STRING_LENGTH) {
    if (err) *err = INVALID_ARGUMENT;
    return NULL;
  }

  Game* game = (Game*)malloc(sizeof(Game));
  if (!game) {
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }

  game->name = malloc(strlen(name) + 1);
  if (!game->name) {
    free(game);
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }
  strcpy(game->name, name);
  game->price = price;
  game->revenue = 0;

  return game;
}

void freeGame(Game* game) {
  free(game->name);
  free(game);
}

Node* createNode(Game* game, ErrorCode* err) {
  Node* node = (Node*)malloc(sizeof(Node));
  if (!node) {
    if (game) freeGame(game);
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }
  node->game = game;
  node->left = NULL;
  node->right = NULL;
  node->list_node = NULL;
  return node;
}

List* createListNode(Game* game, ErrorCode* err) {
  List* node = (List*)malloc(sizeof(List));
  if (!node) {
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }
  node->game = game;
  node->next = NULL;
  node->prev = NULL;
  return node;
}

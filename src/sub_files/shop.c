#include "../project.h"

void init_shop(Shop* shop) {
  shop->root = NULL;
  shop->revenue = NULL;
}

void free_shop(Shop* shop) {
  freeRevenueList(shop->revenue);
  freeTree(shop->root);
  free(shop);
}

void free_revenue_list(Node* head) {
  while (head) {
    Node* tmp = head;
    head = head->next;
    free(tmp);
  }
}

Game* create_game(const char* name, double price, ErrorCode* err) {
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

void free_game(Game* game) {
  free(game->name);
  free(game);
}

Vertex* create_vertex(Game* game, ErrorCode* err) {
  Vertex* node = (Vertex*)malloc(sizeof(Vertex));
  if (!node) {
    if (game) free_game(game);
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }
  node->game = game;
  node->left = NULL;
  node->right = NULL;
  node->node = NULL;
  return node;
}

Node* createListNode(Game* game, ErrorCode* err) {
  Node* node = (Node*)malloc(sizeof(Node));
  if (!node) {
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }
  node->game = game;
  node->next = NULL;
  node->prev = NULL;
  return node;
}

#include <stddef.h>

#include "../project.h"

List* insertRevenueList(List** head, Game* game, ErrorCode* err) {
  // Validate input
  if (head == NULL || game == NULL) {
    if (err) *err = INVALID_OBJECT;
    return NULL;
  }

  // Create new list node
  List* node = createListNode(game, err);
  if (!node) {
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }

  // empty list
  if (*head == NULL) {
    *head = node;
    if (err) *err = SUCCESS;
    return node;
  }
  // insert at head if revenue is greater or equal to current head
  if (game->revenue >= (*head)->game->revenue) {
    node->next = *head;
    if (*head) (*head)->prev = node;
    *head = node;
    if (err) *err = SUCCESS;
    return node;
  }
  // find insertion point
  List* current = *head;
  while (current->next && current->next->game->revenue > game->revenue) {
    current = current->next;
  }
  // insert after current
  node->next = current->next;
  current->next = node;

  node->prev = current;
  if (current->next) {
    current->next->prev = node;
  }

  if (err) *err = SUCCESS;
  return node;
}

void removeRevenueList(List** head, Node* node, ErrorCode* err) {
  // Validate input
  if (head == NULL || *head == NULL || node == NULL) {
    if (err) *err = INVALID_OBJECT;
    return;
  }

  List* prev = node->list_node ? node->list_node->prev : NULL;
  List* next = node->list_node ? node->list_node->next : NULL;

  // Remove node from list
  if (prev) {
    prev->next = next;
  } else {
    *head = next;  // node is head
  }
  if (next) {
    next->prev = prev;
  }
  // free the removed node
  free(node->list_node);
  *err = SUCCESS;
  return;
}

int cmprevenue(const void* a, const void* b) {
  const Game* g1 = *(const Game**)a;
  const Game* g2 = *(const Game**)b;
  return (g1->revenue < g2->revenue) - (g1->revenue > g2->revenue);
}

void makeList(Node* root, Game** arr, size_t* index) {
  if (!root) return;
  makeList(root->left, arr, index);
  arr[*index] = root->game;
  (*index)++;
  makeList(root->right, arr, index);
}

void attachListNodes(Node* root, Game** arr, Node* list_nodes, size_t* index) {
  if (!root) return;

  attachListNodes(root->left, arr, list_nodes, index);

  // arr[*index] corresponds to this root->game
  // list_nodes[*index] is the newly created List node
  if (root->game == arr[*index]) {
    root->list_node = list_nodes[*index].list_node;
  }

  (*index)++;

  attachListNodes(root->right, arr, list_nodes, index);
}

void buildRevenueListFromTree(Node* node, List** revenue, ErrorCode* err) {
  if (node == NULL) {
    if (err) *err = SUCCESS;
    return;
  }
  // slow but simple: insert each game into the list one by one
  /*
  buildRevenueListFromTree(node->left, revenue, err);
  insertRevenueList(revenue, node->game, err);
  buildRevenueListFromTree(node->right, revenue, err);
  */

  // faster approach: collect games into array, sort by revenue, build list
  //-- a previously discarded approach to form the revenue print.

  size_t size_of_tree = treeSize(node);

  Game** gamearr = (Game**)calloc(size_of_tree, sizeof(Game*));
  if (!gamearr) {
    if (err) *err = OUT_OF_MEMORY;
    return;
  }

  size_t index = 0;
  makeList(node, gamearr, &index);
  size_t size_of_revenue_list = index;

  qsort(gamearr, size_of_revenue_list, sizeof(Game*), cmprevenue);

  Node* list_nodes = build_balanced(gamearr, 0, size_of_revenue_list - 1, err);
  if (!list_nodes) {
    free(gamearr);
    if (err && *err == SUCCESS) *err = OUT_OF_MEMORY;
    return;
  }

  *revenue =
      list_nodes->list_node;  // head of the list is the first node in the

  index = 0;
  attachListNodes(node, gamearr, list_nodes, &index);
}

void printRevenue(const Shop* shop) {
  List* current = shop->revenue;
  while (current != NULL) {
    printGame(current->game);
    current = current->next;
  }
}

void printGame(Game* game) { write_game_plaintext(game, stdout); }

#include <stddef.h>

#include "../project.h"

Node* list_insert_node(Node** head, Game* game, ErrorCode* err) {
  // Validate input
  if (head == NULL || game == NULL) {
    if (err) *err = INVALID_OBJECT;
    return NULL;
  }

  // Create new list node
  Node* node = createListNode(game, err);
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
  Node* current = *head;
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

ErrorCode list_remove_node(Node** head, Vertex* node) {
  // Validate input
  if (head == NULL || *head == NULL || node == NULL) {
    return INVALID_OBJECT;
  }

  Node* prev = node->node ? node->node->prev : NULL;
  Node* next = node->node ? node->node->next : NULL;

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
  free(node->node);
  return SUCCESS;
}

/*void attachListNodes(Vertex* root, Game** arr, Vertex** list_nodes,
                     size_t* index) {
  // base case: empty subtree
  if (!root) return;
  // in-order traversal to match sorted array order
  attachListNodes(root->left, arr, list_nodes, index);

  // Find the corresponding list node for this vertex's game
  Game* game = root->game;
  for (size_t i = 0; i < *index; i++) {
    if (arr[i] == game) {
      root->node = list_nodes[i]->node;  // attach the list node pointer
      break;
    }
  }
  // Move to the next index in the array
  (*index)++;
  attachListNodes(root->right, arr, list_nodes, index);
}*/

ErrorCode list_rebuild_from_bst(Vertex* node, Node** head) {
  if (node == NULL) {
    return SUCCESS;
  }
  // slow but simple: insert each game into the list one by one
  /*
  buildRevenueListFromTree(node->left, revenue, err);
  insertRevenueList(revenue, node->game, err);
  buildRevenueListFromTree(node->right, revenue, err);
  */

  // faster approach: collect games into array, sort by revenue, build list
  //-- a previously discarded approach to form the revenue print.

  // Get size of BST to allocate array
  size_t size_of_tree = bst_size(node);

  // Collect games into array
  Game** gamearr = (Game**)calloc(size_of_tree, sizeof(Game*));
  if (!gamearr) {
    return OUT_OF_MEMORY;
  }

  size_t index = 0;
  bst_make_array(node, gamearr, &index);
  size_t size_of_revenue_list = index;

  // Sort array by revenue
  qsort(gamearr, size_of_revenue_list, sizeof(Game*), list_revenue_compare);

  // Build linked list from sorted array
  ErrorCode err;
  Vertex* list =
      bst_build_from_sorted_array(gamearr, 0, size_of_revenue_list - 1, &err);
  if (!list) {
    free(gamearr);
    return err != SUCCESS ? err : OUT_OF_MEMORY;
  }

  *head = list->node;  // head of the list is the first node in the

  //  index = 0;
  // attachListNodes(node, gamearr, list, &index);
}

void print_revenue(const Shop* shop) {
  Node* current = shop->revenue;
  while (current != NULL) {
    print_game(current->game);
    current = current->next;
  }
}

void print_game(Game* game) { io_text_write_game(game, stdout); }

void bst_make_array(Vertex* root, Game** arr, size_t* index) {
  if (!root) return;
  bst_make_array(root->left, arr, index);
  arr[*index] = root->game;
  (*index)++;
  bst_make_array(root->right, arr, index);
}

void free_game_array(Game* array, int count) {
  for (int i = 0; i < count; i++) {
    free(array[i].name);
  }
  free(array);
}

int list_revenue_compare(const void* a, const void* b) {
  const Game* g1 = *(const Game**)a;
  const Game* g2 = *(const Game**)b;
  return (g1->revenue < g2->revenue) - (g1->revenue > g2->revenue);
}

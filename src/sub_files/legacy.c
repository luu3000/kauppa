#include "../project.h"

void legacy_print_revenue(const Shop* shop) {
  size_t size_of_tree = treeSize(shop->root);

  Game** gamearr = (Game**)calloc(size_of_tree, sizeof(Game*));
  if (!gamearr) {
    return;
  }
  int index = 0;
  makeList(shop->root, gamearr, &index);
  size_t size_of_list = index;

  qsort(gamearr, size_of_list, sizeof(Game*), cmprevenue);
  for (size_t i = 0; i < size_of_list; i++) {
    printGame(gamearr[i]);
  }
  free(gamearr);
}

ErrorCode legacy_remove_revenue(Node** head, Game* game) {
  // Validate input
  if (head == NULL || *head == NULL || game == NULL) {
    return INVALID_OBJECT;
  }

  // find the game in the list
  Node* current = *head;
  Node* previous = NULL;
  while (current != NULL && current->game != game) {
    previous = current;
    current = current->next;
  }
  // game not found
  if (current == NULL) {
    return NOT_FOUND;
  }
  // remove current
  if (previous == NULL) {
    *head = current->next;
  } else {
    previous->next = current->next;
  }

  // free the removed node
  free(current);
  return SUCCESS;
}

Game* legacy_find_previous(Node* head, Game* game) {
  while (head && head->next) {
    if (head->next->game == game) return head->game;
    head = head->next;
  }
  return NULL;
}
#include "../project.h"

void printShop(const Shop* shop) { printTree(shop, shop->root, 0); }

Game* findPrevious(List* head, Game* game) {
  while (head && head->next) {
    if (head->next->game == game) return head->game;
    head = head->next;
  }
  return NULL;
}

void printTab(int num) {
  for (int i = 0; i < num; i++) {
    printf("\t");
  }
}

void printTree(const Shop* shop, Node* node, int depth) {
  if (node == NULL) return;

  printTab(depth);
  printGame(node->game);

  printTab(depth);
  Game* prev = findPrevious(shop->revenue, node->game);
  if (prev) {
    printf("is connected to %s\n", prev->name);
  } else {
    printf("is connected to <none>\n");
  }

  if (node->left != NULL) {
    printTab(depth + 1);
    printf("left\n");
    printTree(shop, node->left, depth + 1);
  }
  if (node->right != NULL) {
    printTab(depth + 1);
    printf("right\n");
    printTree(shop, node->right, depth + 1);
  }
}

void printError(ErrorCode err) {
  switch (err) {
    case SUCCESS:
      printf("SUCCESS\n");
      break;
    case NOT_FOUND:
      printf("a file was not found\n");
      break;
    case FILE_ERROR:
      printf("a file error occurred\n");
      break;
    case OUT_OF_MEMORY:
      printf("program ran out of memory\n");
      break;
    case READ_ERROR:
      printf("a read error occurred\n");
      break;
    case IO_EOF:
      printf("end of file reached, no more data to read\n");
      break;
    case PARSE_ERROR:
      printf("a parse error occurred, input data was malformed\n");
      break;
    case GAME_IN_SYSTEM:
      printf("a game with the same name already exists in the system\n");
      break;
    case INVALID_OBJECT:
      printf("an invalid object was encountered (e.g. NULL pointer)\n");
      break;
    case INVALID_ARGUMENT:
      printf("an invalid argument was provided to a function\n");
      break;
    default:
      printf("an unknown error occurred\n");
  }
}
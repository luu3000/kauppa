#include "../project.h"

void debug_print_shop(const Shop* shop) {
  debug_print_bst(shop, shop->root, 0);
}

void debug_print_tab(int num) {
  for (int i = 0; i < num; i++) {
    printf("\t");
  }
}

void debug_print_bst(const Shop* shop, Vertex* vertex, int depth) {
  if (vertex == NULL) return;

  debug_print_tab(depth);
  printGame(vertex->game);

  debug_print_tab(depth);
  Game* prev = legacy_find_previous(shop->revenue, vertex->game);
  if (prev) {
    printf("is connected to %s\n", prev->name);
  } else {
    printf("is connected to <none>\n");
  }

  if (vertex->left != NULL) {
    debug_print_tab(depth + 1);
    printf("left\n");
    debug_print_bst(shop, vertex->left, depth + 1);
  }
  if (vertex->right != NULL) {
    debug_print_tab(depth + 1);
    printf("right\n");
    debug_print_bst(shop, vertex->right, depth + 1);
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
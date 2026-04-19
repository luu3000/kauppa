#include "../project.h"

void debug_print_shop(const Shop* shop) {
  // call recursion
  debug_print_bst(shop, shop->root, 0);
}

void debug_print_tabs(int num) {
  for (int i = 0; i < num; i++) {
    printf("\t");
  }
}

Game* find_previous(Vertex* vertex) {
  if (vertex->node) {                   // test node exists
    if (vertex->node->prev) {           // test it has previous
      return vertex->node->prev->game;  // return the game
    }
  }
  return NULL;
}

void debug_print_bst(const Shop* shop, Vertex* vertex, int depth) {
  // if (vertex == NULL) return; // Basecase optional as the if statments mean
  // never called

  // whoami
  debug_print_tabs(depth);
  print_game(vertex->game);

  // neighbor in linked list
  debug_print_tabs(depth);
  Game* prev = find_previous(vertex);
  if (prev) {
    printf("is connected to %s\n", prev->name);
  } else {
    printf("is connected to <none>\n");
  }

  // recursion
  if (vertex->left != NULL) {  // do left
    debug_print_tabs(depth + 1);
    printf("left\n");
    debug_print_bst(shop, vertex->left, depth + 1);
  }
  if (vertex->right != NULL) {  // do right
    debug_print_tabs(depth + 1);
    printf("right\n");
    debug_print_bst(shop, vertex->right, depth + 1);
  }
}

void debug_print_error(ErrorCode err) {
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
      printf("an unknown error occurred %d\n", err);
  }
}
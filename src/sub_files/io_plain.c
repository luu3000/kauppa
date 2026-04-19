#include "../project.h"

ErrorCode io_text_write_game(Game* game, FILE* file_ptr) {
  if (fprintf(file_ptr, "\"%s\" %.2f %.2f\n", game->name, game->price,
              game->revenue) < 0) {
    return FILE_ERROR;
  }
  return SUCCESS;
}

#define iferror_or_success(expr) \
  do {                           \
    ErrorCode _err = (expr);     \
    if (_err != SUCCESS) {       \
      return _err;               \
    }                            \
  } while (0)

ErrorCode io_text_write_tree(Vertex* node, FILE* file_ptr) {
  if (node == NULL) {
    return SUCCESS;
  }
  ErrorCode left = io_text_write_tree(node->left, file_ptr);
  iferror_or_success(left);
  ErrorCode current = io_text_write_game(node->game, file_ptr);
  iferror_or_success(current);
  ErrorCode right = io_text_write_tree(node->right, file_ptr);
  iferror_or_success(right);

  return SUCCESS;
}

ErrorCode io_text_write(const char* filename, const Shop* shop) {
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    return FILE_ERROR;
  }

  ErrorCode result = io_text_write_tree(shop->root, file);
  if (result != SUCCESS) {
    fclose(file);
    return result;
  }

  int error = ferror(file) || (fflush(file) == EOF) || (fclose(file) == EOF);
  return error ? FILE_ERROR : SUCCESS;
}

Shop* io_text_read(const char* filename, ErrorCode* err) {
  FILE* file_ptr = fopen(filename, "r");
  if (!file_ptr) {
    if (err) *err = FILE_ERROR;
    return NULL;
  }

  Game** array = NULL;
  int count = 0;
  Shop* shop = NULL;

  array = malloc(sizeof(Game*));
  if (!array) {
    if (err) *err = OUT_OF_MEMORY;
    goto cleanup;
  }

  char line[1000];
  while (fgets(line, sizeof(line), file_ptr)) {
    char namebuf[996];
    double price, revenue;
    if (sscanf(line, " \"%[^\"]\" %lf %lf ", namebuf, &price, &revenue) != 3) {
      continue;
    }

    Game* game = create_game(namebuf, price, err);
    if (!game) {
      if (err) *err = OUT_OF_MEMORY;
      goto cleanup;
    }
    game->revenue = revenue;

    Game** newarray = realloc(array, sizeof(Game*) * (count + 1));
    if (!newarray) {
      free_game(game);
      if (err) *err = OUT_OF_MEMORY;
      goto cleanup;
    }
    array = newarray;
    array[count] = game;
    count++;
  }

  fclose(file_ptr);
  file_ptr = NULL;

  shop = (Shop*)malloc(sizeof(Shop));
  if (!shop) {
    if (err) *err = OUT_OF_MEMORY;
    goto cleanup;
  }
  init_shop(shop);

  if (count == 0) {
    if (err) *err = SUCCESS;
    goto cleanup;
  }

  // Sort array by name for balanced tree
  qsort(array, count, sizeof(Game*), cmp_game_name);

  shop->root = bst_build_from_sorted_array(array, 0, count - 1, err);
  if (!shop->root) {
    if (err && *err == SUCCESS) *err = OUT_OF_MEMORY;
    goto cleanup;
  }

  free(array);
  array = NULL;

  ErrorCode list_err = list_rebuild_from_bst(shop->root, &shop->revenue);
  if (list_err != SUCCESS) {
    if (err) *err = list_err;
    goto cleanup;
  }

  if (err) *err = SUCCESS;
  return shop;

cleanup:
  if (file_ptr) fclose(file_ptr);
  if (array) {
    for (int i = 0; i < count; i++) {
      freeGame(array[i]);
    }
    free(array);
  }
  if (shop) freeShop(shop);
  return NULL;
}

int cmp_game_name(const void* a, const void* b) {
  const Game* ga = *(const Game**)a;
  const Game* gb = *(const Game**)b;
  return strcmp(ga->name, gb->name);
}

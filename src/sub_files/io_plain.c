#include "../project.h"

ErrorCode io_text_write_game(Game* game, FILE* file_ptr) {
  if (fprintf(file_ptr, "\"%s\" %f %f\n", game->name, game->price,
              game->revenue) < 0) {
    return FILE_ERROR;
  }
  return SUCCESS;
}

#define TRY(expr)                     \
  do {                                \
    ErrorCode _err = (expr);          \
    if (_err != SUCCESS) return _err; \
  } while (0)
ErrorCode io_text_write_tree(Vertex* node, FILE* file_ptr) {
  if (node == NULL) {
    return SUCCESS;
  }
  TRY(io_text_write_tree(node->left, file_ptr));
  TRY(io_text_write_game(node->game, file_ptr));
  TRY(io_text_write_tree(node->right, file_ptr));

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
  if (err) *err = SUCCESS;
  FILE* file_ptr = fopen(filename, "r");
  if (!file_ptr) {
    if (err) *err = FILE_ERROR;
    return NULL;
  }

  int count = 0;

  char line[MAX_STRING_LENGTH];
  size_t capacity = 1;
  Game** array = malloc(capacity * sizeof(Game*));
  if (!array) {
    if (err) *err = OUT_OF_MEMORY;
    fclose(file_ptr);
    return NULL;
  }

  while (fgets(line, sizeof(line), file_ptr)) {
    char namebuf[MAX_STRING_LENGTH];
    double price, revenue;
    if (sscanf(line, " \"%[^\"]\" %lf %lf ", namebuf, &price, &revenue) != 3) {
      if (err) *err = PARSE_ERROR;
      goto cleanup;
    }
    if (price < 0 || revenue < 0) {
      if (err) *err = PARSE_ERROR;
      goto cleanup;
    }

    Game* game = create_game(namebuf, price, err);
    if (!game) {
      goto cleanup;
    }
    game->revenue = revenue;
    // memory efficient
    /*Game** newarray = realloc(array, sizeof(Game*) * (count + 1));
     if (!newarray) {
        free_game(game);
        if (err) *err = OUT_OF_MEMORY;
        goto cleanup;
      }
      array = newarray;*/
    // runtime efficiency
    if (count == capacity) {
      capacity *= 2;
      Game** newarray = realloc(array, capacity * sizeof(Game*));
      if (!newarray) {
        free_game(game);
        if (err) *err = OUT_OF_MEMORY;
        goto cleanup;
      }
      array = newarray;
    }
    array[count] = game;
    count++;
  }

  if (ferror(file_ptr)) {
    if (err) *err = FILE_ERROR;
    goto cleanup;
  }

  fclose(file_ptr);
  file_ptr = NULL;

  Shop* shop = (Shop*)malloc(sizeof(Shop));

  if (!shop) {
    if (err) *err = OUT_OF_MEMORY;
    goto cleanup;
  }
  init_shop(shop);

  if (count == 0) {
    free(array);
    if (err) *err = SUCCESS;
    return shop;
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
      free_game(array[i]);
    }
    free(array);
  }
  if (shop) free_shop(shop);
  if (err && *err == SUCCESS) {
    *err = FILE_ERROR;
  }
  return NULL;
}

int cmp_game_name(const void* a, const void* b) {
  const Game* ga = *(const Game**)a;
  const Game* gb = *(const Game**)b;
  return strcmp(ga->name, gb->name);
}

#include "../project.h"

ErrorCode io_binary_write_game(const Game* game, FILE* file_ptr) {
  if (!game || !game->name) {
    return INVALID_OBJECT;
  }
  size_t len = strlen(game->name);

  if (fwrite(&len, sizeof(len), 1, file_ptr) != 1) return FILE_ERROR;
  if (fwrite(game->name, 1, len, file_ptr) != len) return FILE_ERROR;
  double price_revenue[2] = {game->price, game->revenue};
  if (fwrite(price_revenue, sizeof(double), 2, file_ptr) != 2)
    return FILE_ERROR;
  return SUCCESS;
}

ErrorCode io_binary_write_tree(Vertex* node, FILE* file_ptr) {
  if (node == NULL) {
    return SUCCESS;
  }

  if (node->game == NULL) {
    return INVALID_OBJECT;
  }

  ErrorCode left = io_binary_write_tree(node->left, file_ptr);
  if (left != SUCCESS) return left;

  ErrorCode game_result = io_binary_write_game(node->game, file_ptr);
  if (game_result != SUCCESS) return game_result;

  ErrorCode right = io_binary_write_tree(node->right, file_ptr);
  if (right != SUCCESS) return right;

  return SUCCESS;
}

ErrorCode io_binary_write(const char* filename, const Shop* shop) {
  if (!shop) return INVALID_OBJECT;

  FILE* file_ptr = fopen(filename, "wb");
  if (!file_ptr) return FILE_ERROR;

  ErrorCode result = io_binary_write_tree(shop->root, file_ptr);
  if (result != SUCCESS) {
    fclose(file_ptr);
    return result;
  }

  int error = ferror(file_ptr) || (fflush(file_ptr) == EOF) ||
              (fclose(file_ptr) == EOF);

  return error ? FILE_ERROR : SUCCESS;
}

Game* io_binary_read_game(FILE* file_ptr, ErrorCode* err) {
  // read length of name
  size_t len;
  if (fread(&len, sizeof(len), 1, file_ptr) != 1) {
    goto whaterror;
  }

  if (len == 0 || len >= MAX_STRING_LENGTH) {
    if (err) *err = READ_ERROR;
    return NULL;
  }
  // read name
  char* name = malloc(len + 1);
  if (!name) {
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }

  if (fread(name, 1, len, file_ptr) != len) {
    free(name);
    goto whaterror;
  }
  name[len] = '\0';
  // read price and revenue into an array acting as a temporary tuple
  double price_revenue[2];
  if (fread(price_revenue, sizeof(double), 2, file_ptr) != 2) {
    free(name);
    goto whaterror;
  }
  if (price_revenue[0] < 0 || price_revenue[1] < 0) {
    free(name);
    if (err) *err = PARSE_ERROR;
    return NULL;
  }

  Game* game = create_game(name, price_revenue[0], err);
  free(name);
  if (!game) {
    if (err) *err = OUT_OF_MEMORY;
    return NULL;
  }
  game->revenue = price_revenue[1];
  if (err) *err = SUCCESS;
  return game;

whaterror:
  if (feof(file_ptr)) {
    if (err) *err = IO_EOF;
  } else {
    if (err) *err = FILE_ERROR;
  }
  return NULL;
}

Shop* io_binary_read(const char* filename, ErrorCode* err) {
  if (err) *err = SUCCESS;

  FILE* file_ptr = fopen(filename, "rb");
  if (!file_ptr) {
    if (err) *err = FILE_ERROR;
    return NULL;
  }

  size_t capacity = 1;
  Game** array = malloc(capacity * sizeof(Game*));

  if (!array) {
    if (err) *err = OUT_OF_MEMORY;
    fclose(file_ptr);
    return NULL;
  }

  int count = 0;
  Shop* shop = NULL;

  while (1) {
    ErrorCode read_err;

    Game* temp = io_binary_read_game(file_ptr, &read_err);
    if (!temp) {
      if (read_err == IO_EOF) {
        break;
      }
      if (err) *err = read_err;
      goto cleanup;
    }

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
        free_game(temp);
        if (err) *err = OUT_OF_MEMORY;
        goto cleanup;
      }
      array = newarray;
    }
    array[count] = temp;
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
    free(array);
    if (err) *err = SUCCESS;
    return shop;
  }
  qsort(array, count, sizeof(Game*), cmp_game_name);
  shop->root = bst_build_from_sorted_array(array, 0, count - 1, err);

  if (!shop->root) {
    if (err && *err == SUCCESS) *err = OUT_OF_MEMORY;
    goto cleanup;
  }

  ErrorCode list_err = list_rebuild_from_bst(shop->root, &shop->revenue);
  if (list_err != SUCCESS) {
    if (err) *err = list_err;
    goto cleanup;
  }

  free(array);
  array = NULL;

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
  return NULL;
}

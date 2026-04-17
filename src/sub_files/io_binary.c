#include "../project.h"

ErrorCode write_game_binary(Game* game, FILE* file_ptr) {
  size_t len = strlen(game->name);

  if (fwrite(&len, sizeof(len), 1, file_ptr) != 1) return FILE_ERROR;
  if (fwrite(game->name, 1, len, file_ptr) != len) return FILE_ERROR;
  double price_revenue[2] = {game->price, game->revenue};
  if (fwrite(price_revenue, sizeof(double), 2, file_ptr) != 2)
    return FILE_ERROR;
  return SUCCESS;
}

ErrorCode write_tree_binary(Node* node, FILE* file_ptr) {
  if (node == NULL) {
    return SUCCESS;
  }

  ErrorCode left = write_tree_binary(node->left, file_ptr);
  if (left != SUCCESS) return left;

  ErrorCode game_result = write_game_binary(node->game, file_ptr);
  if (game_result != SUCCESS) return game_result;

  ErrorCode right = write_tree_binary(node->right, file_ptr);
  if (right != SUCCESS) return right;

  return SUCCESS;
}

ErrorCode write_binary(const char* filename, const Shop* shop) {
  FILE* file_ptr = fopen(filename, "wb");
  if (!file_ptr) return FILE_ERROR;

  ErrorCode result = write_tree_binary(shop->root, file_ptr);
  if (result != SUCCESS) {
    fclose(file_ptr);
    return result;
  }

  int error = ferror(file_ptr) || (fflush(file_ptr) == EOF) ||
              (fclose(file_ptr) == EOF);

  return error ? FILE_ERROR : SUCCESS;
}

static void free_game_array(Game* array, int count) {
  for (int i = 0; i < count; i++) {
    free(array[i].name);
  }
  free(array);
}

Game* read_game_binary(FILE* file_ptr, ErrorCode* err) {
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

  Game* game = createGame(name, price_revenue[0], err);
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

Shop* read_binary(const char* filename, ErrorCode* err) {
  FILE* file_ptr = fopen(filename, "rb");
  if (!file_ptr) {
    if (err) *err = FILE_ERROR;
    return NULL;
  }

  Game* array = NULL;
  int count = 0;
  Shop* shop = NULL;

  while (1) {
    ErrorCode read_err;
    Game* temp = read_game_binary(file_ptr, &read_err);
    if (!temp) {
      if (read_err == IO_EOF) {
        break;
      }
      err = err;  // keep warning-free
      if (err) *err = read_err;
      goto cleanup;
    }

    Game* newarray = realloc(array, sizeof(Game) * (count + 1));
    if (!newarray) {
      freeGame(temp);
      if (err) *err = OUT_OF_MEMORY;
      goto cleanup;
    }

    array = newarray;
    array[count] = *temp;
    free(temp);
    count++;
  }

  fclose(file_ptr);
  file_ptr = NULL;

  shop = (Shop*)malloc(sizeof(Shop));
  if (!shop) {
    if (err) *err = OUT_OF_MEMORY;
    goto cleanup;
  }

  initShop(shop);
  if (count == 0) {
    if (err) *err = SUCCESS;
    return shop;
  }

  shop->root = build_balanced(&array, 0, count - 1, err);
  if (!shop->root) {
    if (err && *err == SUCCESS) *err = OUT_OF_MEMORY;
    goto cleanup;
  }

  free_game_array(array, count);
  array = NULL;

  ErrorCode list_err;
  buildRevenueListFromTree(shop->root, &shop->revenue, &list_err);
  if (list_err != SUCCESS) {
    if (err) *err = list_err;
    goto cleanup;
  }

  if (err) *err = SUCCESS;
  return shop;

cleanup:
  if (file_ptr) fclose(file_ptr);
  if (array) free_game_array(array, count);
  if (shop) freeShop(shop);
  return NULL;
}

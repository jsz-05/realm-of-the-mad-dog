#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "list.h"
#include "sdl_wrapper.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 18;
const size_t INITIAL_CAPACITY = 5;

typedef struct {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

static void asset_cache_free_entry(entry_t *entry) {
  switch (entry->type) {
  case ASSET_IMAGE: {
    SDL_DestroyTexture(entry->obj);
    break;
  }
  case ASSET_FONT: {
    TTF_CloseFont(entry->obj);
    break;
  }
  case ASSET_BUTTON: {
    asset_destroy(entry->obj);
    break;
  }
  }

  free(entry);
}

void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() { list_free(ASSET_CACHE); }

void *check_present(const char *filepath) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (!entry->filepath || !filepath) {
      continue;
    }
    if (strcmp(entry->filepath, filepath) == 0) {
      return entry->obj;
    }
  }
  return NULL;
}

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  void *ret = check_present(filepath);
  if (ret) {
    return ret;
  }

  entry_t *entry = malloc(sizeof(entry_t));
  assert(entry);
  entry->type = ty;
  entry->filepath = filepath;

  if (ty == ASSET_IMAGE) {
    entry->obj = image_to_texture(filepath);
  } else if (ty == ASSET_FONT) {
    entry->obj = TTF_OpenFont(filepath, FONT_SIZE);
  }

  list_add(ASSET_CACHE, entry);

  return entry->obj;
}

void asset_cache_register_button(asset_t *button) {
  assert(asset_get_type(button) == ASSET_BUTTON);

  entry_t *button_entry = malloc(sizeof(entry_t));
  assert(button_entry);

  button_entry->type = ASSET_BUTTON;
  button_entry->filepath = NULL;
  button_entry->obj = button;

  list_add(ASSET_CACHE, button_entry);
}

void asset_cache_handle_buttons(state_t *state, double x, double y) {
  size_t size_of_asset_cache = list_size(ASSET_CACHE);

  for (size_t i = 0; i < size_of_asset_cache; ++i) {
    entry_t *current_cache = list_get(ASSET_CACHE, i);
    if (current_cache->type == ASSET_BUTTON) {
      asset_on_button_click(current_cache->obj, state, x, y);
    }
  }
}
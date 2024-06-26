#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *text;
  rgb_color_t color;
} text_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
  double angle; // Add angle to the struct
} image_asset_t;

typedef struct button_asset {
  asset_t base;
  image_asset_t *image_asset;
  text_asset_t *text_asset;
  button_handler_t handler;
  bool is_rendered;
} button_asset_t;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  asset_t *new;
  switch (ty) {
  case ASSET_IMAGE: {
    new = malloc(sizeof(image_asset_t));
    break;
  }
  case ASSET_FONT: {
    new = malloc(sizeof(text_asset_t));
    break;
  }
  case ASSET_BUTTON: {
    new = malloc(sizeof(button_asset_t));
    break;
  }
  default: {
    assert(false && "Unknown asset type");
  }
  }
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

asset_t *asset_make_image_with_body(const char *filepath, SDL_Rect bounding_box,
                                    body_t *body) {
  image_asset_t *img = (image_asset_t *)asset_init(ASSET_IMAGE, bounding_box);
  img->texture = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  img->body = body; // Set body to the given body

  return (asset_t *)img;
}

asset_t *asset_make_image_with_body_angle(const char *filepath, SDL_Rect bounding_box,
                                    body_t *body, double angle) { // Add angle parameter
  image_asset_t *img = (image_asset_t *)asset_init(ASSET_IMAGE, bounding_box);
  img->texture = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  img->body = body; // Set body to the given body
  img->angle = angle; // Initialize angle

  return (asset_t *)img;
}

asset_t *asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  return asset_make_image_with_body_angle(filepath, bounding_box, NULL, 0);
}

asset_type_t asset_get_type(asset_t *asset) {
  assert(asset);
  return asset->type;
}

asset_t *asset_make_text(const char *filepath, SDL_Rect bounding_box,
                         const char *text, rgb_color_t color) {
  text_asset_t *txt = (text_asset_t *)asset_init(ASSET_FONT, bounding_box);

  txt->color = color;
  txt->text = text;
  txt->font = asset_cache_obj_get_or_create(ASSET_FONT, filepath);

  return (asset_t *)txt;
}

asset_t *asset_make_button(SDL_Rect bounding_box, asset_t *image_asset,
                           asset_t *text_asset, button_handler_t handler) {
  button_asset_t *new_button =
      (button_asset_t *)asset_init(ASSET_BUTTON, bounding_box);

  assert(image_asset != NULL && asset_get_type(image_asset) == ASSET_IMAGE);

  new_button->handler = handler;
  new_button->image_asset = (image_asset_t *)image_asset;
  new_button->text_asset = (text_asset_t *)text_asset;
  new_button->is_rendered = true;

  return (asset_t *)new_button;
}

body_t *asset_get_body(asset_t *asset) {
  assert(asset_get_type(asset) == ASSET_IMAGE);
  image_asset_t *img_asset = (image_asset_t *) asset;
  return img_asset->body;
}

void asset_on_button_click(asset_t *button, state_t *state, double x,
                           double y) {
  button_asset_t *current_button = (button_asset_t *)button;

  if (!current_button->is_rendered) {
    return;
  }

  SDL_Rect *box = &button->bounding_box;
  if (x >= box->x && x <= (box->x + box->w) && y >= box->y &&
      y <= (box->y + box->h)) {
    if (current_button->handler != NULL) {
      current_button->handler(state);
      current_button->is_rendered = false;
    }
  }
}

void asset_image_render(asset_t *asset) {
  image_asset_t *img_asset = (image_asset_t *) asset;
  // Render image with rotation
  sdl_render_image_rotated(img_asset->texture, asset->bounding_box, img_asset->angle); 
}

void asset_text_render(asset_t *asset) {
  text_asset_t *txt_asset = (text_asset_t *)asset;
  sdl_render_text(txt_asset->text, txt_asset->font, asset->bounding_box);
}

void asset_render(asset_t *asset) {
  switch (asset->type) {
  case ASSET_IMAGE: {
    image_asset_t *img_asset = (image_asset_t *)asset;
    if (img_asset->body != NULL) {
      // Set the bounding box to the body's bounding box      
      asset->bounding_box = sdl_get_bounding_box(img_asset->body);
    }
    asset_image_render(asset);
    break;
  }
  case ASSET_FONT: {
    asset_text_render(asset);
    break;
  }
  case ASSET_BUTTON: {
    button_asset_t *current_button = (button_asset_t *)asset;
    if (current_button->image_asset != NULL) {
      asset_image_render((asset_t *)current_button->image_asset);
    }
    if (current_button->text_asset != NULL) {
      asset_text_render((asset_t *)current_button->text_asset);
    }
    current_button->is_rendered = true;
    break;
  }
  }
}

void asset_destroy(asset_t *asset) { free(asset); }
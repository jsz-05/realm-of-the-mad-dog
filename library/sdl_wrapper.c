#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "asset_cache.h"
#include "sdl_wrapper.h"

const size_t BLUE_VALUE = 255;
SDL_Color WHITE = {255, 255, 255};

const char WINDOW_TITLE[] = "CS 3";
const size_t WINDOW_WIDTH = 1000;
const size_t WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

const char *MUSIC_PATH = "assets/music.ogg";
const size_t FREQUENCY = 22050;
const size_t CHANNELS = 2;
const size_t CHUNK_SIZE = 4096;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The music object loaded at the start of the game.
*/
Mix_Music *music;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * The mouse press handler, or NULL if none has been configured.
 */
mouse_handler_t mouse_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE_BAR;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

SDL_Rect sdl_get_bounding_box(body_t *body) {
  list_t *shape = body_get_shape(body);
  double min_x = __DBL_MAX__;
  double max_x = -__DBL_MAX__;
  double min_y = __DBL_MAX__;
  double max_y = -__DBL_MAX__;

  // Iterate over all points in the body
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t *point = list_get(shape, i);
    if (point->x < min_x) {
      min_x = point->x;
    }
    if (point->x > max_x) {
      max_x = point->x;
    }
    if (point->y < min_y) {
      min_y = point->y;
    }
    if (point->y > max_y) {
      max_y = point->y;
    }
  }

  vector_t window_center = get_window_center();
  vector_t top_left =
      get_window_position((vector_t){min_x, max_y}, window_center);
  vector_t bottom_right =
      get_window_position((vector_t){max_x, min_y}, window_center);

  SDL_Rect bounding_box;
  bounding_box.x = top_left.x;
  bounding_box.y = top_left.y;
  bounding_box.w = bottom_right.x - top_left.x;
  bounding_box.h = bottom_right.y - top_left.y;

  return bounding_box;
}

void sdl_start_music() {
  if (Mix_OpenAudio(FREQUENCY, MIX_DEFAULT_FORMAT, CHANNELS, CHUNK_SIZE) == -1) {
    return;
  }

  music = Mix_LoadMUS(MUSIC_PATH);

  if (music == NULL) {
    return;
  }

  if (Mix_PlayMusic( music, -1) == -1) {
    return;
  }
}

void sdl_stop_music() {
  Mix_FreeMusic(music);
  Mix_CloseAudio();
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  TTF_Init();

  sdl_start_music();
}

bool sdl_is_done(void *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
      case SDL_QUIT: {
        free(event);
        return true;
      }
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        // Skip the keypress if no handler is configured
        // or an unrecognized key was pressed
        if (key_handler == NULL)
          break;
        char key = get_keycode(event->key.keysym.sym);
        if (key == '\0')
          break;

        uint32_t timestamp = event->key.timestamp;
        if (!event->key.repeat) {
          key_start_timestamp = timestamp;
        }
        key_event_type_t type =
            event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
        double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
        key_handler(key, type, held_time, state);
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        if (mouse_handler == NULL) {
          break;
        }
        if (event->button.button == SDL_BUTTON_LEFT) {
          mouse_handler(MOUSE_LEFT, event->motion.x, event->motion.y, state);
        }
        else if(event->button.button == SDL_BUTTON_RIGHT) {
          mouse_handler(MOUSE_RIGHT, event->motion.x, event->motion.y, state);
        }
        break;
      }
      default: {
        break;
      }
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(polygon_t *poly, rgb_color_t color) {
  list_t *points = polygon_get_points(poly);
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  assert(boundary);
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene, void *aux) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    polygon_t *poly = polygon_init(shape, (vector_t){0, 0}, 0, 0, 0, 0);
    sdl_draw_polygon(poly, *body_get_color(body));
    list_free(shape);
  }
  if (aux != NULL) {
    body_t *body = aux;
    sdl_draw_polygon(body_get_polygon(body), *body_get_color(body));
  }
  sdl_show();
}

void sdl_render_text(const char *txt, TTF_Font *font, SDL_Rect message_rect) {
  SDL_Surface *surface_message = TTF_RenderText_Solid(font, txt, WHITE);
  SDL_Texture *message = SDL_CreateTextureFromSurface(renderer, surface_message);
  SDL_RenderCopy(renderer, message, NULL, &message_rect);
  SDL_FreeSurface(surface_message);
  SDL_DestroyTexture(message);
}

void sdl_render_text_color(const char *txt, TTF_Font *font, SDL_Rect message_rect, 
                           SDL_Color color) {
    SDL_Surface *surfaceMessage = TTF_RenderText_Solid(font, txt, color);
    SDL_Texture *message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    message_rect.w = surfaceMessage->w;
    message_rect.h = surfaceMessage->h;
    SDL_RenderCopy(renderer, message, NULL, &message_rect);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(message);
}

void sdl_render_image(SDL_Texture *texture, SDL_Rect image_rect) {
  SDL_RenderCopy(renderer, texture, NULL, &image_rect);
}

void sdl_render_image_rotated(SDL_Texture *texture, SDL_Rect image_rect, double angle) {
    // The center of rotation is the center of the image
    SDL_Point center = {image_rect.w / 2, image_rect.h / 2};
    // Render the image with the given rotation
    SDL_RenderCopyEx(renderer, texture, NULL, &image_rect, angle, &center, SDL_FLIP_NONE);
}


SDL_Texture *image_to_texture(const char *file_path) {
  return IMG_LoadTexture(renderer, file_path);
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

void sdl_on_mouse(mouse_handler_t m_handler) { mouse_handler = m_handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

void sdl_draw_bar(size_t current_value, size_t max_health, SDL_Rect bar_rect, 
                  SDL_Color health_color, SDL_Color lost_color) {
    // Calculate the width of the health portion and the lost portion
    int health_width = (int)((double)current_value / max_health * bar_rect.w);
    int lost_width = bar_rect.w - health_width;

    // Set up the rectangles for the health and lost portions
    SDL_Rect health_rect = {bar_rect.x, bar_rect.y, health_width, bar_rect.h};
    SDL_Rect lost_rect = {bar_rect.x + health_width, bar_rect.y, lost_width, bar_rect.h};

    // Render the health portion
    SDL_SetRenderDrawColor(renderer, health_color.r, health_color.g, health_color.b, 255);
    SDL_RenderFillRect(renderer, &health_rect);

    // Render the lost portion
    SDL_SetRenderDrawColor(renderer, lost_color.r, lost_color.g, lost_color.b, 255);
    SDL_RenderFillRect(renderer, &lost_rect);
}


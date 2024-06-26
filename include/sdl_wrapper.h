#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4,
  SPACE_BAR = 5
} arrow_key_t;

/**
 * Gets the bounding box for a body.
 *
 * @param body a body object
 * @return the smallest SDL_Rect that can cover the body entirely
 */
SDL_Rect sdl_get_bounding_box(body_t *body);

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * The possible types of mouse events.
 * #define MOUSE_PRESSED 0
 * #define MOUSE_RELEASE 1
 */
typedef enum { MOUSE_LEFT, MOUSE_RIGHT } mouse_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time,
                              void *state);

/**
 * A mousepress handler.
 * When a mouse is pressed or released, the handler is passed a mouse_event
 * (either pressed or unpressed), the x and y locations of the press,
 * and the state.
 * @param type the type of mouse event (MOUSE_PRESSED or MOUSE_RELEASED)
 * @param x_loc the x-coordinate of where the mouse was pressed
 * @param y_loc the y-coordinate of where the mouse was pressed
 * @param state the state that the mouse event is applied to
 */
typedef void (*mouse_handler_t)(mouse_event_type_t type, double x_loc,
                                double y_loc, void *state);

/**
 * Initializes the SDL music mixers. 
*/ 
void sdl_start_music(); 
            
/**
 * Stops the background music and frees it. Also closes the SDL music mixers.  
*/ 
void sdl_stop_music(); 

/**
 * Initializes the SDL window, renderer, and music mixers.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle inputs.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param poly a struct representing the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(polygon_t *poly, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param aux an additional body to draw (can be NULL if no additional bodies)
 */
void sdl_render_scene(scene_t *scene, void *aux);

void sdl_render_text(const char *txt, TTF_Font *font, SDL_Rect message_rect);

void sdl_render_text_color(const char *txt, TTF_Font *font, SDL_Rect message_rect, 
                           SDL_Color color);

void sdl_render_image(SDL_Texture *texture, SDL_Rect image_rect);

void sdl_render_image_rotated(SDL_Texture *texture, SDL_Rect image_rect, double angle);

SDL_Texture *image_to_texture(const char *file_path);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Registers a function to be called every time a mouse button is pressed.
 * Overwrites any existing handler.
**/
void sdl_on_mouse(mouse_handler_t m_handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);


/**
 * Draws the bars.
 * 
 * @param current_value value for the proportion of the bar that is positive.
 * @param max_health the maximum width of the bar
 * @param bar_rect the rectangle of which the bar is contained in
 * @param health_color the color of the positive portion
 * @param lost_color the color of the negative portion
 * @return the number of seconds that have elapsed
 */
void sdl_draw_bar(size_t current_value, size_t max_health, SDL_Rect bar_rect, 
                  SDL_Color health_color, SDL_Color lost_color);


#endif // #ifndef __SDL_WRAPPER_H__

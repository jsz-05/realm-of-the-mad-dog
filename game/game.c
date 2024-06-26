#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "asset.h"
#include "asset_cache.h"
#include "sdl_wrapper.h"
#include "player.h"
#include "projectile.h" 
#include "collision.h" 
#include "forces.h" 
#include "enemy.h"
#include "boss.h"
#include "portal.h"

// Movement speed constants
const double H_STEP = 80;
const double V_STEP = 80;

const vector_t START_POS = {500, 30};
const vector_t RESET_POS = {500, 45};
const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};
const char *BOSS_BACKGROUND_PATH = "assets/bossbackground.png";
const char *OVERWORLD_PATH = "assets/overworld.png";
const char *INTERFACE_PATH = "assets/interface.png";
const char *STARTSCREEN_PATH = "assets/startscreen.png";
const char *DEATHSCREEN_PATH = "assets/deathoverlay.png";
const char *WINSCREEN_PATH = "assets/winoverlay.png";

const SDL_Rect PLAYBUTTON_SIZE = (SDL_Rect){(MAX.x / 2) - 30, 440, 75, 38};
const SDL_Rect RESTARTBUTTON_SIZE = (SDL_Rect){(MAX.x / 2) - 30, 440, 75, 38};

const rgb_color_t projectile_color = (rgb_color_t){0.1, 0.9, 0.2};
const char *PLAYER_PATH = "assets/wizzy.png";
const char *PLAYER_BULLET_PATH = "assets/playerattack.png";
const char *ENEMY_PATH = "assets/zombie.png";
const char *ENEMY_BULLET_PATH = "assets/zombiebullet.png";
const char *HUSKY_PATH = "assets/husky.png";
const char *HUSKY_BULLET_PATH = "assets/bossshuriken.png";
const char *HUSKY_RAY_PATH = "assets/bossray.png";
const char *PORTAL_PATH_BOSS = "assets/bossportal.png";
const char *PORTAL_PATH_END = "assets/endportal.png";

// Game stats interface
const SDL_Rect HP_BAR = {.x = 260, .y = 10, .w = 150, .h = 23};
const SDL_Rect XP_BAR = {.x = 425, .y = 10, .w = 150, .h = 23};
const SDL_Rect BULLETS_BAR = {.x = 590, .y = 10, .w = 150, .h = 23};
const SDL_Rect BOSS_BAR = {.x = (1000 - 30) / 2, .y = 290, .w = 30, .h = 5};
const size_t BAR_MARGIN = 7;
const size_t BAR_LIST_SIZE_INIT = 50;
const char *XP_BAR_TEXT = "XP: %zu/%zu";
const char *HP_BAR_TEXT = "HP: %zu/100";
const char *BULLETS_BAR_TEXT = "Bullets: %zu/%zu";
const char *RELOAD_BAR_TEXT = "Reload: (%zus)";

const char *FONT_PATH = "assets/summerpixel.ttf";
const size_t TEXT_SIZE = 24;
const SDL_Color GREEN = {20, 200, 5, 255};
const SDL_Color RED = {186, 7, 7, 255};
const SDL_Color BLUE = {0, 0, 255, 255};
const SDL_Color GREY = {128, 128, 128, 255};
const SDL_Color BLACK = {0, 0, 0, 255};
const SDL_Color WHTE = {255, 255, 255, 255};
const SDL_Color PURPLE = {161, 73, 255, 185};

const size_t PLAYER_MELEE_RANGE = 10;
const size_t PLAYER_MAX_HEALTH = 100;
const double PLAYER_BULLET_COOLDOWN = 3.0; // In seconds
const double PLAYER_MAX_BULLETS = 5;
const double PROJECTILE_SPEED = 100; // Speed of the projectiles
const double PROJECTILE_ANGLE = -45; // Angle of the projectiles in degrees

const double ENEMY_SPAWN_TIME = 3.0; // In seconds
const double ENEMY_ATTACK_TIME = 2.0; // In seconds
const double ENEMY_DODGE_SPEED = 75.0;
const double ENEMY_DODGE_RADIUS = 200.0;
const double ENEMY_STOP_RADIUS = 50.0; 
const double ENEMY_SPEED = 75.0; 

const size_t MAX_BOSS_HEALTH = 5000;
const double BOSS_RING_TIME = 15.0; // In seconds
const double BOSS_RAY_TIME = 5.0; // In seconds
const size_t SPAWN_THRESHOLD = 20; // Enemies to kill before the boss spawns

const double INIT_TIME = 1; // Starting representative clock time
const size_t BODY_ASSETS = 4;

const size_t NUM_BUTTONS = 2;

struct state {
    list_t *body_assets;
    scene_t *scene;
    player_t *player; // Player in the state
    list_t *projectiles; // Projectiles in the state
    list_t *enemies; // Enemies in the state
    boss_t *boss; // Boss in the state
    portal_t *portal;
    TTF_Font *font;
    size_t bullets_fired;
    size_t enemies_killed;
    
    asset_t *start_screen;
    asset_t *lose_screen;
    asset_t *win_screen;
    asset_t *play_button;
    asset_t *restart_button;
    asset_t *overworld_image;
    asset_t *boss_background_image;

    bool boss_spawned;
    bool portal_spawned;
    bool game_over;

    double time_since_spawn;
    double time_since_atk;
    double time_since_boss_ring;
    double time_since_boss_ray;
    double time_since_cooldown_start;
};

typedef struct button_info {
  const char *image_path;
  SDL_Rect image_box;
  button_handler_t handler;
} button_info_t;

/**
 * Wraps a specific body around the edge if needed. 
 * 
 * @param body the body to be wrapped
 * @param state the current state of the game
*/
void handle_edges(body_t *body, state_t *state) {
    vector_t centroid = body_get_centroid(body);
    bool out_of_bounds = false;

    if (centroid.x > MAX.x) {
        out_of_bounds = true;
    } else if (centroid.x < MIN.x) {
        out_of_bounds = true;
    } else if (centroid.y > MAX.y) {
        out_of_bounds = true;
    } else if (centroid.y < MIN.y) {
        out_of_bounds = true;
    }

    if (out_of_bounds) {
        // Check if the body is a projectile
        bool is_projectile = false;
        for (size_t i = 0; i < list_size(state->projectiles); i++) {
            projectile_t *projectile = list_get(state->projectiles, i);
            body_t *projectile_body = projectile_get_hitbox(projectile);
            if (body == projectile_body) {
                is_projectile = true;
                break;
            }
        }
        if (is_projectile) {
            body_remove(body);
        } else {
            // If it's not a projectile, stick it at the edge
            if (centroid.x > MAX.x) {
                body_set_centroid(body, (vector_t){MAX.x, centroid.y});
            } else if (centroid.x < MIN.x) {
                body_set_centroid(body, (vector_t){MIN.x, centroid.y});
            } else if (centroid.y > MAX.y) {
                body_set_centroid(body, (vector_t){centroid.x, MAX.y});
            } else if (centroid.y < MIN.y) {
                body_set_centroid(body, (vector_t){centroid.x, MIN.y});
            }
        }
    }
}

/**
 * The key handler for player controls. 
 * 
 * @param key the key pressed
 * @param type the type of event (pressed or released)
 * @param held_time the time the key was held
 * @param state the state of the game
*/
void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
    body_t *player_body = player_get_hitbox(state->player);
    vector_t velocity = body_get_velocity(player_body);
    if (type == KEY_PRESSED) {
        switch (key) {
            case 'a':
            case 'A':
                velocity.x = -H_STEP;
                break;
            case 'd':
            case 'D':
                velocity.x = H_STEP;
                break;
            case 'w':
            case 'W':
                velocity.y = V_STEP;
                break;
            case 's':
            case 'S':
                if (body_get_centroid(player_body).y > START_POS.y) {
                    velocity.y = -V_STEP;
                }
                break;
        }
    } else if (type == KEY_RELEASED) {
        switch (key) {
            case 'a':
            case 'A':
            case 'd':
            case 'D':
                velocity.x = 0;
                break;
            case 'w':
            case 'W':
            case 's':
            case 'S':
                velocity.y = 0;
                break;
        }
    }
    body_set_velocity(player_body, velocity);
}

/**
 * Renders the ranged attack of the player on the screen. To be called upon click. 
 * 
 * @param state the current state of the game
 * @param mouse_loc the current location of the cursor on the screen
*/
void render_player_ranged_attack(state_t *state, vector_t mouse_loc);

/**
 * Registers a melee attack for the player. To be called upon click. 
 * 
 * @param state the current state of the game
*/
void render_player_melee_attack(state_t *state);

void on_mouse(mouse_event_type_t type, double x_loc, double y_loc, state_t *state) {
    switch (scene_get_type(state->scene)) {
        case SCENE_MENU: {
            asset_cache_handle_buttons(state, x_loc, y_loc);
            break;
        } 
        case SCENE_GAME: 
        case SCENE_BOSS: {
            vector_t mouse_location = {.x = x_loc, .y = MAX.y - y_loc}; // y is shifted
            if (type == MOUSE_LEFT) {
                render_player_melee_attack(state);
            } 
            else if (type == MOUSE_RIGHT) {
                render_player_ranged_attack(state, mouse_location);
            }
            break;
        }
        case SCENE_GAME_OVER_LOSS:
        case SCENE_GAME_OVER_WIN: {
            break;
        }
    }
}

/**
 * The collision handler for enemy attacks upon player. 
 * 
 * @param player_body the body of the player
 * @param projectile_body the body of the enemy projectile
 * @param axis the axis (unused)
 * @param aux auxiliary component (unused)
 * @param force_const the force constant for the collision (unused)
*/
void player_projectile_collision_handler(body_t *player_body, body_t *projectile_body, 
                                         vector_t axis, void *aux, double force_const) {
    // Get the player and projectile structures
    player_t *player = body_get_info(player_body);
    projectile_t *projectile = body_get_info(projectile_body);

    // Decrease the player's health by the projectile's damage
    size_t player_health = player_get_health(player);
    size_t projectile_damage = projectile_get_damage(projectile);
    if (player_health >= projectile_damage) {
        player_set_health(player, player_health - projectile_damage);
    } else {
        player_set_health(player, 0);
    }

    body_remove(projectile_body);
}

/**
 * The collision handler for the player attacks upon the enemy. 
 * 
 * @param enemy_body the body of the enemy
 * @param projectile_body the body of the player projectile
 * @param axis the axis (unused)
 * @param aux auxiliary component (unused)
 * @param force_const the force constant for the collision (unused)
*/
void enemy_projectile_collision_handler(body_t *enemy_body, body_t *projectile_body, 
                                        vector_t axis, void *aux, double force_const) {
    body_remove(projectile_body);
    body_remove(enemy_body);
}

/**
 * The collision handler for the player attacks upon the boss. 
 * 
 * @param boss_body the body of the boss
 * @param projectile_body the body of the projectile
 * @param axis the axis (unused)
 * @param aux auxiliary component (unused)
 * @param force_const the force constant for the collision (unused)
*/
void boss_projectile_collision_handler(body_t *boss_body, body_t *projectile_body, 
                                       vector_t axis, void *aux, double force_const) {
    boss_t *current_boss = body_get_info(boss_body);

    size_t current_boss_health = boss_get_health(current_boss);
    projectile_t *current_projectile = body_get_info(projectile_body);
    size_t projectile_damage = projectile_get_damage(current_projectile);
    if (current_boss_health >= projectile_damage) {
        boss_set_health(current_boss, current_boss_health - projectile_damage);
    } else {
        boss_set_health(current_boss, 0);
    }

    body_remove(projectile_body);
}

/**
 * The collision handler for the player and a portal
 * 
 * @param player_body the body of the player
 * @param portal_body the body of the portal
 * @param axis the axis (unused)
 * @param aux auxiliary component (unused)
 * @param force_const the force constant for the collision (unused)
*/
void portal_handler(body_t *player_body, body_t *portal_body, vector_t axis, void *aux, 
                    double force_const) {
    collision_info_t info = find_collision(portal_body, player_body);
    if (info.collided) {
        portal_t *portal = body_get_info(portal_body);
        portal_set_status(portal, false);
        body_remove(portal_body);
    }
}

void render_player_ranged_attack(state_t *state, vector_t mouse_loc) {
    if (state->bullets_fired < PLAYER_MAX_BULLETS && 
        state->time_since_cooldown_start >= PLAYER_BULLET_COOLDOWN) {
        state->bullets_fired++;
        if (state->bullets_fired >= PLAYER_MAX_BULLETS) {
            state->time_since_cooldown_start = 0.0;
            state->bullets_fired = 0;
        }

        projectile_t *projectile = player_ranged_attack(state->player, mouse_loc);
        body_t *laser_body = projectile_get_hitbox(projectile);
        body_set_info(laser_body, projectile);

        scene_add_body(state->scene, laser_body);
        list_add(state->projectiles, projectile);

        asset_t *laser_image = 
        asset_make_image_with_body_angle(PLAYER_BULLET_PATH, 
                                         sdl_get_bounding_box(laser_body), 
                                         laser_body, 
                                         -1 * projectile_get_angle(projectile));
        list_add(state->body_assets, laser_image);

        // Make collisions with every existing enemy
        for (size_t i = 0; i < list_size(state->enemies); i++) {
            enemy_t *enemy = list_get(state->enemies, i);
            body_t *enemy_body = enemy_get_hitbox(enemy);
            create_collision(state->scene, enemy_body, laser_body, 
                             enemy_projectile_collision_handler, NULL, 1.0);
        }

        if (state->boss_spawned) {
            body_t *boss_body = boss_get_hitbox(state->boss);
            body_set_info(boss_body, state->boss);
            create_collision(state->scene, boss_body, laser_body, 
                             boss_projectile_collision_handler, NULL, 1.0);
        }
    } else {
        if (state->bullets_fired >= PLAYER_MAX_BULLETS) {
            state->time_since_cooldown_start = PLAYER_BULLET_COOLDOWN;
            state->bullets_fired = 0;
        }
    }
}

void render_player_melee_attack(state_t *state) {
    body_t *player_body = player_get_hitbox(state->player);

    size_t num_enemies = list_size(state->enemies);
    for (size_t i = 0; i < num_enemies; ++i) {
        enemy_t *current_enemy = list_get(state->enemies, i);
        body_t *current_enemy_body = enemy_get_hitbox(current_enemy);

        // Directly deals with collision here instead of making a collision since we
        // don't want to make an extra handler to deal with melee attacks.
        collision_info_t info = find_collision_melee(current_enemy_body, 
                                                     player_body, PLAYER_MELEE_RANGE);
        if (info.collided) {
            body_remove(current_enemy_body);
        }
    }
}

/**
 * Renders an enemy attack on the screen (at the location of the enemy)
 * 
 * @param state the current state of the game
 * @param enemy the enemy whose attack should be rendered. 
*/
void render_enemy_attack(state_t *state, enemy_t *enemy) {
    player_t *player = state->player;
    body_t *player_body = player_get_hitbox(player);
    vector_t player_loc = body_get_centroid(player_body);

    projectile_t *projectile = enemy_attack(enemy, player_loc);

    body_t *laser_body = projectile_get_hitbox(projectile);
    body_set_info(laser_body, projectile); // Store projectile_t pointer in info field

    scene_add_body(state->scene, laser_body);
    list_add(state->projectiles, projectile);

    asset_t *laser_image = 
    asset_make_image_with_body_angle(ENEMY_BULLET_PATH, sdl_get_bounding_box(laser_body), 
                                     laser_body, -projectile_get_angle(projectile)); 
    list_add(state->body_assets, laser_image);

    create_collision(state->scene, player_body, laser_body, 
                     player_projectile_collision_handler, NULL, 1.0);
}

/**
 * Renders the special ring attack of the boss on the screen
 * 
 * @param state the current state of the game
*/
void render_boss_ring_attack(state_t *state) {
    list_t *projectiles = boss_ring_move(state->boss);

    body_t *player_body = player_get_hitbox(state->player);

    for (size_t i = 0; i < list_size(projectiles); i++) {
        projectile_t *projectile = list_get(projectiles, i);

        body_t *laser_body = projectile_get_hitbox(projectile);
        body_set_info(laser_body, projectile); 

        scene_add_body(state->scene, laser_body);
        list_add(state->projectiles, projectile);

        asset_t *laser_image = 
        asset_make_image_with_body_angle(HUSKY_BULLET_PATH, 
                                         sdl_get_bounding_box(laser_body), 
                                         laser_body, 
                                         -1 * projectile_get_angle(projectile)); 
        list_add(state->body_assets, laser_image);

        create_collision(state->scene, player_body, laser_body, 
                         player_projectile_collision_handler, NULL, 1.0);
    }
}

/**
 * Renders the special ring attack of the boss on the screen
 * 
 * @param state the current state of the game
*/
void render_boss_ray_attack(state_t *state) {
    body_t *player_body = player_get_hitbox(state->player);
    list_t *projectiles = boss_ray_move(state->boss, body_get_centroid(player_body));

    for (size_t i = 0; i < list_size(projectiles); i++) {
        projectile_t *projectile = list_get(projectiles, i);

        body_t *laser_body = projectile_get_hitbox(projectile);
        body_set_info(laser_body, projectile); 

        scene_add_body(state->scene, laser_body);
        list_add(state->projectiles, projectile);

        asset_t *laser_image = 
        asset_make_image_with_body_angle(HUSKY_RAY_PATH, sdl_get_bounding_box(laser_body), 
                                         laser_body, -projectile_get_angle(projectile)); 
        list_add(state->body_assets, laser_image);

        create_collision(state->scene, player_body, laser_body, 
                         player_projectile_collision_handler, NULL, 1.0);
    }
}

/**
 * Spawns the boss at the center of the screen
 * 
 * @param state the current state of the game
*/
void spawn_boss(state_t *state) {
    boss_t *boss = boss_init((vector_t){MAX.x / 2, MAX.y / 2});
    body_t *boss_body = boss_get_hitbox(boss);

    scene_add_body(state->scene, boss_body);

    asset_t *boss_image = 
    asset_make_image_with_body(HUSKY_PATH, sdl_get_bounding_box(boss_body), boss_body);
    list_add(state->body_assets, boss_image);
    state->boss = boss;
}

/**
 * Spawns an enemy at a random location on the border of the map
 * 
 * @param state the current state of the game
 * @param damage the damage dealt by the enemy's attacks
*/
void spawn_enemy(state_t *state, size_t damage) {
    // Calculate random starting position on border
    double start_x;
    double start_y;
    if ((double) rand() / (double) RAND_MAX < 0.5) {
        start_x = (double) rand() / (double) RAND_MAX * MAX.x;
        start_y = (double) rand() / (double) RAND_MAX < 0.5 ? 0 : MAX.y;
    } else {
        start_x = (double) rand() / (double) RAND_MAX < 0.5 ? 0 : MAX.x;
        start_y = (double) rand() / (double) RAND_MAX * MAX.y;
    }

    enemy_t *enemy = enemy_init(damage, (vector_t){start_x, start_y}, 0, 0);
    body_t *enemy_body = enemy_get_hitbox(enemy);
    scene_add_body(state->scene, enemy_body);
    list_add(state->enemies, enemy);

    asset_t *enemy_image = 
    asset_make_image_with_body(ENEMY_PATH, sdl_get_bounding_box(enemy_body), enemy_body);
    list_add(state->body_assets, enemy_image);

    for (size_t i = 0; i < list_size(state->projectiles); i++) {
        projectile_t *projectile = list_get(state->projectiles, i);
        if (projectile_get_type(projectile) != PROJECTILE_PLAYER) {
            continue;
        }
        body_t *projectile_body = projectile_get_hitbox(projectile);

        create_collision(state->scene, enemy_body, projectile_body, 
                         enemy_projectile_collision_handler, NULL, 1.0);
    }
}

/**
 * Spawns a portal at the center of the screen
 * 
 * @param state the current state of the game
 * @param type the type of portal to spawn (END GAME OR BOSS ROOM)
*/
void spawn_portal(state_t *state, portal_type_t type) {
    portal_t *portal = portal_init((vector_t){MAX.x / 2, MAX.y / 2}, type);
    body_t *portal_body = portal_get_hitbox(portal);
    body_set_info(portal_body, portal);

    asset_t *portal_image = NULL;
    switch (type) {
        case PORTAL_BOSS: {
            portal_image = 
            asset_make_image_with_body(PORTAL_PATH_BOSS, 
            sdl_get_bounding_box(portal_body), 
                                       portal_body);
            break;
        }
        case PORTAL_END: {
            portal_image = 
            asset_make_image_with_body(PORTAL_PATH_END, sdl_get_bounding_box(portal_body), 
                                       portal_body);
            break;
        }
    }
    list_add(state->body_assets, portal_image);

    state->portal = portal;

    body_t *player_body = player_get_hitbox(state->player);
    create_collision(state->scene, player_body, portal_body, portal_handler, NULL, 1.0);
}

/**
 * Renders the health bars, exp bars, level bars, and projectile load bars on screen. 
 * 
 * @param state the current state of the game
*/
void render_bars(state_t *state) {
    size_t current_health = player_get_health(state->player);
    size_t current_exp = player_get_exp(state->player);
    size_t current_level_scale = player_get_level_scale(state->player);

    sdl_draw_bar(current_health, PLAYER_MAX_HEALTH, HP_BAR, GREEN, RED);
    sdl_draw_bar(current_exp, current_level_scale, XP_BAR, PURPLE, GREY);

    char health_text[BAR_LIST_SIZE_INIT];
    sprintf(health_text, HP_BAR_TEXT, current_health);
    SDL_Rect health_text_rect = {
        .x = HP_BAR.x + BAR_MARGIN, 
        .y = HP_BAR.y + BAR_MARGIN / 2, // 2 to center vertically
        .w = HP_BAR.w - 2 * BAR_MARGIN, 
        .h = TTF_FontHeight(state->font) - (2 * BAR_MARGIN)
    };
    sdl_render_text_color(health_text, state->font, health_text_rect, WHTE);

    // text for XP bar
    char exp_text[BAR_LIST_SIZE_INIT];
    sprintf(exp_text, XP_BAR_TEXT, current_exp, current_level_scale);
    SDL_Rect exp_text_rect = {
        .x = XP_BAR.x + BAR_MARGIN, 
        .y = XP_BAR.y + BAR_MARGIN / 2, // 2 to center vertically
        .w = XP_BAR.w - 2 * BAR_MARGIN, 
        .h = TTF_FontHeight(state->font) - (2 * BAR_MARGIN)
    };
    sdl_render_text_color(exp_text, state->font, exp_text_rect, WHTE);

    // Calculate the bullets left and whether the player is in cooldown
    size_t bullets_left = 
    PLAYER_MAX_BULLETS - (state->bullets_fired % (size_t)PLAYER_MAX_BULLETS);
    sdl_draw_bar(bullets_left, PLAYER_MAX_BULLETS, BULLETS_BAR, BLUE, GREY);

    char bullets_text[BAR_LIST_SIZE_INIT];
    if (state->bullets_fired < (size_t)PLAYER_MAX_BULLETS && 
        state->time_since_cooldown_start >= PLAYER_BULLET_COOLDOWN) {
        sprintf(bullets_text, BULLETS_BAR_TEXT, bullets_left, (size_t)PLAYER_MAX_BULLETS);

    } else {
        size_t remaining_cooldown = 
        (size_t)ceil(PLAYER_BULLET_COOLDOWN - state->time_since_cooldown_start);
        sprintf(bullets_text, RELOAD_BAR_TEXT, remaining_cooldown);
    }
    SDL_Rect bullets_text_rect = {
        .x = BULLETS_BAR.x + BAR_MARGIN, 
        .y = BULLETS_BAR.y + BAR_MARGIN / 2, //2 to center verticalluy
        .w = BULLETS_BAR.w - 2 * BAR_MARGIN, 
        .h = TTF_FontHeight(state->font) - (2 * BAR_MARGIN)
    };
    sdl_render_text_color(bullets_text, state->font, bullets_text_rect, WHTE);
}


/**
 * Renders the border of our interface
 * 
 * @param state the current state of the game
*/
void render_interface_border(state_t *state) {
    SDL_Rect interface_box = {.x = MIN.x, .y = MIN.y, .w = MAX.x, .h = MAX.y};
    asset_t *interface_image = asset_make_image(INTERFACE_PATH, interface_box);
    list_add(state->body_assets, interface_image);
    asset_render(interface_image);
}

/**
 * Clears all the enemies and proejctiles currently on the screen 
 * (marks them for removal)
 * 
 * @param state the current state of the game
*/
void clear_screen(state_t *state) {
    for (size_t i = 0; i < list_size(state->enemies); ++i) {
        enemy_t *current_enemy = list_get(state->enemies, i);
        body_t *current_enemy_body = enemy_get_hitbox(current_enemy);
        body_remove(current_enemy_body);
    }

    for (size_t i = 0; i < list_size(state->projectiles); ++i) {
        projectile_t *current_projectile = list_get(state->projectiles, i);
        body_t *current_projectile_body = projectile_get_hitbox(current_projectile);
        body_remove(current_projectile_body);
    }
}

/**
 * Cleans up and frees all the enemies marked for removal on the screen
 * 
 * @param state the current state of the game
*/
void clear_enemies(state_t *state) {
    for (size_t i = 0; i < list_size(state->enemies); i++) {
        enemy_t *enemy = list_get(state->enemies, i);
        if (body_is_removed(enemy_get_hitbox(enemy))) {
            enemy_free(list_remove(state->enemies, i));
            i -= 1;

            scene_type_t current_scene_type = scene_get_type(state->scene);
            if (current_scene_type != SCENE_GAME_OVER_WIN && 
                current_scene_type != SCENE_GAME_OVER_LOSS && !state->portal_spawned) {    
                state->enemies_killed++;

                player_gain_exp(state->player);
                player_lvl_up(state->player);

                if (state->enemies_killed >= SPAWN_THRESHOLD && 
                    current_scene_type != SCENE_BOSS) {
                    spawn_portal(state, PORTAL_BOSS);
                    state->portal_spawned = true;
                }
            }
        }
    }
}


/**
 * Cleans up and frees all the projectiles marked for removal on the screen
 * 
 * @param state the current state of the game
*/
void clear_projectiles(state_t *state) {
    for (size_t i = 0; i < list_size(state->projectiles); i++) {
        projectile_t *projectile = list_get(state->projectiles, i);
        if (body_is_removed(projectile_get_hitbox(projectile))) {
            projectile_free(list_remove(state->projectiles, i));
            i -= 1;
        }
    }
}

/**
 * Renders all the assets on screen
 * 
 * @param state the current state of the game
*/
void render_assets(state_t *state) {
    for (size_t i = 0; i < list_size(state->body_assets); i++) {
        asset_t *asset = list_get(state->body_assets, i);
        bool render = true;
        if (asset_get_type(asset) == ASSET_IMAGE) {
            body_t *body = asset_get_body(asset);
            
            if (body && body_is_removed(body)) {
                render = false;
                asset_destroy(list_remove(state->body_assets, i));
                i -= 1;
            }
        }

        if (render) {
            asset_render(asset);
        }
    }
}

/**
 * Starts the game
 * 
 * @param state the current state of the game
*/
void play(state_t *state) {
    scene_set_type(state->scene, SCENE_GAME);
}

/**
 * Restarts the game
 * 
 * @param state the current state of the game
*/
void restart(state_t *state) {
    scene_set_type(state->scene, SCENE_MENU);
}

// Image and location mapping for the buttons
button_info_t button_templates[] = {
    {.image_path = "assets/playbutton.png",
     .image_box = PLAYBUTTON_SIZE,
     .handler = (void *)play},
    {.image_path = "assets/restartbutton.png",
     .image_box = RESTARTBUTTON_SIZE,
     .handler = (void *)restart},
};

/**
 * Using `info`, initializes a button in the scene.
 *
 * @param info the button info struct used to initialize the button
 */
asset_t *create_button_from_info(state_t *state, button_info_t info) {
  asset_t *current_image = NULL;

  if (info.image_path != NULL) {
    current_image = asset_make_image(info.image_path, info.image_box);
  }

  asset_t *current_button = asset_make_button(info.image_box, current_image,
                                              NULL, info.handler);
  asset_cache_register_button(current_button);

  return current_button;
}

/**
 * Initializes and stores the button assets in the state.
 * 
 * @param state the current state of the game
 */
void create_buttons(state_t *state) {
  for (size_t i = 0; i < NUM_BUTTONS; i++) {
    button_info_t info = button_templates[i];
    asset_t *button = create_button_from_info(state, info);
    if (strcmp(info.image_path, "assets/playbutton.png") == 0) {
        state->play_button = button;
    } else {
        state->restart_button = button;
    }
  }
}

state_t *emscripten_init() {
    // Initialize the asset cache and SDL
    asset_cache_init();
    sdl_init(MIN, MAX);

    // Allocate memory for the state
    state_t *state = malloc(sizeof(state_t));
    assert(state);

    // Initialize the scene and body assets
    state->scene = scene_init();
    scene_set_type(state->scene, SCENE_MENU);
    state->body_assets = list_init(BODY_ASSETS, (free_func_t) asset_destroy);
    state->font = TTF_OpenFont(FONT_PATH, TEXT_SIZE);

    // Start the spawn and attack timers/counters at 0
    state->time_since_spawn = 0.0;
    state->time_since_atk = 0.0;
    state->time_since_boss_ring = 0.0;
    state->time_since_boss_ray = 0.0;
    state->bullets_fired = 0.0;
    state->time_since_cooldown_start = PLAYER_BULLET_COOLDOWN;
    state->enemies_killed = 0;
    state->boss_spawned = false;
    state->portal_spawned = false;
    state->game_over = false;
    state->boss = NULL;

    // Create the buttons for the menu and the end screen.
    asset_cache_init();
    create_buttons(state);

    // Initialize the list of projectiles and enemies
    state->projectiles = list_init(10, (free_func_t) projectile_free);
    state->enemies = list_init(10, (free_func_t) enemy_free);

    // Initialize the player and add it to the scene
    state->player = player_init();
    body_t *player_body = player_get_hitbox(state->player);
    body_set_centroid(player_body, RESET_POS);
    body_set_info(player_body, state->player); // Store the player_t pointer in info field
    scene_add_body(state->scene, player_body);

    // Create the images for all necessary backgrounds and add it to the body assets
    SDL_Rect background_box = {.x = MIN.x, .y = MIN.y, .w = MAX.x, .h = MAX.y};

    asset_t *startscreen_image = asset_make_image(STARTSCREEN_PATH, background_box);
    state->start_screen = startscreen_image;

    asset_t *overworld_image = asset_make_image(OVERWORLD_PATH, background_box);
    state->overworld_image = overworld_image;
    
    asset_t *boss_background_image = asset_make_image(BOSS_BACKGROUND_PATH, 
                                                      background_box);
    state->boss_background_image = boss_background_image;

    asset_t *death_overlay_image = asset_make_image(DEATHSCREEN_PATH, background_box);
    state->lose_screen = death_overlay_image;

    asset_t *win_overlay_image = asset_make_image(WINSCREEN_PATH, background_box);
    state->win_screen = win_overlay_image;

    // Create the player image and add it to the body assets
    asset_t *player_image = 
    asset_make_image_with_body(PLAYER_PATH, sdl_get_bounding_box(player_body), 
                               player_body);
    list_add(state->body_assets, player_image);

    // Set the key and mouse handlers
    sdl_on_key((key_handler_t)on_key);
    sdl_on_mouse((mouse_handler_t)on_mouse);

    return state;
}

bool emscripten_main(state_t *state) {
    switch (scene_get_type(state->scene)) {
        case SCENE_MENU: {
            sdl_clear();
            double dt = time_since_last_tick();

            asset_render(state->start_screen);
            asset_render(state->play_button);

            sdl_show();
            scene_tick(state->scene, dt);
            return false;
        }
        case SCENE_GAME: {
            double dt = time_since_last_tick();

            if (player_get_health(state->player) <= 0) {
                clear_screen(state);
                state->game_over = true;
                scene_set_type(state->scene, SCENE_GAME_OVER_LOSS);
                return false;
            }
            else {
                if (state->enemies_killed < SPAWN_THRESHOLD) {
                    state->time_since_spawn += dt;
                    state->time_since_atk += dt;
                    state->time_since_boss_ring += dt;
                    state->time_since_boss_ray += dt;
                    state->time_since_cooldown_start += dt;

                    for (size_t i = 0; i < list_size(state->enemies); i++) {
                        enemy_t *enemy = list_get(state->enemies, i);
                        enemy_move_towards_player(enemy, player_get_hitbox(state->player), 
                                                  state->projectiles, ENEMY_STOP_RADIUS, 
                                                  ENEMY_SPEED, ENEMY_DODGE_SPEED, 
                                                  ENEMY_DODGE_RADIUS);
                    }

                    if (state->time_since_spawn >= ENEMY_SPAWN_TIME) {
                        spawn_enemy(state, 10);
                        state->time_since_spawn = 0.0;
                    }

                    if (state->time_since_atk >= ENEMY_ATTACK_TIME) {
                        for (size_t i = 0; i < list_size(state->enemies); i++) {
                            render_enemy_attack(state, list_get(state->enemies, i));
                        }
                        state->time_since_atk = 0.0;
                    }

                    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
                        handle_edges(scene_get_body(state->scene, i), state);
                    }
                } 
                else {
                    if (!portal_get_status(state->portal)) {
                        asset_destroy(state->overworld_image);
                        scene_set_type(state->scene, SCENE_BOSS);
                        state->portal_spawned = false;
                    }
                }
            }
            
            scene_forces(state->scene);

            if (state->portal_spawned) {
                clear_screen(state);
            }

            sdl_clear();

            if (!state->game_over) {
                if (state->enemies_killed < SPAWN_THRESHOLD || state->portal_spawned) {
                    asset_render(state->overworld_image);
                }
            }

            // Clean up super-class objects for projectiles and enemies
            clear_projectiles(state);
            
            clear_enemies(state);

            render_assets(state);

            if (!state->game_over) {
                render_bars(state);
                render_interface_border(state);
            }

            sdl_show();
            scene_tick(state->scene, dt);
            
            return false;
        }
        case SCENE_BOSS: {
            double dt = time_since_last_tick();
            if (!state->boss_spawned && state->enemies_killed >= SPAWN_THRESHOLD) {
                body_t *player_body = player_get_hitbox(state->player);
                body_set_centroid(player_body, RESET_POS);
                spawn_boss(state);
                state->boss_spawned = true;
            }
            
            if (player_get_health(state->player) <= 0) {
                asset_destroy(state->boss_background_image);
                body_t *player_body = player_get_hitbox(state->player);
                body_remove(player_body);
                body_t *boss_body = boss_get_hitbox(state->boss);
                body_remove(boss_body);

                state->game_over = true;
            
                scene_set_type(state->scene, SCENE_GAME_OVER_LOSS);
                return false;
            }
            else if (boss_get_health(state->boss) <= 0) {
                if (!state->portal_spawned) {
                    body_t *boss_body = boss_get_hitbox(state->boss);
                    body_remove(boss_body);
                    spawn_portal(state, PORTAL_END);
                    state->portal_spawned = true;
                }
                if (!portal_get_status(state->portal)) {
                    asset_destroy(state->boss_background_image);
                    body_t *player_body = player_get_hitbox(state->player);
                    body_remove(player_body);

                    state->game_over = true;
                    state->portal_spawned = false;

                    scene_set_type(state->scene, SCENE_GAME_OVER_WIN);
                    return false;
                }
            }
            else {
                state->time_since_spawn += dt;
                state->time_since_atk += dt;
                state->time_since_boss_ring += dt;
                state->time_since_boss_ray += dt;
                state->time_since_cooldown_start += dt;

                for (size_t i = 0; i < list_size(state->enemies); i++) {
                    enemy_t *enemy = list_get(state->enemies, i);
                    enemy_move_towards_player(enemy, player_get_hitbox(state->player), 
                                              state->projectiles, ENEMY_STOP_RADIUS, 
                                              ENEMY_SPEED, ENEMY_DODGE_SPEED, 
                                              ENEMY_DODGE_RADIUS);
                }

                if (state->time_since_spawn >= ENEMY_SPAWN_TIME) {
                    spawn_enemy(state, 10);
                    state->time_since_spawn = 0.0;
                }

                if (state->time_since_atk >= ENEMY_ATTACK_TIME) {
                    for (size_t i = 0; i < list_size(state->enemies); i++) {
                        render_enemy_attack(state, list_get(state->enemies, i));
                    }
                    state->time_since_atk = 0.0;
                }

                if (state->boss_spawned) {
                    if (state->time_since_boss_ring >= BOSS_RING_TIME) {
                        render_boss_ring_attack(state);
                        state->time_since_boss_ring = 0.0;
                    }

                    if (state->time_since_boss_ray >= BOSS_RAY_TIME) {
                        render_boss_ray_attack(state);
                        state->time_since_boss_ray = 0.0;
                    }
                }

                for (size_t i = 0; i < scene_bodies(state->scene); i++) {
                    handle_edges(scene_get_body(state->scene, i), state);
                }
            }

            scene_forces(state->scene);

            if (state->portal_spawned) {
                clear_screen(state);
            }

            sdl_clear();


            if (!state->game_over || state->portal_spawned) {
                asset_render(state->boss_background_image);

                if (state->boss_spawned && boss_get_health(state->boss)) {
                    sdl_draw_bar(boss_get_health(state->boss), MAX_BOSS_HEALTH, BOSS_BAR, 
                                GREEN, RED);
                }   
            }

            // Clean up super-class objects for projectiles and enemies
            clear_projectiles(state);
            clear_enemies(state);
            
            render_assets(state);

            if (!state->game_over) {
                render_bars(state);
                render_interface_border(state);
            }

            sdl_show();
            scene_tick(state->scene, dt);

            return false;
        }
        case SCENE_GAME_OVER_LOSS: {
            sdl_clear();
            asset_render(state->lose_screen);
            sdl_show();
            return false;
        }
        case SCENE_GAME_OVER_WIN: {
            sdl_clear();
            asset_render(state->win_screen);
            sdl_show();
            return false;
        }
    }
}

void emscripten_free(state_t *state) {
    asset_cache_destroy();
    list_free(state->body_assets);
    scene_free(state->scene);
    player_free(state->player); 
    boss_free(state->boss);
    portal_free(state->portal);

    list_free_with_function(state->projectiles, (free_func_t)projectile_free);
    list_free_with_function(state->enemies, (free_func_t)enemy_free); 

    clear_screen(state);
    clear_enemies(state);
    clear_projectiles(state);

    asset_destroy(state->start_screen);
    asset_destroy(state->lose_screen);
    asset_destroy(state->win_screen);
    asset_destroy(state->play_button);
    asset_destroy(state->restart_button);
    asset_destroy(state->overworld_image);
    asset_destroy(state->boss_background_image);

    free(state);

    sdl_stop_music();
}

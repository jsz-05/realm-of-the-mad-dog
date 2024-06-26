#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "player.h"

const size_t PLAYER_HEIGHT = 35; 
const size_t PLAYER_WIDTH = 35; 
const rgb_color_t PLAYER_COLOR = (rgb_color_t){0.2, 0.2, 0.3}; 
const size_t PLAYER_HEALTH = 100;
const size_t PLAYER_DAMAGE = 10;
const size_t DAMAGE_SCALING_FACTOR = 10;
const size_t PLAYER_EXP = 0;
const size_t PLAYER_LVL = 1;
const vector_t PLAYER_PROJ_SIZE = (vector_t) {22, 10};
const rgb_color_t PLAYER_PROJ_COLOR = (rgb_color_t){0.1, 0.9, 0.2};
const size_t PLAYER_PROJ_SPEED = 200;
const size_t EXP_PER_ENEMY = 50;

const size_t LVL_SCALE_START = 100; // Meaning you need 100 x LVL exp to level up
const size_t LVL_SCALE_INCREASE = 50;

struct player {
  body_t *hitbox;
  size_t health;
  size_t damage;
  size_t experience;
  size_t level;
  size_t level_scale;
};

const vector_t PLAYER_START_POS = {500, 30};

player_t *player_init() {
  player_t *player = malloc(sizeof(player_t));
  assert(player);

  player->health = PLAYER_HEALTH;
  player->damage = PLAYER_DAMAGE;
  player->experience = PLAYER_EXP;
  player->level = PLAYER_LVL;
  player->hitbox = make_hitbox(
    PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_START_POS, PLAYER_COLOR);
  player->level_scale = LVL_SCALE_START;

  return player;
}

body_t *player_get_hitbox(player_t *player) {
  return player->hitbox;
}

size_t player_get_damage(player_t *player) {
  return player->damage;
}

size_t player_get_health(player_t *player) {
  return player->health;
}

size_t player_get_exp(player_t *player) {
  return player->experience;
}

size_t player_get_level(player_t *player) {
  return player->level;
}

size_t player_get_level_scale(player_t *player) {
  return player->level_scale;
}

void player_gain_exp(player_t *player) {
  player->experience += EXP_PER_ENEMY;
}

void player_set_health(player_t *player, size_t health) {
  player->health = health;
}


void player_lvl_up(player_t *player) {
  if (player->experience >= player->level_scale) {
    player->experience -= player->level_scale;
    player->level++;
    player->level_scale += LVL_SCALE_INCREASE;

    printf("You have leveled up! You are now level %zu.\n", player->level);

    player->health = PLAYER_HEALTH;
    player->damage += DAMAGE_SCALING_FACTOR;
  }
}

projectile_t *player_ranged_attack(player_t *player, vector_t mouse_loc) {
  size_t damage = player_get_damage(player);
  vector_t start_loc = body_get_centroid(player_get_hitbox(player));

  vector_t traj = vec_subtract(mouse_loc, start_loc);
  double angle = atan2(traj.y, traj.x);

  double angle_deg = (angle) * 180 / M_PI;

  projectile_t *projectile = projectile_init(damage, PLAYER_PROJ_SIZE.x, 
    PLAYER_PROJ_SIZE.y, start_loc, PLAYER_PROJ_COLOR, PROJECTILE_PLAYER, angle_deg);

  body_t *projectile_body = projectile_get_hitbox(projectile);

  double v_x = PLAYER_PROJ_SPEED * cos(angle);
  double v_y = PLAYER_PROJ_SPEED * sin(angle);  

  body_set_velocity(projectile_body, (vector_t) {v_x, v_y});

  return projectile;
}

void player_free(player_t *player) {
  // Hitbox is freed in emscripten main. 
  free(player);
}





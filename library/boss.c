#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "boss.h"

const size_t BOSS_NUM_PROJS = 12;
const size_t BOSS_DAMAGE = 30;
const size_t INIT_BOSS_HEALTH = 5000;
const vector_t BOSS_SIZE = (vector_t) {70, 70};
const vector_t BOSS_PROJ_SIZE = (vector_t) {20, 20};
const rgb_color_t BOSS_PROJ_COLOR = (rgb_color_t){0.1, 0.9, 0.2};
const size_t BOSS_PROJ_SPEED = 100;
const size_t RAY_LOW_BOUND = 30;

struct boss {
  enemy_t *base;
  size_t health;
};

boss_t *boss_init(vector_t start_loc) {
  boss_t *boss = malloc(sizeof(boss_t));
  assert(boss);

  boss->base = enemy_init(BOSS_DAMAGE, start_loc, BOSS_SIZE.x, BOSS_SIZE.y);
  
  boss->health = INIT_BOSS_HEALTH;

  return boss;
}

void boss_free(boss_t *boss) {
  // Enemy and enemy's hitbox is freed in enscripten main. 
  free(boss);
}

body_t *boss_get_hitbox(boss_t *boss) {
  return enemy_get_hitbox(boss->base);
}

size_t boss_get_damage(boss_t *boss) {
  return enemy_get_damage(boss->base);
}

size_t boss_get_health(boss_t *boss) {
  return boss->health;
}

void boss_set_health(boss_t *boss, size_t new_health) {
  boss->health = new_health;
}

list_t *boss_ring_move(boss_t *boss) {
  list_t *projectiles = list_init(BOSS_NUM_PROJS, (free_func_t) projectile_free);
  vector_t start_loc = body_get_centroid(boss_get_hitbox(boss));

  for (size_t i = 0; i < BOSS_NUM_PROJS; i++) {
    double angle = 2 * M_PI * i / BOSS_NUM_PROJS;

    projectile_t *projectile = 
    projectile_init(BOSS_DAMAGE, BOSS_PROJ_SIZE.x, BOSS_PROJ_SIZE.y, start_loc, 
                    BOSS_PROJ_COLOR, PROJECTILE_MOB, angle);
    body_t *projectile_body = projectile_get_hitbox(projectile);

    double v_x = BOSS_PROJ_SPEED * cos(angle);
    double v_y = BOSS_PROJ_SPEED * sin(angle);

    body_set_velocity(projectile_body, (vector_t) {v_x, v_y});
    list_add(projectiles, projectile);
  }

  return projectiles;
}

list_t *boss_ray_move(boss_t *boss, vector_t player_loc) {
  list_t *projectiles = list_init(BOSS_NUM_PROJS, (free_func_t) projectile_free);

  vector_t start_loc = body_get_centroid(boss_get_hitbox(boss));
  vector_t traj = vec_subtract(player_loc, start_loc);
  double angle = atan2(traj.y, traj.x);

  for (size_t i = 0; i < BOSS_NUM_PROJS; i++) {
    projectile_t *projectile = projectile_init(BOSS_DAMAGE, BOSS_PROJ_SIZE.x, 
                                               BOSS_PROJ_SIZE.y, start_loc, 
                                               BOSS_PROJ_COLOR, PROJECTILE_MOB, angle);
    body_t *projectile_body = projectile_get_hitbox(projectile);

    double v_x = 
    (BOSS_PROJ_SPEED * ((double) (i + 1) / BOSS_NUM_PROJS) + RAY_LOW_BOUND) * cos(angle);
    double v_y = 
    (BOSS_PROJ_SPEED * ((double) (i + 1) / BOSS_NUM_PROJS) + RAY_LOW_BOUND) * sin(angle);  

    body_set_velocity(projectile_body, (vector_t) {v_x, v_y});
    list_add(projectiles, projectile);
  }

  return projectiles;
}

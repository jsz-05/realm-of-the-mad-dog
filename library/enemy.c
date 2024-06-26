#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "enemy.h"

const rgb_color_t ENEMY_COLOR = (rgb_color_t){0.2, 0.2, 0.3};
const vector_t ENEMY_SIZE = (vector_t) {35, 35};
const vector_t ENEMY_PROJ_SIZE = (vector_t) {20, 10};
const rgb_color_t ENEMY_PROJ_COLOR = (rgb_color_t){0.1, 0.9, 0.2};
const size_t ENEMY_PROJ_SPEED = 200;

struct enemy {
  body_t *hitbox;
  size_t damage;
};

enemy_t *enemy_init(size_t damage, vector_t start_pos, double w, double h) {
  enemy_t *enemy = malloc(sizeof(enemy_t));
  assert(enemy);

  enemy->damage = damage;

  // If no specified width or height, use default sizes
  if (w != 0 && h != 0) {
    enemy->hitbox = make_hitbox(w, h, start_pos, ENEMY_COLOR);
  } else {
    enemy->hitbox = make_hitbox(ENEMY_SIZE.x, ENEMY_SIZE.y, start_pos, ENEMY_COLOR);
  }
  

  return enemy;
}

void enemy_free(enemy_t *enemy) {
  // Hitbox is freed in enscripten main. 
  free(enemy);
}

body_t *enemy_get_hitbox(enemy_t *enemy) {
  return enemy->hitbox;
}

size_t enemy_get_damage(enemy_t *enemy) {
  return enemy->damage;
}

projectile_t *enemy_attack(enemy_t *enemy, vector_t player_loc) {
  size_t damage = enemy_get_damage(enemy);
  vector_t start_loc = body_get_centroid(enemy_get_hitbox(enemy));

  vector_t traj = vec_subtract(player_loc, start_loc);
  double angle = atan2(traj.y, traj.x);

  double angle_deg = (angle) * 180 / M_PI;

  projectile_t *projectile = projectile_init(damage, ENEMY_PROJ_SIZE.x, 
    ENEMY_PROJ_SIZE.y, start_loc, ENEMY_PROJ_COLOR, PROJECTILE_MOB, angle_deg);

  body_t *projectile_body = projectile_get_hitbox(projectile);

  double v_x = ENEMY_PROJ_SPEED * cos(angle);
  double v_y = ENEMY_PROJ_SPEED * sin(angle);  

  body_set_velocity(projectile_body, (vector_t) {v_x, v_y});

  return projectile;
}

void enemy_move_towards_player(enemy_t *enemy, body_t *player, list_t *projectiles, 
                               double stop_radius, double speed, double dodge_speed, 
                               double dodge_radius) {
    vector_t enemy_pos = body_get_centroid(enemy_get_hitbox(enemy));
    vector_t player_pos = body_get_centroid(player);

    // Calculate the direction and distance towards the player
    vector_t direction_to_player = vec_subtract(player_pos, enemy_pos);
    double distance_to_player = vec_get_length(direction_to_player);

    // Normalize the direction towards the player
    if (distance_to_player > stop_radius) {
        direction_to_player = vec_multiply(speed / distance_to_player, 
                                           direction_to_player);
    } else {
        direction_to_player = VEC_ZERO;
    }

    vector_t dodge_direction = VEC_ZERO;
    for (size_t i = 0; i < list_size(projectiles); i++) {
        projectile_t *projectile = list_get(projectiles, i);
        if (projectile_get_type(projectile) == PROJECTILE_PLAYER) {
            vector_t projectile_pos = 
            body_get_centroid(projectile_get_hitbox(projectile));
            vector_t to_projectile = vec_subtract(projectile_pos, enemy_pos);
            double distance_to_projectile = vec_get_length(to_projectile);

            // First check if the projectile is within dodge radius
            if (distance_to_projectile < dodge_radius) {
                vector_t projectile_vel = 
                body_get_velocity(projectile_get_hitbox(projectile));

                // Calculate the perpendicular dodge direction based on projectile's 
                // approach angle
                vector_t to_projectile_normalized = 
                vec_multiply(1 / distance_to_projectile, to_projectile);
                double cross_product = 
                vec_cross(to_projectile_normalized, projectile_vel);

                if (cross_product > 0) {
                    dodge_direction = vec_rotate(to_projectile, -M_PI / 2); // Dodge right
                } else {
                    dodge_direction = vec_rotate(to_projectile, M_PI / 2); // Dodge left
                }
                dodge_direction = 
                vec_multiply(dodge_speed / vec_get_length(dodge_direction), 
                             dodge_direction);
            }
        }
    }

    // Combine dodge and movement velocities
    vector_t combined_direction = vec_add(direction_to_player, dodge_direction);
    body_set_velocity(enemy_get_hitbox(enemy), combined_direction);
}



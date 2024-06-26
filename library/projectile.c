#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "projectile.h"

struct projectile {
    body_t *hitbox;
    size_t damage;
    projectile_type_t type;
    double angle; 
};

projectile_t *projectile_init(size_t damage, size_t w, size_t h, vector_t start_pos, 
                              rgb_color_t color, projectile_type_t type, double angle) {
    projectile_t *projectile = malloc(sizeof(projectile_t));
    assert(projectile);

    projectile->hitbox = make_hitbox(w, h, start_pos, color);
    projectile->damage = damage;
    projectile->type = type;
    projectile->angle = angle; 

    return projectile;
}

void projectile_free(projectile_t *projectile) {
    // Hitbox is freed in emscripten main. 
    free(projectile);
}

void projectile_correct_hitbox(projectile_t *projectile) {
    body_set_rotation(projectile->hitbox, -projectile->angle);
}

body_t *projectile_get_hitbox(projectile_t *projectile) {
  return projectile->hitbox;
}

size_t projectile_get_damage(projectile_t *projectile) {
  return projectile->damage;
}

projectile_type_t projectile_get_type(projectile_t *projectile) {
    return projectile->type;
}

double projectile_get_angle(projectile_t *projectile) {
    return projectile->angle;
}

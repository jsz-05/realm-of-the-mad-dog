#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"

struct body {
  polygon_t *poly;

  double mass;

  vector_t force;
  vector_t impulse;
  bool removed;

  void *info;
  free_func_t info_freer;
};

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *body = malloc(sizeof(body_t));
  assert(body);

  body->mass = mass;
  body->poly = polygon_init(shape, VEC_ZERO, 0, color.r, color.g, color.b);
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->removed = false;
  body->info = info;
  body->info_freer = info_freer;

  return body;
}

void body_reset(body_t *body) {
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

polygon_t *body_get_polygon(body_t *body) { return body->poly; }

void *body_get_info(body_t *body) { return body->info; }

void body_set_info(body_t *body, void *info) {
    body->info = info;
}


void body_free(body_t *body) {
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  polygon_free(body->poly);
  free(body);
}

list_t *body_get_shape(body_t *body) {
  list_t *shape = polygon_get_points(body->poly);
  list_t *ret = list_init(list_size(shape), free);

  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t orig_vec = *(vector_t *)list_get(shape, i);

    // Make copy of point vector
    vector_t *list_v = malloc(sizeof(*list_v));
    assert(list_v);
    *list_v = (vector_t){orig_vec.x, orig_vec.y};

    list_add(ret, list_v);
  }

  return ret;
}

vector_t body_get_centroid(body_t *body) {
  return polygon_get_center(body->poly);
}

vector_t body_get_velocity(body_t *body) {
  return *(polygon_get_velocity(body->poly));
}

rgb_color_t *body_get_color(body_t *body) {
  return polygon_get_color(body->poly);
}

void body_set_color(body_t *body, rgb_color_t *col) {
  polygon_set_color(body->poly, col);
}

void body_set_centroid(body_t *body, vector_t x) {
  polygon_set_center(body->poly, x);
}

void body_set_velocity(body_t *body, vector_t v) {
  polygon_set_velocity(body->poly, v);
}

double body_get_rotation(body_t *body) {
  return polygon_get_rotation(body->poly);
}

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->poly, angle, body_get_centroid(body));
}

void body_tick(body_t *body, double dt) {
  vector_t current_velocity = vec_add(
      body_get_velocity(body),
      vec_multiply(1 / body->mass,
                   vec_add(body->impulse, vec_multiply(dt, body->force))));
  body_set_velocity(body, vec_multiply(0.5, vec_add(current_velocity,
                                                    body_get_velocity(body))));
  polygon_move(body->poly, dt);
  body_set_velocity(body, current_velocity);

  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

double body_get_mass(body_t *body) { return body->mass; }

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_remove(body_t *body) { body->removed = true; }

bool body_is_removed(body_t *body) { return body->removed; }

body_t *make_hitbox(size_t w, size_t h, vector_t start_pos, rgb_color_t color) {
  list_t *hitbox_points = list_init(4, free);
  vector_t *v1 = malloc(sizeof(vector_t));
  assert(v1);
  *v1 = (vector_t){0, 0};
  list_add(hitbox_points, v1);

  vector_t *v2 = malloc(sizeof(vector_t));
  assert(v2);
  *v2 = (vector_t){w, 0};
  list_add(hitbox_points, v2);

  vector_t *v3 = malloc(sizeof(vector_t));
  assert(v3);
  *v3 = (vector_t){w, h};
  list_add(hitbox_points, v3);

  vector_t *v4 = malloc(sizeof(vector_t));
  assert(v4);
  *v4 = (vector_t){0, h};
  list_add(hitbox_points, v4);
  body_t *hitbox = body_init(hitbox_points, 1, color);
  body_set_centroid(hitbox, start_pos);

  return hitbox;
}

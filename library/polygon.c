#include "polygon.h"
#include "color.h"
#include "list.h"
#include "math.h"
#include <assert.h>
#include <stdlib.h>

typedef struct polygon {
  list_t *points;
  vector_t velocity;
  vector_t centroid;
  double rotation_speed;
  double rotation;
  rgb_color_t *color;
} polygon_t;

polygon_t *polygon_init(list_t *points, vector_t initial_velocity,
                        double rotation_speed, double red, double green,
                        double blue) {
  polygon_t *polygon = malloc(sizeof(polygon_t));
  assert(polygon);
  polygon->points = points;
  polygon->velocity = initial_velocity;
  polygon->centroid = polygon_centroid(polygon);
  polygon->rotation_speed = rotation_speed;
  polygon->rotation = 0;
  polygon->color = color_init(red, green, blue);
  return polygon;
}

list_t *polygon_get_points(polygon_t *polygon) { return polygon->points; }

void polygon_move(polygon_t *polygon, double time_elapsed) {
  vector_t translation = vec_multiply(time_elapsed, polygon->velocity);
  polygon_translate(polygon, translation);
  polygon_rotate(polygon, polygon->rotation_speed * time_elapsed,
                 polygon->centroid);
}

void polygon_set_velocity(polygon_t *polygon, vector_t vel) {
  polygon->velocity = vel;
}

void polygon_free(polygon_t *polygon) {
  list_free(polygon->points);
  color_free(polygon->color);
  free(polygon);
}

vector_t *polygon_get_velocity(polygon_t *polygon) {
  return &(polygon->velocity);
}

double polygon_area(polygon_t *polygon) {
  double area = 0.0;
  size_t num_points = list_size(polygon->points);

  for (size_t i = 0; i < num_points; ++i) {
    vector_t *current_point = list_get(polygon->points, i);
    vector_t *next_point = list_get(
        polygon->points, (i + 1) % num_points); // Use modulo for wrap-around
    area += vec_cross(*current_point, *next_point);
  }

  return 0.5 * area;
}

vector_t polygon_centroid(polygon_t *polygon) {
  vector_t centroid = VEC_ZERO;
  size_t num_points = list_size(polygon->points);

  for (size_t i = 0; i < num_points; ++i) {
    vector_t *current_point = list_get(polygon->points, i);
    vector_t *next_point = list_get(
        polygon->points, (i + 1) % num_points); // Use modulo for wrap-around
    centroid =
        vec_add(centroid, vec_multiply(vec_cross(*current_point, *next_point),
                                       vec_add(*current_point, *next_point)));
  }

  centroid.x /= 6 * polygon_area(polygon);
  centroid.y /= 6 * polygon_area(polygon);

  return centroid;
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
  size_t num_points = list_size(polygon->points);
  for (size_t i = 0; i < num_points; ++i) {
    vector_t *current_point = list_get(polygon->points, i);
    *current_point = vec_add(*current_point, translation);
  }
  polygon->centroid = vec_add(polygon->centroid, translation);
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
  polygon_translate(polygon, vec_multiply(-1, point));
  size_t num_points = list_size(polygon->points);
  for (size_t i = 0; i < num_points; ++i) {
    vector_t *current_point = list_get(polygon->points, i);
    *current_point = vec_rotate(*current_point, angle);
  }
  polygon_translate(polygon, point);
  polygon->rotation += angle;
}

rgb_color_t *polygon_get_color(polygon_t *polygon) { return polygon->color; }

void polygon_set_color(polygon_t *polygon, rgb_color_t *color) {
  color_free(polygon->color);
  polygon->color = color_init(color->r, color->g, color->b);
}

void polygon_set_center(polygon_t *polygon, vector_t centroid) {
  vector_t translation = vec_subtract(centroid, polygon->centroid);
  polygon_translate(polygon, translation);
}

vector_t polygon_get_center(polygon_t *polygon) {
  return polygon_centroid(polygon);
}

void polygon_set_rotation(polygon_t *polygon, double rot) {
  polygon_rotate(polygon, rot - polygon->rotation, polygon->centroid);
}

double polygon_get_rotation(polygon_t *polygon) { return polygon->rotation; }

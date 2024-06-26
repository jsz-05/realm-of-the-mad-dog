/**
 * Modified vector.c from Project 01
 */

#include "vector.h"
#include "math.h"

const vector_t VEC_ZERO = {0.0, 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t resultant_vector = {v1.x, v1.y};

  resultant_vector.x += v2.x;
  resultant_vector.y += v2.y;

  return resultant_vector;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  v2 = vec_negate(v2);
  v1 = vec_add(v1, v2);

  return v1;
}

vector_t vec_negate(vector_t v) {
  v.x *= -1;
  v.y *= -1;

  return v;
}

vector_t vec_multiply(double scalar, vector_t v) {
  v.x *= scalar;
  v.y *= scalar;

  return v;
}

double vec_dot(vector_t v1, vector_t v2) {
  // Using definition of dot product
  return v1.x * v2.x + v1.y * v2.y;
}

double vec_cross(vector_t v1, vector_t v2) {
  // Using definition of cross product
  return v1.x * v2.y - v1.y * v2.x;
}

double vec_get_length(vector_t v) { return sqrt(pow(v.x, 2) + pow(v.y, 2)); }

vector_t vec_rotate(vector_t v, double angle) {
  vector_t resultant_vector = VEC_ZERO;

  resultant_vector.x = (v.x * cos(angle)) - (v.y * sin(angle));
  resultant_vector.y = (v.x * sin(angle)) + (v.y * cos(angle));

  return resultant_vector;
}

double vec_angle_between(vector_t v1, vector_t v2) {
    double dot = vec_dot(v1, v2);
    double len1 = vec_get_length(v1);
    double len2 = vec_get_length(v2);

    // Error checking
    if (len1 == 0 || len2 == 0) {
        return 0;
    }

    double cos_theta = dot / (len1 * len2);
    cos_theta = fmax(fmin(cos_theta, 1), -1);

    // Return the angle in radians
    return acos(cos_theta);
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "forces.h"
#include "scene.h"

typedef struct scene {
  scene_type_t type;
  size_t num_bodies;
  list_t *bodies;
  list_t *force_creator_list;
} scene_t;

const size_t INIT_SIZE = 10;
const size_t MAX_FORCES = 3;

void scene_forces(scene_t *scene) {
  // Call all the force creators in the scene
  for (size_t i = 0; i < list_size(scene->force_creator_list); i++) {
    force_activator_t *force_activator = list_get(scene->force_creator_list, i);
    force_activator->forcer(force_activator->aux);
  }
}

void scene_tick(scene_t *scene, double dt) {
  for (ssize_t i = scene->num_bodies - 1; i >= 0; i--) {
    body_t *body = scene_get_body(scene, i);
    if (body_is_removed(body)) {

      // Iterate through the scene's force creators
      for (ssize_t j = list_size(scene->force_creator_list) - 1; j >= 0; j--) {
        force_activator_t *force_activator =
            list_get(scene->force_creator_list, j);

        // Iterate through the force creator's list of bodies
        for (ssize_t k = list_size(force_activator->bodies) - 1; k >= 0; k--) {
          body_t *force_body = list_get(force_activator->bodies, k);

          if (body == force_body) {
            force_act_free(list_remove(scene->force_creator_list, j));
            break;
          }
        }
      }
      body_free(list_remove(scene->bodies, i));
      scene->num_bodies--;
    } else {
      body_tick(body, dt);
    }
  }
}

void scene_add_force_creator(scene_t *scene, force_creator_t force_creator,
                             void *aux) {
  scene_add_bodies_force_creator(scene, force_creator, aux, list_init(0, free));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies) {
  force_activator_t *new_force_activator = force_act_init(forcer, aux, bodies);
  list_add(scene->force_creator_list, new_force_activator);
}

scene_type_t scene_get_type(scene_t *current_scene) {
    return current_scene->type;
}

void scene_set_type(scene_t *current_scene, scene_type_t new_scene_type) {
    current_scene->type = new_scene_type;
}

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene);

  scene->type = SCENE_GAME;
  scene->num_bodies = 0;
  scene->bodies = list_init(INIT_SIZE, (free_func_t)body_free);
  scene->force_creator_list =
      list_init(MAX_FORCES, (free_func_t)force_act_free);

  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->force_creator_list);
  list_free(scene->bodies);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return scene->num_bodies; }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
  scene->num_bodies++;
}

// Depreceated body removal function
void scene_remove_body(scene_t *scene, size_t index) {
  body_t *body = scene_get_body(scene, index);
  body_remove(body);
}

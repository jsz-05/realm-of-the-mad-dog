#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "portal.h"

const rgb_color_t PORTAL_COLOR = (rgb_color_t){0.0, 0.0, 0.0};
const vector_t PORTAL_SIZE = (vector_t) {50, 50};

struct portal {
    body_t *hitbox;
    portal_type_t portal_type;
    bool portal_spawned;
};

portal_t *portal_init(vector_t start_pos, portal_type_t type) {
    portal_t *portal = malloc(sizeof(portal_t));
    assert(portal);

    portal->hitbox = make_hitbox(PORTAL_SIZE.x, PORTAL_SIZE.y, start_pos, PORTAL_COLOR);
    portal->portal_type = type;
    portal->portal_spawned = true;
    
    return portal;
}

body_t *portal_get_hitbox(portal_t *portal) {
    return portal->hitbox;
}

portal_type_t portal_get_type(portal_t *portal) {
    return portal->portal_type;
}

void portal_set_type(portal_t *portal, portal_type_t new_portal_type) {
    portal->portal_type = new_portal_type;
}

bool portal_get_status(portal_t *portal) {
    return portal->portal_spawned;
}

void portal_set_status(portal_t *portal, bool status) {
    portal->portal_spawned = status;
} 

void portal_free(portal_t *portal) {
    // Hitbox is freed in emscripten main. 
    free(portal);
}
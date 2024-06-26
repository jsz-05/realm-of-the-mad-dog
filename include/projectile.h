#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "body.h"

typedef struct projectile projectile_t;

typedef enum {
    PROJECTILE_PLAYER,
    PROJECTILE_MOB
} projectile_type_t;

/**
 * Initializes a projectile based on the given stats and starting dimensions. 
 * 
 * @param damage The damage the projectile deals per hit. 
 * @param w The width of the projectile's body. 
 * @param h The height of the projectile's body. 
 * @param start_pos The starting position of the projectile's body. 
 * @param angle the angle of the projectile's trajectory.
 * @return a pointer to a new projectile
*/
projectile_t *projectile_init(size_t damage, size_t w, size_t h, vector_t start_pos, 
                              rgb_color_t color, projectile_type_t type, double angle);

/**
 * Retrives the hitbox of the projectile at a given time. 
 * 
 * @param projectile a pointer to the projectile returned from projectile_init()
 * @return a pointer to the hitbox of the projectile
*/
body_t *projectile_get_hitbox(projectile_t *projectile);

/**
 * Returns the damage a projectile deals
 * 
 * @param projectile a pointer to the projectile returned from projectile_init()
 * @return the damage dealt upon collision with a given projectile
*/
size_t projectile_get_damage(projectile_t *projectile);

/**
 * Returns the type of a given projectile
 * 
 * @param projectile a pointer to the projectile returned from projectile_init()
 * @return the type of the provided projectile
*/
projectile_type_t projectile_get_type(projectile_t *projectile);

/**
 * Frees a given projectile 
 * 
 * @param projectile a pointer to the projectile returned from projectile_init()
*/
void projectile_free(projectile_t *projectile);

/**
 * Returns the angle of a given projectile
 * 
 * @param projectile a pointer to the projectile returned from projectile_init()
 * @return the angle the projectile is travelling
*/
double projectile_get_angle(projectile_t *projectile);

/**
 * Corrects the hitbox of the player at a given time. 
 * This is necessary because of the way SDL renders rotated images. 
 * We must apply the rotation to the body only AFTER the image has been
 * rendered and rotated. 
 * 
 * @param projectile a pointer to the projectile returned from projectile_init()
*/
void projectile_correct_hitbox(projectile_t *projectile);

#endif // #ifndef __PROJECTILE_H__


#ifndef __BOSS_H__
#define __BOSS_H__

#include "enemy.h"
#include <math.h>

typedef struct boss boss_t;
/**
 * Initializes a boss based on the given stats and starting dimensions. 
 * 
 * @param start_pos The starting position of the boss's body. 
 * @return a pointer to a new boss struct
*/
boss_t *boss_init(vector_t start_pos);

/**
 * Frees a boss. 
 * 
 * @param boss a pointer to the boss returned from boss_init()
*/
void boss_free(boss_t *boss);

/**
 * Retrives the hitbox of the boss at a given time. 
 * 
 * @param boss a pointer to the boss returned from boss_init()
 * @return a pointer to the hitbox of the boss
*/
body_t *boss_get_hitbox(boss_t *boss);

/**
 * Returns a list of projectiles for the boss's ring attack
 * 
 * @param boss a pointer to the boss returned from boss_init()
 * @return a pointer to the list of projectiles released by the boss
*/
list_t *boss_ring_move(boss_t *boss);

/**
 * Returns the boss's current health.
 * 
 * @param boss a pointer to the boss returned from boss_init()
 * @return the boss's health
*/
size_t boss_get_health(boss_t *boss);

/**
 * Sets and returns the boss's resultant health.
 * 
 * @param boss a pointer to the boss returned from boss_init()
 * @param new_health the new amount of health that the boss has
*/
void boss_set_health(boss_t *boss, size_t new_health);

/**
 * Returns a list of projectiles for the boss's ray attack
 * 
 * @param boss a pointer to the boss returned from boss_init()
 * @param player_loc the current location of the player
 * @return a pointer to the list of projectiles released by the boss
*/
list_t *boss_ray_move(boss_t *boss, vector_t player_loc);

#endif // #ifndef __BOSS_H__
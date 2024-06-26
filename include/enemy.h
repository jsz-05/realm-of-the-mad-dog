#ifndef __ENEMY_H__
#define __ENEMY_H__    

#include "body.h"
#include "projectile.h"

typedef struct enemy enemy_t;
/**
 * Initializes a enemy based on the given stats and starting dimensions. 
 * Setting starting dimensions w or h to 0 causes enemy to use the default enemy size. 
 * 
 * @param damage The damage the enemy deals per hit. 
 * @param start_pos The starting position of the enemy's body. 
 * @param w The width of the enemy's body. 
 * @param h The height of the enemy's body. 
 * @return pointer to the new enemy
*/
enemy_t *enemy_init(size_t damage, vector_t start_pos, double w, double h);

/**
 * Retrives the hitbox of the enemy at a given time. 
 * 
 * @param enemy a pointer to the enemy returned from enemy_init()
 * @return pointer to the hitbox of the enemy
*/
body_t *enemy_get_hitbox(enemy_t *enemy);

/**
 * Retrives the damage of the enemy. 
 * 
 * @param enemy a pointer to the enemy returned from enemy_init()
 * @return the damage of the enemy. 
*/
size_t enemy_get_damage(enemy_t *enemy);

/**
 * Frees a given enemy 
 * 
 * @param enemy a pointer to the enemy returned from enemy_init()
*/
void enemy_free(enemy_t *enemy);

/**
 * Creates a projectile aimed towards the player
 * 
 * @param enemy a pointer to the enemy returned from enemy_init()
 * @param player_loc the current location of the player (to aim at)
 * @return a new projectile aimed towards the player, centered at the enemy
*/
projectile_t *enemy_attack(enemy_t *enemy, vector_t player_loc);

/**
 * Moves an enemy towards the player, but stops within a certain radius
 * 
 * @param enemy a pointer to the enemy returned from enemy_init()
 * @param player a pointer to the player's body
 * @param stop_radius the radius within which the enemy should stop moving to the player
 * @param speed the speed at which the enemy should move
 * @param dodge_speed the speed at which the enemy dodges
 * @param dodge_radius the radius at which the enemy dodges
 */
void enemy_move_towards_player(enemy_t *enemy, body_t *player, list_t *projectiles, 
                               double stop_radius, double speed, double dodge_speed, 
                               double dodge_radius);

#endif // #ifndef __ENEMY_H__

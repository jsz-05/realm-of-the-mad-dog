#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "body.h"
#include "vector.h"
#include "projectile.h"
#include "state.h"

typedef struct player player_t;
/**
 * Initializes a player based on the constants in the file. 
 * @return a pointer to a new player
*/
player_t *player_init();

/**
 * Retrives the hitbox of the player at a given time. 
 * 
 * @param player a pointer to the player returned from player_init()
 * @return the hitbox of the player
*/
body_t *player_get_hitbox(player_t *player);

/**
 * Sets the health of a player to a certain value
 * 
 * @param player a pointer to the player returned from player_init()
 * @param health the new desired health value
*/
void player_set_health(player_t *player, size_t health);

/**
 * Gets the health of a player
 * 
 * @param player a pointer to the player returned from player_init()
 * @return the current health of the player
*/
size_t player_get_health(player_t *player);

/**
 * Gets the experience the player has
 * 
 * @param player a pointer to the player returned from player_init()
 * @return the current exp of the player
*/
size_t player_get_exp(player_t *player);

/**
 * Adds to the current quantity of experience the player has
 * 
 * @param player a pointer to the player returned from player_init()
*/
void player_gain_exp(player_t *player);

/**
 * Gets the level of the player.
 * 
 * @param player a pointer to the player returned from player_init()
 * @return the current level of the player
*/
size_t player_get_level(player_t *player);

/**
 * Gets the current level scaling of the player.
 * 
 * @param player a pointer to the player returned from player_init()
 * @return the current level scale of the player
*/
size_t player_get_level_scale(player_t *player);

/**
 * Checks whether or not the player is elligbile and levels them up. 
 * 
 * @param player a pointer to the player returned from player_init()
*/
void player_lvl_up(player_t *player);

/**
 * Sets up the player's projectile attack.
 * 
 * @param player a pointer to the player returned from player_init()
 * @param mouse_loc a vector representing the location of the mouse.
 * @return the projectile that has been released from player.
*/
projectile_t *player_ranged_attack(player_t *player, vector_t mouse_loc);

/**
 * Frees the player.
 * 
 * @param player a pointer to the player returned from player_init()
*/
void player_free(player_t *player);

#endif // #ifndef __PLAYER_H__


#ifndef __PORTAL_H__
#define __PORTAL_H__

#include "body.h"

typedef enum { PORTAL_BOSS, PORTAL_END } portal_type_t;

typedef struct portal portal_t;
/**
 * Creates a portal based on the given parameters.
 * 
 * @param start_pos The starting position of the portal. 
 * @param type The type of the portal.
 * @return pointer to the new portal
*/
portal_t *portal_init(vector_t start_pos, portal_type_t type);

/**
 * Retrives the hitbox of the portal at a given time. 
 * 
 * @param portal a pointer to the portal returned from portal_init()
 * @return pointer to the hitbox of the portal
*/
body_t *portal_get_hitbox(portal_t *portal);

/**
 * Retrives the type of portal. 
 * 
 * @param portal a pointer to the portal returned from portal_init()
 * @return type of the portal
*/
portal_type_t portal_get_type(portal_t *portal);

/**
 * Sets the hitbox of the portal at a given time. 
 * 
 * @param portal a pointer to the portal returned from portal_init()
 * @param new_portal_type new desired portal type for the portal
*/
void portal_set_type(portal_t *portal, portal_type_t new_portal_type);

/**
 * Sets the status of whether the portal is spawned or not. 
 * 
 * @param portal a pointer to the portal returned from portal_init()
*/
void portal_set_status(portal_t *portal, bool status);

/**
 * Gets the status of whether the portal is spawned or not. 
 * 
 * @param portal a pointer to the portal returned from portal_init()
 * @return the status of whether the portal is spawned or not. 
*/
bool portal_get_status(portal_t *portal);

/**
 * Frees a given portal
 * 
 * @param portal a pointer to the portal returned from portal_init()
*/
void portal_free(portal_t *portal);

#endif // #ifndef __PORTAL_H__
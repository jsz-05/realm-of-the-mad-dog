# Realm of the Mad Dog

**CS3 Final Project: Jeffrey Zhou, Ryan Lin, Jacob Chang**

**Concept:** The game we designed and implemented is a roguelike, open-world game. The player starts in a an overworld where they can kill waves of mobs to gain experience and eventually fight the boss in the boss room. 

## Section 1: Gameplay
The game starts off in a lobby, as described in the previous section. The game engine then periodically spawns waves of mobs, likely wolves, and the player will gain experience by killing these mobs (thus raising their statistics, like attack and health, to get stronger). The win condition is if the player clears all the waves, and defeats the boss in the boss room. The loss condition is if the player takes enough damage from the mobs to deplete their health bar. 

The player moves with the WASD keys, and uses the mouse pointer to aim. Right click activates melee (close range) attacks, and left click activates magic (long range) attacks. To spawn the boss room portal, the player must survive various waves in the overworld and gain enough XP by killing the zombie dogs. Be careful, as the zombie dogs are able to dodge your projectiles! The boss is stationary, but possesses a range of very dangerous attacks. Good luck! 

## Section 2: Feature Set
**Level 1 Features:**
- Abstractions for entities in game - Ryan: (Body is superclass of Enemy, Player, Projectiles, and Boss will be a subclass of Enemy, etc.)
- Mouse/Key Handling (Controls) - Jacob: The player will move via arrows and attack via mouse aim/clicks. Jacob will develop the handlers for these things, and the collisions that allow for attacks. Jacob will also call the later-mentioned projectile handling in the case of ranged attacks for the player. 
- Projectile Handling (bullets coming in contact, etc) - Jeff: Players and mobs will be able to shoot projectiles in some cases. This handling and separation will need to be implemented by porting over our existing physics engine and modifying it such that we are able to differentiate between our different types of projectiles. It will be helpful to look at the space_invaders demo from previous projects for differentiation projectiles. 

**Level 2 Features:**
- Enemy AI algorithm - Jeff: Define a movement changing algorithm every tick - the enemies (unless they are bosses) should gravitate towards the player within a certain distance. We can also make some random motion in the enemies to make the game feel more natural. 
- Enemy/Boss Attack Features - Ryan: Each enemy should draw from a certain pool of attack actions, such as firing projectiles or coming into contact with the player. The boss can have multiple attack patterns (for example, a straight beam of projectiles or a ring of projectiles). The enemies should have pre-defined stats (attack, health, etc.). 
- Player Attack Features - Jacob: The player can either perform melee or projectile attacks depending on left or right click or different key presses (handled in Priority 1). It is Jacob's job to implement these potential attack patterns. 

**Level 3 Features:**
- Scene Switching (Lobby, Overworld, Boss Room) - Jacob: The player will be able to rotate between the aforementioned three scenes, so Jacob will handle the switching and the background art for these. Beef up the current "scene" implementation such that it can handle this (i.e. cycle through background images). The player cycles through these scenes by going through "portals" on the map (these will have pre-defined coordinates, and be part of the scene image). 
- Enemy Spawning - Ryan: Enemies should spawn periodically on the edge of the map, trickling in as the game progresses. The boss should spawn after a certain number of enemies are killed by the player. 
- Game Sprites - Jeff: The reason we chose to put this feature at a lower priority is because we believe the core functionality of the game takes precedence. However, adding game sprites for the player and the mobs will give life to the game, and make it visually more enjoyable. 


**Level 4 Features:**
- Experience system / Player stats backend - Jacob: The player struct, which inherits the body struct, will have experience fields as well as statistics. If the experience passes a certain threshold, then the stats increase by a pre-determined amount that scales with respect to the player's current level. The player's stats should reset when they die (health reaches 0). 
- Experience system / Player stats frontend - Jeff: Display the character stats, experience bar, and other relevant information. We can re-use the text render from the previous priority 3 Captions feature. 
- Music - Ryan: A lot of these open-world games have very unique soundtracks. Game experience is not the same without them! Music will lend more to the ambiance of the game, and make the gameplay more enjoyable. 

# SDL_RhythmGame

This is a project made by Anders Hägglund 2022-12-16 for an assignment in data oriented programming.

Game description:   
You play the game by shooting rocks in synch with the music.    
The goal is to keep the rocks from reaching a protecting laser at the bottom of the screen. The width of the laser indicates the remaining health.
If you shoot a rock with incorrect timing, the rock will become indestructible & immovable and will eventually reach the laser.   
There are three different levels (songs) that increases in tempo and difficulty. 

Data oriented description:    
Instead of creating structs/classes of enemies/rocks/lasers etc. I created lists of the following attributes that defines a movable object:      
Positions, velocities, rotations, textures and tags.
So an "object" doesn't really "exist", it's just an index in all those lists. 

When I move the objects, I only use the positions, velocities and tags (an Enum that seperates between immovable objects).     
      - > in engine.cpp -> moveObjects()
      
When I draw the objects, I only use the textures, positions and rotations.      
      - > in engine.cpp -> drawObjects()
I use the same principles many other cases (in engine.cpp).

Performance description:      
To make sure that this system could be used in a larger scale, I implemented insertion sort of the lists. 
I chose insertion sort because of it's speed when it comes to lists that are already sorted (or close to), which would be the case most of the frames.
I sorted every object by it's most left coordinate. When I do my collision checks, I only check to the right of each object and
I stop checking when the other object's most left coordinate is bigger than the original object's most right coordinate...
since this would mean that they couldn't collide, and neither could the following objects in the sorted list.


Notes:     
I didn't have time to polish and structure the code the way I had planned to. I got too ambitious for the deadline and kept adding features until
the last day. But I don't regret anything, because the game is pretty sweet. 


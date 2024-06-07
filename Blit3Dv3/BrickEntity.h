#pragma once

#include "Entity.h"

enum class BrickColour { WHITE, ORANGE, BLUE, GREEN, SILVER, CATCHBRICK, LPBRICK, MBBRICK };

//externed sprites
extern Sprite *whiteBrickSprite;
extern Sprite *blueBrickSprite;
extern Sprite *orangeBrickSprite;
extern Sprite *greenBrickSprite;
extern Sprite* silverBrickSprite;
extern Sprite* catchBrickSprite;
extern Sprite* lpBrickSprite;
extern Sprite* mbBrickSprite;

class BrickEntity : public Entity
{
public:
	BrickColour colour;
	float x=0, y=0;
	BrickEntity()
	{
		typeID = ENTITYBRICK;
		colour = BrickColour::WHITE;
		
	}

	bool HandleCollision();
	
};

BrickEntity * MakeBrick(BrickColour type, float xpos, float ypos);
void LoadMap(std::string fileName, std::vector<Entity*>& brickList);

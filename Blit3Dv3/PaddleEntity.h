#pragma once

#include "Entity.h"

class PaddleEntity : public Entity
{
public:
	float halfWidth;
	float bend;

	PaddleEntity()
	{
		typeID = ENTITYPADDLE;
		halfWidth = 178.f / (2 * PTM_RATIO); 
		bend = 1.1f; //increasing this increases the amount of bending applied to the ball
	}

	//Method to "bend" flight of ball that hits the paddle
	void HandleCollision(b2Body *ballBody);
};

PaddleEntity * MakePaddle(float xpos, float ypos, Sprite *sprite, float width);
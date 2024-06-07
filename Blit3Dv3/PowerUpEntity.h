#pragma once

#include "Entity.h"

enum class PowerUpType { MULTIBALL,  LARGERPADDLE, CATCH, NONE};

class PowerUpEntity : public Entity
{
private:	
	int currentFrameNum;
	float timeBetweenFrames;
	float frameTimer;
public:
	PowerUpType powerUpType;
	std::vector<Sprite *> spriteList;
	PowerUpEntity();
	void Update(float seconds);
};

PowerUpEntity * MakePowerUp(PowerUpType type, float xpos, float ypos);
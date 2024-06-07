/*
Example program that demonstrates collision handling
*/
#include "Blit3D.h"

#include <random>

#include "Physics.h"
#include "Entity.h"
#include "PaddleEntity.h"
#include "BallEntity.h"
#include "BrickEntity.h"
#include "GroundEntity.h"
#include "PowerUpEntity.h"

#include "MyContactListener.h" //for handling collisions

#include "CollisionMask.h"

#include "Particle.h" //particles, yay!

#include "Camera.h" //Screenshake, more yay!

Blit3D *blit3D = NULL;

//this code sets up memory leak detection
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#define DEBUG_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif


//GLOBAL DATA
std::mt19937 rng;
std::uniform_real_distribution<float> plusMinus5Degrees(-5, +5);
std::uniform_real_distribution<float> plusMinus70Degrees(-70, +70);
std::uniform_real_distribution<float> plusMinus90Degrees(-90, +90);
std::uniform_real_distribution<float> random360Degrees(0, 360);


b2Vec2 gravity; //defines our gravity vector
b2World *world; //our physics engine

// Prepare for simulation. Typically we use a time step of 1/60 of a
// second (60Hz) and ~10 iterations. This provides a high quality simulation
// in most game scenarios.
int32 velocityIterations = 8;
int32 positionIterations = 3;
float timeStep = 1.f / 60.f; //one 60th of a second
float elapsedTime = 0; //used for calculating time passed

//contact listener to handle collisions between important objects
MyContactListener *contactListener;

float cursorX = 0;
PaddleEntity *paddleEntity = NULL;

enum GameState { START, PLAYING, GAMEOVER };
GameState gameState = START;
bool attachedBall = true; //is the ball ready to be launched from the paddle?
int lives = 3;
int scoreValue = 0;
int powerUpControl = 0;
bool status = false;

std::vector<Entity *> brickEntityList; //bricks go here
std::vector<Entity *> ballEntityList; //track the balls seperately from everything else
std::vector<Entity *> entityList; //other entities in our game go here
std::vector<Entity *> deadEntityList; //dead entities

float currentBallSpeed = 70; //defaultball speed
int level = 0; //current level of play

//Sprites 
Sprite *logo = NULL;
Sprite* go = NULL;
Sprite *ballSprite = NULL;
Sprite* bigBallSprite = NULL;
Sprite* paddleSprite = NULL;
Sprite* longPaddleSprite = NULL;
Sprite* voidPaddle = NULL;

Sprite* silverBrickSprite = NULL;
Sprite *whiteBrickSprite = NULL;
Sprite *blueBrickSprite = NULL;
Sprite *orangeBrickSprite = NULL;
Sprite* greenBrickSprite = NULL;

//powerUp Brick
Sprite* catchBrickSprite = NULL;
Sprite* lpBrickSprite = NULL;
Sprite* mbBrickSprite = NULL;

Sprite *multiBallSprites[14];
Sprite* catchBallSprites[14];
Sprite* largePaddleSprites[14];

//font
AngelcodeFont *captainFont = NULL;

//particle stuff
std::vector<Particle*> particleList;

Sprite* cloudSprite1 = NULL;
Sprite* cloudSprite2 = NULL;
Sprite* cloudSprite3 = NULL;

Sprite* explosionSprite1 = NULL;
Sprite* explosionSprite2 = NULL;
Sprite* explosionSprite3 = NULL;
Sprite* explosionSprite4 = NULL;
Sprite* explosionSprite5 = NULL;
Sprite* explosionSprite6 = NULL;
Sprite* explosionSprite7 = NULL;

Sprite* hitBrickSprite1 = NULL;
Sprite* hitBrickSprite2 = NULL;
Sprite* hitBrickSprite3 = NULL;
Sprite* hitBrickSprite4 = NULL;
Sprite* hitBrickSprite5 = NULL;


Camera2D* camera = NULL; //for screenshake

//______MAKE SOME BRICKS_______________________
void MakeLevel()
{
	//delete any extra balls and attach ball
	for (int i = ballEntityList.size() - 1; i > 0; --i)
	{
		world->DestroyBody(ballEntityList[i]->body);
		delete ballEntityList[i];
		ballEntityList.erase(ballEntityList.begin() + i);
	}
	attachedBall = true;

	//delete all extra entities, like powerups
	for (int i = entityList.size() - 1; i >= 0; --i)
	{
		if (entityList[i]->typeID != ENTITYGROUND && entityList[i]->typeID != ENTITYPADDLE)
		{
			world->DestroyBody(entityList[i]->body);
			delete entityList[i];
			entityList.erase(entityList.begin() + i);
		}
	}

	
	currentBallSpeed += 100 * (level * 0.1); //balls move faster by default at higher levels 

	//remove any old bricks, just in case
	for (auto &b : brickEntityList)
	{
		world->DestroyBody(b->body);
		delete b;
	}

	brickEntityList.clear();

	//make the new level
	BrickColour brickColour;
	
	//BrickEntity* b = new BrickEntity();

	switch (level)
	{
	case 1: LoadMap("Level1.txt", brickEntityList);
		break;
	case 2: LoadMap("Level2.txt", brickEntityList);
		break;
	case 3: LoadMap("Level3.txt", brickEntityList);
		break;
	Default:
		int puBricksCounter = 0;
		int randomBrick = 0;
		int previousRandom = 0;
		int controlRandom = 0;
		for (int y = 0; y < 4; ++y)
		{
			for (int x = -8; x <= 8; ++x)
			{
				BrickEntity* b;
				if (puBricksCounter < level + 5)
				{
					do
					{
						std::uniform_int_distribution<int> brickRandom(1, 8);
						randomBrick = brickRandom(rng);
						controlRandom++;
					} while ((previousRandom > 5 && randomBrick > 5) || (randomBrick > 5 && controlRandom < 7));
					if (controlRandom > 6)
						controlRandom = 0;
				}
				else
				{
					std::uniform_int_distribution<int> brickRandom(1, 5);
					randomBrick = brickRandom(rng);
				}
				previousRandom = randomBrick;
				switch (randomBrick)
				{
				case 1:
					brickColour = BrickColour::WHITE;
					break;
				case 2:
					brickColour = BrickColour::ORANGE;
					break;
				case 3:
					brickColour = BrickColour::BLUE;
					break;
				case 4:
					brickColour = BrickColour::GREEN;
					break;
				case 5:
					brickColour = BrickColour::SILVER;
					break;
				case 6:
					brickColour = BrickColour::CATCHBRICK;
					puBricksCounter++;
					break;
				case 7:
					brickColour = BrickColour::LPBRICK;
					puBricksCounter++;
					break;
				case 8:
					brickColour = BrickColour::MBBRICK;
					puBricksCounter++;
					break;
				default:
					brickColour = BrickColour::ORANGE;
					break;

				}

				b = MakeBrick(brickColour, blit3D->screenWidth / 2 + x * 96,
					blit3D->screenHeight * 0.65 + y * 70);

				brickEntityList.push_back(b);

			}
		}



	}


}

//ensures that entities are only added ONCE to the deadEntityList
void AddToDeadList(Entity *e)
{
	bool unique = true;

	for (auto &ent : deadEntityList)
	{
		if (ent == e)
		{
			unique = false;
			break;
		}
	}

	if (unique) deadEntityList.push_back(e);
}

void Init()
{
	//seed random generator
	std::random_device rd;
	rng.seed(rd());

	//turn off the cursor
	blit3D->ShowCursor(false);

	//setup camera
	camera = new Camera2D();

	//set it's valid pan area
	camera->minX = -blit3D->screenWidth / 2;
	camera->minY = -blit3D->screenHeight / 2;
	camera->maxX = blit3D->screenWidth / 2;
	camera->maxY = blit3D->screenHeight / 2;

	camera->PanTo(blit3D->screenWidth / 2, blit3D->screenHeight / 2);

	//Font
	captainFont = blit3D->MakeAngelcodeFontFromBinary32("Media\\captain60.bin");


	//load the sprites
	whiteBrickSprite = blit3D->MakeSprite(0, 0, 96, 48, "Media\\Sprite_sheet.png");
	orangeBrickSprite = blit3D->MakeSprite(0, 48, 96, 48, "Media\\Sprite_sheet.png");
	blueBrickSprite = blit3D->MakeSprite(0, 96, 96, 48, "Media\\Sprite_sheet.png");
	greenBrickSprite = blit3D->MakeSprite(0, 144, 96, 48, "Media\\Sprite_sheet.png");
	silverBrickSprite = blit3D->MakeSprite(0, 384, 96, 48, "Media\\Sprite_sheet.png");

	//powerUp bricks
	catchBrickSprite = blit3D->MakeSprite(96, 432, 96, 48, "Media\\Sprite_sheet.png");
	lpBrickSprite = blit3D->MakeSprite(480, 432, 96, 48, "Media\\Sprite_sheet.png");
	mbBrickSprite = blit3D->MakeSprite(576, 432, 96, 48, "Media\\Sprite_sheet.png");

	//Multiball - catch ball animated sprite lists
	for (int i = 0; i < 14; ++i)
	{
		multiBallSprites[i] = blit3D->MakeSprite(i * 51 + 3, 588, 48, 48, "Media\\Sprite_sheet.png");
		catchBallSprites[i] = blit3D->MakeSprite(i * 51 + 3, 486, 48, 48, "Media\\Sprite_sheet.png");
		largePaddleSprites[i] = blit3D->MakeSprite(i * 51 + 3, 537, 48, 48, "Media\\Sprite_sheet.png");
	}

	logo = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\titleScreen.png");
	go = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\gameOver.png");

	//paddleSprite = blit3D->MakeSprite(0, 1266, 288, 48, "Media\\Sprite_sheet.png");
	paddleSprite = blit3D->MakeSprite(0, 1213, 192, 47, "Media\\Sprite_sheet.png");
	longPaddleSprite = blit3D->MakeSprite(0, 1266, 288, 48, "Media\\Sprite_sheet.png");
	voidPaddle = blit3D->MakeSprite(514, 1266, 288, 48, "Media\\Sprite_sheet.png");
	//paddleSprite = blit3D->MakeSprite(236, 175, 42, 11, "Media\\arinoid_master.png");

	ballSprite = blit3D->MakeSprite(130, 924, 36, 36, "Media\\Sprite_sheet.png");
	bigBallSprite = blit3D->MakeSprite(80, 912, 48, 48, "Media\\Sprite_sheet.png");

	//particle sprites
	cloudSprite1 = blit3D->MakeSprite(355, 419, 26, 26, "Media\\arinoid_master.png");
	cloudSprite2 = blit3D->MakeSprite(386, 419, 28, 29, "Media\\arinoid_master.png");
	cloudSprite3 = blit3D->MakeSprite(421, 423, 23, 21, "Media\\arinoid_master.png");

	explosionSprite1 = blit3D->MakeSprite(3, 165, 25, 24, "Media\\arinoid_master.png");
	explosionSprite2 = blit3D->MakeSprite(37, 163, 23, 24, "Media\\arinoid_master.png");
	explosionSprite3 = blit3D->MakeSprite(69, 166, 21, 20, "Media\\arinoid_master.png");
	explosionSprite4 = blit3D->MakeSprite(102, 168, 19, 17, "Media\\arinoid_master.png");
	explosionSprite5 = blit3D->MakeSprite(136, 170, 15, 14, "Media\\arinoid_master.png");
	explosionSprite6 = blit3D->MakeSprite(170, 172, 11, 11, "Media\\arinoid_master.png");
	explosionSprite7 = blit3D->MakeSprite(204, 174, 7, 7, "Media\\arinoid_master.png");
	
	hitBrickSprite1 = blit3D->MakeSprite(5, 965, 135, 87, "Media\\Sprite_sheet.png");
	hitBrickSprite2 = blit3D->MakeSprite(146, 965, 135, 87, "Media\\Sprite_sheet.png");
	hitBrickSprite3 = blit3D->MakeSprite(288, 965, 135, 87, "Media\\Sprite_sheet.png");
	hitBrickSprite4 = blit3D->MakeSprite(429, 965, 135, 87, "Media\\Sprite_sheet.png");
	hitBrickSprite5 = blit3D->MakeSprite(570, 965, 135, 87, "Media\\Sprite_sheet.png");


	//from here on, we are setting up the Box2D physics world model

	// Define the gravity vector.
	gravity.x = 0.f;
	gravity.y = 0.f;

	// Construct a world object, which will hold and simulate the rigid bodies.
	world = new b2World(gravity);
	//world->SetGravity(gravity); <-can call this to change gravity at any time
	world->SetAllowSleeping(true); //set true to allow the physics engine to 'sleep" objects that stop moving

								   //_________GROUND OBJECT_____________
								   //make an entity for the ground
	GroundEntity *groundEntity = new GroundEntity();
	//A bodyDef for the ground
	b2BodyDef groundBodyDef;
	// Define the ground body.
	groundBodyDef.position.Set(0, 0);

	//add the userdata
	groundBodyDef.userData.pointer = reinterpret_cast<uintptr_t>(groundEntity);

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	groundEntity->body = world->CreateBody(&groundBodyDef);

	//an EdgeShape object, for the ground
	b2EdgeShape groundBox;

	// Define the ground as 1 edge shape at the bottom of the screen.
	b2FixtureDef boxShapeDef;

	boxShapeDef.shape = &groundBox;

	//collison masking
	boxShapeDef.filter.categoryBits = CMASK_GROUND;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_BALL | CMASK_POWERUP;		//it collides wth balls and powerups

																	//bottom
	groundBox.SetTwoSided(b2Vec2(0, 0), b2Vec2(blit3D->screenWidth / PTM_RATIO, 0));
	//Create the fixture
	groundEntity->body->CreateFixture(&boxShapeDef);
	
	//add to the entity list
	entityList.push_back(groundEntity);

	//now make the other 3 edges of the screen on a seperate body
	groundBodyDef.userData.pointer = NULL;
	b2Body *screenEdgeBody = world->CreateBody(&groundBodyDef);

	boxShapeDef.filter.categoryBits = CMASK_EDGES;  //this is the ground
	boxShapeDef.filter.maskBits = CMASK_BALL;		//it collides wth balls
	//note that even if we set the colison mask for colliding with the paddle, it wouldn't work, as
	//Box2D's automatic filtering disables collisions between STATIC objects (the edges) and 
	//KINEMATIC objects (the paddle)

	//left
	groundBox.SetTwoSided(b2Vec2(0, (blit3D->screenHeight-30) / PTM_RATIO), b2Vec2(0, 0));
	screenEdgeBody->CreateFixture(&boxShapeDef);

	//top
	groundBox.SetTwoSided(b2Vec2(0,(blit3D->screenHeight-30) / PTM_RATIO),
		b2Vec2(blit3D->screenWidth / PTM_RATIO, (blit3D->screenHeight-30) / PTM_RATIO));
	screenEdgeBody->CreateFixture(&boxShapeDef);

	//right
	groundBox.SetTwoSided(b2Vec2(blit3D->screenWidth / PTM_RATIO,
		0), b2Vec2(blit3D->screenWidth / PTM_RATIO, (blit3D->screenHeight-30) / PTM_RATIO));
	screenEdgeBody->CreateFixture(&boxShapeDef);

	//______MAKE A BALL_______________________
	BallEntity *ball = MakeBall(ballSprite);

	//move the ball to the center bottomof the window
	b2Vec2 pos(blit3D->screenWidth / 2, 35);							
	pos = Pixels2Physics(pos);//convert coordinates from pixels to physics world
	ball->body->SetTransform(pos, 0.f);

	//add the ball to our list of (ball) entities
	ballEntityList.push_back(ball);

	// Create contact listener and use it to collect info about collisions
	contactListener = new MyContactListener();
	world->SetContactListener(contactListener);

	//paddle
	cursorX = blit3D->screenWidth / 2;
	paddleEntity = MakePaddle(blit3D->screenWidth / 2, 30, paddleSprite, 170);
	entityList.push_back(paddleEntity);
}

void DeInit(void)
{
	if (camera != NULL) delete camera;

	//delete all particles
	for (auto& p : particleList) delete p;
	particleList.clear();

	//delete all the entities
	for (auto &e : entityList) delete e;
	entityList.clear();

	for (auto &b : ballEntityList) delete b;
	ballEntityList.clear();

	for (auto &b : brickEntityList) delete b;
	brickEntityList.clear();

	//delete the contact listener
	delete contactListener;

	//Free all physics game data we allocated
	delete world;

	//any sprites/fonts still allocated are freed automatcally by the Blit3D object when we destroy it
}

void Update(double seconds)
{
	if (gameState == START || gameState == GAMEOVER) return;


	//stop it from lagging hard if more than a small amount of time has passed
	if (seconds > 1.0 / 30) elapsedTime += 1.f / 30;
	else elapsedTime += (float)seconds;
	
	//did we finish the level?
	if (brickEntityList.size() == 0)
	{
		level++;
		MakeLevel();
	}

	//move paddle to where the cursor is
	b2Vec2 paddlePos;
	paddlePos.y = 30;

	float halfWidthPixels = paddleEntity->halfWidth * PTM_RATIO;
	if (cursorX < halfWidthPixels) paddlePos.x = halfWidthPixels;
	else if (cursorX > blit3D->screenWidth - halfWidthPixels) paddlePos.x = blit3D->screenWidth - halfWidthPixels;
	else paddlePos.x = cursorX;

	paddlePos = Pixels2Physics(paddlePos);
	paddleEntity->body->SetTransform(paddlePos, 0);

	//make sure the balls keep moving at the right speed, and delete any way out of bounds
	for (int i = ballEntityList.size() - 1; i >= 0; --i)
	{
		b2Vec2 dir = ballEntityList[i]->body->GetLinearVelocity();

		//check if we are not moving enough vertically
		float minAmountY = 5.f * PTM_RATIO;
		if (dir.y < 0)
		{
			if (dir.y > -minAmountY)
			{
				dir.y = -minAmountY;
			}
		}
		else
		{
			if (dir.y < minAmountY)
			{
				dir.y = minAmountY;
			}
		}
		dir.Normalize();
		dir *= currentBallSpeed; //scale up the velocity tp correct ball speed
		ballEntityList[i]->body->SetLinearVelocity(dir); //apply velocity to kinematic object

		b2Vec2 pos = ballEntityList[i]->body->GetPosition();
		if (pos.x < 0 || pos.y < 0
			|| pos.x * PTM_RATIO > blit3D->screenWidth
			|| pos.y * PTM_RATIO > blit3D->screenHeight)
		{
			//kill this ball, it is out of bounds
			world->DestroyBody(ballEntityList[i]->body);
			delete ballEntityList[i];
			ballEntityList.erase(ballEntityList.begin() + i);
		}
	}

	//sanity check, should always have at least one ball!
	if (ballEntityList.size() == 0)
	{
		//welp, let's make a ball
		BallEntity *ball = MakeBall(ballSprite);
		ballEntityList.push_back(ball);
		attachedBall = true;
	}

	//if ball is attached to paddle, move ball to where paddle is
	if (attachedBall)									
	{
		assert(ballEntityList.size() > 0); //make sure there is at least one ball
		b2Vec2 ballPos = paddleEntity->body->GetPosition();
		ballPos.y = 70 / PTM_RATIO;
		ballEntityList[0]->body->SetTransform(ballPos, 0);
		ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
	}
	
	if (status == true)
	{
		if (powerUpControl > 1000)
		{
			status	= false;
			paddleEntity->sprite = voidPaddle;
			deadEntityList.push_back(paddleEntity);

			b2Vec2 pos = paddleEntity->body->GetPosition();
			paddleEntity = MakePaddle(pos.x, 30, paddleSprite, 178);
			entityList.push_back(paddleEntity);
			paddleEntity->halfWidth = 178 / (2 * PTM_RATIO);
			powerUpControl = 0;
			
		}
		else
			powerUpControl++;
	}
	int counter =0;
	for (int i = 0; i < entityList.size(); i++)
	{ 
		if (entityList.at(i)->typeID == ENTITYPADDLE)
		{
			PaddleEntity* x = (PaddleEntity*)entityList.at(i);
			counter++;
			
		}
	}
	


	//don't apply physics unless at least a timestep worth of time has passed
	while (elapsedTime >= timeStep)
	{
		//update the physics world
		world->Step(timeStep, velocityIterations, positionIterations);

		// Clear applied body forces. 
		world->ClearForces();

		elapsedTime -= timeStep;

		//update game logic/animation
		for (auto &e : entityList) e->Update(timeStep);
		for (auto &b : ballEntityList) b->Update(timeStep);
		for (auto &b : brickEntityList) b->Update(timeStep);

		camera->Update(timeStep);

		//update the particle list and remove dead particles
		for (int i = particleList.size() - 1; i >= 0; --i)
		{
			if (particleList[i]->Update(timeStep))
			{
				//time to die!
				delete particleList[i];
				particleList.erase(particleList.begin() + i);
			}
		}

		//loop over contacts
		MyContact contact;
		for (int pos = 0; pos < contactListener->contacts.size(); ++pos)
		{
			contact = contactListener->contacts[pos];

			//fetch the entities from the body userdata
			Entity *A = (Entity *)contact.fixtureA->GetBody()->GetUserData().pointer;
			Entity *B = (Entity *)contact.fixtureB->GetBody()->GetUserData().pointer;

			if (A != NULL && B != NULL) //if there is an entity for these objects...
			{
				if (A->typeID == ENTITYBALL)
				{
					//swap A and B
					Entity *C = A;
					A = B;
					B = C;
				}

				if (B->typeID == ENTITYBALL && A->typeID == ENTITYBRICK)
				{
					BrickEntity *be = (BrickEntity *)A;
					if (be->HandleCollision())
					{
						AddToDeadList(be);
						if (be->colour == BrickColour::BLUE || be->colour == BrickColour::GREEN || be->colour == BrickColour::SILVER)
						{
							scoreValue += 20;
						}
					}
					//check for powerUps
					if (be->colour == BrickColour::MBBRICK)
					{
						b2Vec2 position = be->body->GetPosition();
						position = Physics2Pixels(position);
						PowerUpEntity *p = MakePowerUp(PowerUpType::MULTIBALL, position.x, position.y);
						entityList.push_back(p);
					}
					else if (be->colour == BrickColour::CATCHBRICK)
					{
						b2Vec2 position = be->body->GetPosition();
						position = Physics2Pixels(position);
						PowerUpEntity* p = MakePowerUp(PowerUpType::CATCH, position.x, position.y);
						entityList.push_back(p);
					}
					else if (be->colour == BrickColour::LPBRICK)
					{
						b2Vec2 position = be->body->GetPosition();
						position = Physics2Pixels(position);
						PowerUpEntity* p = MakePowerUp(PowerUpType::LARGERPADDLE, position.x, position.y);
						entityList.push_back(p);
					}

					//add a particle effect
					Particle* p = new Particle();
					p->spriteList.push_back(hitBrickSprite1);
					p->spriteList.push_back(hitBrickSprite2);
					p->spriteList.push_back(hitBrickSprite3);
					p->spriteList.push_back(hitBrickSprite4);
					p->spriteList.push_back(hitBrickSprite5);
					//let's make it 'follow' after the ball
					b2Vec2 dir = B->body->GetLinearVelocity();
					float speed = dir.Length() * PTM_RATIO;
					dir.Normalize();
					p->direction = dir;
					p->startingSpeed = speed / 5;
					p->targetSpeed = speed / 10;
					p->totalTimeToLive = 0.2f;
					//get coords of contact
					p->coords = Physics2Pixels(contact.contactPoint);

					particleList.push_back(p);

					//shake the screen!
					camera->Shake(1.8);
				}

				if (B->typeID == ENTITYBALL && A->typeID == ENTITYPADDLE)
				{
					PaddleEntity *pe = (PaddleEntity *)A;
					//bend the ball's flight
					pe->HandleCollision(B->body);

				}

				if (B->typeID == ENTITYBALL && A->typeID == ENTITYGROUND)
				{
					if (ballEntityList.size() > 1)
					{
						//remove this ball, but we have others
						AddToDeadList(B);
						//shake the screen a little
						camera->Shake(3);
					}
					else
					{
						//shake the screen!
						camera->Shake(10);
						//lose a life
						lives--;
						attachedBall = true; //attach the ball for launching
						ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
						if (lives < 1)
						{
							//gameover
							gameState = GAMEOVER;
						}
					}

					//add a particle effect
					Particle* p = new Particle();
					p->spriteList.push_back(explosionSprite4);
					p->spriteList.push_back(explosionSprite1);
					p->spriteList.push_back(explosionSprite1);
					p->spriteList.push_back(explosionSprite2);
					p->spriteList.push_back(explosionSprite3);
					p->spriteList.push_back(explosionSprite4);
					p->spriteList.push_back(explosionSprite5);
					p->spriteList.push_back(explosionSprite6);
					p->spriteList.push_back(explosionSprite7);
					p->spriteList.push_back(cloudSprite3);
					p->spriteList.push_back(cloudSprite2);
					p->spriteList.push_back(cloudSprite1);
					p->spriteList.push_back(cloudSprite1);
					p->spriteList.push_back(cloudSprite2);
					p->spriteList.push_back(cloudSprite3);
					p->rotationSpeed = plusMinus90Degrees(rng);
					p->angle = random360Degrees(rng);
					p->direction = b2Vec2(0.f, 0.f);
					p->startingSpeed = 0;
					p->targetSpeed = 0;
					p->totalTimeToLive = 0.5f;
					//get coords of ball
					p->coords = Physics2Pixels(B->body->GetPosition());

					particleList.push_back(p);
				}

				//check and see if paddle or ground hit a powerup 
				if (A->typeID == ENTITYPOWERUP)
				{
					//swap A and B
					Entity *C = A;
					A = B;
					B = C;
				}

				if (B->typeID == ENTITYPOWERUP)
				{
					AddToDeadList(B);

					if (A->typeID == ENTITYPADDLE)
					{
						PowerUpEntity *p = (PowerUpEntity *)B;
						switch (p->powerUpType)
						{
						case PowerUpType::MULTIBALL:
							for (int j = ballEntityList.size() - 1; j >= 0; --j)
							{
								//make 2 new balls
								for (int i = 0; i < 2; ++i)
								{
									//add extra balls if we haven't reached pur limit
									if (ballEntityList.size() < 30)
									{
										BallEntity *b = MakeBall(ballSprite);
										b2Vec2 ballPos = ballEntityList[j]->body->GetPosition();
										ballPos.x += (i * 18 - 9) / PTM_RATIO;
										b->body->SetTransform(ballPos, 0);
										//kick the ball in a random direction upwards			
										b2Vec2 dir = deg2vec(90 + plusMinus70Degrees(rng)); //between 20-160 degrees
																				
										//make the ball move
										dir *= currentBallSpeed; //scale up the velocity
										b->body->SetLinearVelocity(dir); //apply velocity to kinematic object	
																		 //add to the ballist
										ballEntityList.push_back(b);
									}
								}
							}
							break;

						case PowerUpType::CATCH:

							attachedBall = true; //attach the ball to paddle
							break;

						case PowerUpType::LARGERPADDLE:
							b2Vec2 pos = paddleEntity->body->GetPosition();
							paddleEntity->sprite = voidPaddle;
							//cursorX = pos.x;
							deadEntityList.push_back(paddleEntity);
							paddleEntity = MakePaddle(pos.x, 30, longPaddleSprite, 270);
							paddleEntity->halfWidth = 270 / (2 * PTM_RATIO);
							entityList.push_back(paddleEntity);
							status = true;
							powerUpControl = 0;
							break;

						case PowerUpType::NONE:
						default:
							assert("Unknown PowerUp type" == 0);
							break;
						}//end switch on powerup type
					}
				}
			}//end of checking if they are both NULL userdata
		}//end of collison handling

		



		 //clean up dead entities
		for (auto &e : deadEntityList)
		{
			//remove body from the physics world and free the body data
			world->DestroyBody(e->body);
			//remove the entity from the appropriate entityList
			if (e->typeID == ENTITYBALL)
			{
				for (int i = 0; i < ballEntityList.size(); ++i)
				{
					if (e == ballEntityList[i])
					{
						delete ballEntityList[i];
						ballEntityList.erase(ballEntityList.begin() + i);
						break;
					}
				}
			}
			else if (e->typeID == ENTITYBRICK)
			{
				for (int i = 0; i < brickEntityList.size(); ++i)
				{
					if (e == brickEntityList[i])
					{
						delete brickEntityList[i];
						brickEntityList.erase(brickEntityList.begin() + i);
						break;
					}
				}
			}
			else
			{
				for (int i = 0; i < entityList.size(); ++i)
				{
					if (e == entityList[i])
					{
						delete entityList[i];
						entityList.erase(entityList.begin() + i);
						break;
					}
				}
			}
		}

		deadEntityList.clear();
	}
}

void Draw(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera->Draw();
	std::string scoreV;
	switch (gameState)
	{
	case START:
		logo->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		break;
	case GAMEOVER:
		go->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);
		scoreV = "Final score: " + std::to_string(scoreValue);
		captainFont->BlitText((blit3D->screenWidth / 2) - (scoreV.size() + 100), 440, scoreV);
		break;
	case PLAYING:
		//loop over all entities and draw them
		for (auto &e : entityList) e->Draw();
		for (auto &b : brickEntityList) b->Draw();
		for (auto &b : ballEntityList) b->Draw();
		for (auto& p : particleList) p->Draw();

		std::string scoreString = "Score: " + std::to_string(scoreValue);
		captainFont->BlitText(10, blit3D->screenHeight - 10, scoreString);

		std::string lifeString = "Lives: " + std::to_string(lives);
		captainFont->BlitText(blit3D->screenWidth - (scoreString.size() + 150), blit3D->screenHeight - 10, lifeString);

		break;
	}
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence

	//suicide button is K
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
	{
		if (gameState == PLAYING)
		{
			//kill off the current first ball
			lives--;
			attachedBall = true; //attach the ball for launching

								 //safety check
			//assert(ballEntityList.size() > 0); //make sure there is at least one ball

			ballEntityList[0]->body->SetLinearVelocity(b2Vec2(0.f, 0.f));
			if (lives < 1)
			{
				//gameover
				gameState = GAMEOVER;
			}
		}
	}
}

void DoCursor(double x, double y)
{
	//scale display size
	cursorX = static_cast<float>(x) * (blit3D->screenWidth / blit3D->trueScreenWidth);
}

void DoMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		switch (gameState)
		{
		case GAMEOVER:
			gameState = START;
			break;
		case START:
			gameState = PLAYING;
			attachedBall = true;
			lives = 3;
			level = 1;
			scoreValue = 0;
			MakeLevel();
			break;

		case PLAYING:
			if (attachedBall)
			{
				attachedBall = false;
				//launch the ball

				//safety check
				assert(ballEntityList.size() >= 1); //make sure there is at least one ball	

				Entity *b = ballEntityList[0];
				//kick the ball in a random direction upwards			
				b2Vec2 dir = deg2vec(90 + plusMinus5Degrees(rng)); //between 85-95 degrees
														//make the ball move
				dir *= currentBallSpeed; //scale up the velocity
				b->body->SetLinearVelocity(dir); //apply velocity to kinematic object				
			}
			break;
		}
	}

}

int main(int argc, char *argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//blit3D = new Blit3D(Blit3DWindowModel::DECORATEDWINDOW, 720, 480);
	blit3D = new Blit3D(Blit3DWindowModel::BORDERLESSFULLSCREEN_1080P, 1920, 1080);

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);
	blit3D->SetDoCursor(DoCursor);
	blit3D->SetDoMouseButton(DoMouseButton);

	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);
	if (blit3D) delete blit3D;
}
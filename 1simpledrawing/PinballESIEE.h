// ESIEE Paris - E4 Summer project 2020
// @filename: PinballESIEE.h
// @description:	This file contains all the instructions and interactions for the Pinball
//					by using a gameState manager to handle all the states for the Pinball.
//					It calls also other classes such as the Render and the Engine for setup PhysX and OpenGL


#include <vector>
#include <string>
#include "globals.h"
//#include "Engine.h"
#include "PinballObjects.h"

class PinballESIEE
{
private:
	enum GameState {
		Init,
		Menu,
		Game,
		GameOver,
		Paused
	} gameState;

	// Add Objects to the scene
	void InitBoard();

	void AddActors();

public:
	PinballESIEE();
	~PinballESIEE();

	void Render();
	void GameStatus();
	void Reshape();
	void KeyboardDown();
	void Reset();
	void Exit();
};


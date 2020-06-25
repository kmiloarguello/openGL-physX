#pragma once
/****

@author Group Project - OpenGL ESIEE

****/

#include <vector>
#include <string>
#include "globals.h";

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


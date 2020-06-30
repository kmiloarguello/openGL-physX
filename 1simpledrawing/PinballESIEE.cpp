// ESIEE Paris - E4 Summer project 2020
// @filename: PinballESIEE.h
// @description:	This file contains all the instructions and interactions for the Pinball
//					by using a gameState manager to handle all the states for the Pinball.
//					It calls also other classes such as the Render and the Engine for setup PhysX and OpenGL

#include "PinballESIEE.h"
#include "PinballObjects.h"

PinballESIEE::PinballESIEE() {}
PinballESIEE::~PinballESIEE(){}

void PinballESIEE::AddActors() {
	//Plane* ground = new Plane(PxVec3(0.0f,2.0f,0.0f), 1.0f);

}

void PinballESIEE::Render() {
	if (gameState == GameState::Game) {

	}

	if (gameState == GameState::Menu) {

	}
	else if (gameState == GameState::Init) {

	}
	else if (gameState == GameState::GameOver) {

	}
}

void PinballESIEE::GameStatus() {
	if (gameState == GameState::Game) {

	}
}

void PinballESIEE::Reshape() {

}

void PinballESIEE::KeyboardDown() {

}

void PinballESIEE::Reset() {

}

void PinballESIEE::Exit() {

}

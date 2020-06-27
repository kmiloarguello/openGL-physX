#pragma once
#include <iostream>
#include <vector>
#include <string>

#include <stdlib.h>
#include "globals.h"

using namespace physx;
using namespace std;

class Actor {
private:
	PxActor* actor;

public:
	// Constructor and destructor
	Actor();
	~Actor();

	// Getter and Setters

	PxActor* Get();

	void Name(const string& name);
	string Name();

	void Color(PxVec3 color);
	PxVec3* Color();

	void Material();
	void CreateShape();
};


///
/// Inherits the actor properties to create a new actor with static parameters
///
class StaticActor : public Actor {
public:
	StaticActor(const PxTransform& pose);
	~StaticActor();

	void CreateShape(const PxGeometry& geometry);
};


///
/// Inherits the actor properties to create a new actor with Dynamic parameters
///
class DynamicActor : public Actor {
public:
	DynamicActor();
	~DynamicActor();

	void CreateShape(const PxGeometry& geometry);
	void SetKinematic(bool value);
};


///
/// Creates a new Plane
/// new Plane( PxVec3 normal , PxReal distance )
///
class Plane : public StaticActor {
public: 
	Plane(PxVec3 planeEquation = PxVec3(0.f, 1.f, 0.f), PxReal distance = 0.f) 
		: StaticActor(PxTransformFromPlaneEquation( PxPlane(planeEquation, distance ) )) {
		CreateShape(PxPlaneGeometry());
	}

	Plane();

};


class Paddle {

};


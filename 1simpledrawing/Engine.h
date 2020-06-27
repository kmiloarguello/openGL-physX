#pragma once

#include <vector>
#include <string>
#include "globals.h"

using namespace physx;
using namespace std;

PxFoundation* GetFoundation();
PxPvd* GetPvd();
PxCooking* GetCooking();
PxPhysics* GetPhysX();

PxScene* Start();

///
/// Abstract class for create an actor
/// @desc	Creates an actor
///
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
/// Creates a new scene
///
/*class Scene {

private:
	PxScene* scene;

public:
	Scene();
	~Scene();

	PxScene* Get();
	void Start();
	void Update();
	void End();
};*/

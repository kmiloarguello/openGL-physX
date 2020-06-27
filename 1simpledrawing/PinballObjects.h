#pragma once
#include <iostream>
#include <stdlib.h>
#include "Engine.h"

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
};


class Lever {

};


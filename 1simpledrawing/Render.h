// ESIEE Paris - E4 Summer project 2020
// @filename: Render.h
// @description: This file contains all the instructions for the render and camera settings


#pragma once
#include "globals.h"
using namespace physx;

class Render
{
public:
	Render();
	~Render();

	void setupColors();
	void setupLights();
	void renderActors(PxRigidActor** actors, const PxU32 numActors, bool shadows, const PxVec3& color);
	PX_FORCE_INLINE void renderGeometryHolder(const PxGeometryHolder& h);
	void renderGeometry(const PxGeometry& geom);
	PxRigidDynamic* createConvexMesh(PxPhysics& physics, PxVec3* verts, PxU32 numVerts, PxVec3& position, PxMaterial& material);
	PxConvexMesh* createConvexMesh2(PxPhysics& physics, PxVec3* verts, PxU32 numVerts);
};


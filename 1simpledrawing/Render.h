// ESIEE Paris - E4 Summer project 2020
// @filename: Render.h
// @description: This file contains all the instructions for the render and camera settings


#pragma once
#include "globals.h"
#include "foundation/PxTransform.h"
using namespace physx;

namespace Render
{
	void					setupColors();
	void					setupLights();
	void					renderActors(PxRigidActor** actors, const PxU32 numActors, bool shadows, const PxVec3& color);
	PX_FORCE_INLINE void	renderGeometryHolder(const PxGeometryHolder& h);
	void					renderGeometry(const PxGeometry& geom);
	PxRigidDynamic*			createConvexMesh(PxPhysics& physics, PxVec3* verts, PxU32 numVerts, PxVec3& position, PxMaterial& material);
	PxConvexMesh*			createConvexMesh2(PxPhysics& physics, PxVec3* verts, PxU32 numVerts);
	void					startRender(const PxVec3& cameraEye, const PxVec3& cameraDir, PxReal clipNear, PxReal clipFar);
	void					finishRender();

	class Camera
	{
	public:
		Camera(const physx::PxVec3& eye, const physx::PxVec3& dir);

		void				handleMouse(int button, int state, int x, int y);
		bool				handleKey(unsigned char key, int x, int y, float speed = 1.0f);
		void				handleMotion(int x, int y);
		void				handleAnalogMove(float x, float y);

		physx::PxVec3		getEye()	const;
		physx::PxVec3		getDir()	const;
		physx::PxTransform	getTransform() const;
	private:
		physx::PxVec3	mEye;
		physx::PxVec3	mDir;
		int				mMouseX;
		int				mMouseY;
	};
}


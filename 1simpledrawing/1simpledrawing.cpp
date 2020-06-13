#include <fstream>
#include <string>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#undef main

#define GLM_FORCE_RADIANS
#define PVD_HOST "127.0.0.1"
#define MAX_NUM_ACTOR_SHAPES 128
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include "PxPhysicsAPI.h"

#include "myShader.h"

using namespace std;
using namespace physx;

// SDL variables
SDL_Window* window;
SDL_GLContext glContext;

// PhysX Variables
PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;

PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;

PxMaterial* gMaterial = NULL;
PxMaterial* gMaterial2 = NULL;

PxPvd* gPvd = NULL;

vector<PxRigidActor*> boxes;
#define MAX_NUM_MESH_VEC3S  1024
static PxVec3 gVertexBuffer[MAX_NUM_MESH_VEC3S];
static float gCylinderData[] = {
	1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,
	0.866025f,0.500000f,1.0f,0.866025f,0.500000f,1.0f,0.866025f,0.500000f,0.0f,0.866025f,0.500000f,0.0f,
	0.500000f,0.866025f,1.0f,0.500000f,0.866025f,1.0f,0.500000f,0.866025f,0.0f,0.500000f,0.866025f,0.0f,
	-0.0f,1.0f,1.0f,-0.0f,1.0f,1.0f,-0.0f,1.0f,0.0f,-0.0f,1.0f,0.0f,
	-0.500000f,0.866025f,1.0f,-0.500000f,0.866025f,1.0f,-0.500000f,0.866025f,0.0f,-0.500000f,0.866025f,0.0f,
	-0.866025f,0.500000f,1.0f,-0.866025f,0.500000f,1.0f,-0.866025f,0.500000f,0.0f,-0.866025f,0.500000f,0.0f,
	-1.0f,-0.0f,1.0f,-1.0f,-0.0f,1.0f,-1.0f,-0.0f,0.0f,-1.0f,-0.0f,0.0f,
	-0.866025f,-0.500000f,1.0f,-0.866025f,-0.500000f,1.0f,-0.866025f,-0.500000f,0.0f,-0.866025f,-0.500000f,0.0f,
	-0.500000f,-0.866025f,1.0f,-0.500000f,-0.866025f,1.0f,-0.500000f,-0.866025f,0.0f,-0.500000f,-0.866025f,0.0f,
	0.0f,-1.0f,1.0f,0.0f,-1.0f,1.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,
	0.500000f,-0.866025f,1.0f,0.500000f,-0.866025f,1.0f,0.500000f,-0.866025f,0.0f,0.500000f,-0.866025f,0.0f,
	0.866026f,-0.500000f,1.0f,0.866026f,-0.500000f,1.0f,0.866026f,-0.500000f,0.0f,0.866026f,-0.500000f,0.0f,
	1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f
};

// GUI variables
bool quit = false;
int mouse_position[2];
bool button_pressed = false;
int window_height = 480;
int window_width = 640;

// Projection variables
float fovy = 45.0f;
float znear = 1.0f;
float zfar = 2000.0f;

// Camera variables
glm::vec3 camera_eye = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);

void rotate(glm::vec3& inputvec, glm::vec3 rotation_axis, float theta, bool tonormalize = false)
{
	const float cos_theta = cos(theta);
	const float dot = glm::dot(inputvec, rotation_axis);
	glm::vec3 cross = glm::cross(inputvec, rotation_axis);

	inputvec *= cos_theta;
	inputvec += rotation_axis * dot * (float)(1.0 - cos_theta);
	inputvec -= cross * sin(theta);

	if (tonormalize) inputvec = glm::normalize(inputvec);
}

// Process the event.  
void processEvents(SDL_Event current_event)
{
	switch (current_event.type)
	{
	case SDL_QUIT:
	{
		quit = true;
		break;
	}
	case SDL_KEYDOWN:
	{
		if (current_event.key.keysym.sym == SDLK_ESCAPE)
			quit = true;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		mouse_position[0] = current_event.button.x;
		mouse_position[1] = window_height - current_event.button.y;
		button_pressed = true;
		break;
	}
	case SDL_MOUSEBUTTONUP:
	{
		button_pressed = false;
		break;
	}
	case SDL_MOUSEMOTION:
	{
		if (button_pressed == false) break;

		int x = current_event.motion.x;
		int y = window_height - current_event.motion.y;

		int dx = x - mouse_position[0];
		int dy = y - mouse_position[1];

		if (dx == 0 && dy == 0) break;

		mouse_position[0] = x;
		mouse_position[1] = y;

		float vx = (float)dx / (float)window_width;
		float vy = (float)dy / (float)window_height;
		float theta = 4.0f * (fabs(vx) + fabs(vy));

		glm::vec3 camera_right = glm::normalize(glm::cross(camera_forward, camera_up));

		glm::vec3 tomovein_direction = -camera_right * vx + -camera_up * vy;

		glm::vec3 rotation_axis = glm::normalize(glm::cross(tomovein_direction, camera_forward));

		rotate(camera_forward, rotation_axis, theta, true);
		rotate(camera_up, rotation_axis, theta, true);
		rotate(camera_eye, rotation_axis, theta, false);

		break;
	}
	case SDL_MOUSEWHEEL:
	{
		if (current_event.wheel.y < 0)
			camera_eye -= 0.1f * camera_forward;
		else if (current_event.wheel.y > 0)
			camera_eye += 0.1f * camera_forward;
	}
	default:
		break;
	}
}


void initPhysics(void)
{
	cout << "Initializing PhysX" << endl;
	// STEP 2.
	// Initialization of PhysX
	// The gFoundation instance initialize PhysX
	// This uses some callbacks for memory allocation and errors and also the ID version
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	// PVD is used for visual debugging in PhysX
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// STEP 3.
	// Here the library initialize the Physics with some tolerance (this can be updated) 
	// ? The tolerance is in animation, maybe gravity, etc
	// This PxCreatePhysics does not have Articulations nor Height fields
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	// STEP 4.
	// It is good to mention the simulation part and the time slots fetch()

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	// For handling the Threads, PhysX uses a gDispatcher
	// GPU Optimization
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	// PhysX Works using Tasks, specially for simulation

	// STEP 5
	// Creating the scene with the init parameters
	// Every scene uses a Thread Local Storage slot.
	gScene = gPhysics->createScene(sceneDesc);

	//A scene is a collection of bodies and constraints which can interact
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	gMaterial = gPhysics->createMaterial(1.0f, 1.0f, 1.0f);
	gMaterial2 = gPhysics->createMaterial(0.1f, 0.1f, 1.0f);

	// STEP 6
	// BASE -> Actor -> RigidBody
	// PxRigidStatic simulates a rigid body object 
	// PxCreatePlane is method to create planes of equation a.x + b = 0
	// PxPlane Normal Vector - Distance to the origin (last parameter)
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	// Adding this actor to the scene
	gScene->addActor(*groundPlane);
	boxes.push_back(groundPlane);


	PxRigidDynamic* ball = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(0.0f, 100.0f, 0.0f)), PxSphereGeometry(3.0f), *gMaterial, 1.0f);
	// Damping = amortiguamiento
	ball->setLinearDamping(0.05f);
	// Add a velocity
	ball->setLinearVelocity(PxVec3(0, -50, 0));
	gScene->addActor(*ball);
	boxes.push_back(ball);

}

void cleanupPhysics()
{
	gPhysics->release();
	gFoundation->release();
	printf("SnippetHelloWorld done.\n");
}


///////////////// RENDER ACTORS IN THE SCENE /////////////
// ! ISSUE: Glut is undefined so many objects are marking error
static void renderGeometry(const PxGeometry& geom)
{
	switch (geom.getType())
	{
	case PxGeometryType::eBOX:
	{
		const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);
		glScalef(boxGeom.halfExtents.x, boxGeom.halfExtents.y, boxGeom.halfExtents.z);
		glutSolidCube(2);
	}
	break;

	case PxGeometryType::eSPHERE:
	{
		const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);
		glutSolidSphere(GLdouble(sphereGeom.radius), 10, 10);
	}
	break;

	case PxGeometryType::eCAPSULE:
	{
		const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);
		const PxF32 radius = capsuleGeom.radius;
		const PxF32 halfHeight = capsuleGeom.halfHeight;

		//Sphere
		glPushMatrix();
		glTranslatef(halfHeight, 0.0f, 0.0f);
		glScalef(radius, radius, radius);
		glutSolidSphere(1, 10, 10);
		glPopMatrix();

		//Sphere
		glPushMatrix();
		glTranslatef(-halfHeight, 0.0f, 0.0f);
		glScalef(radius, radius, radius);
		glutSolidSphere(1, 10, 10);
		glPopMatrix();

		//Cylinder
		glPushMatrix();
		glTranslatef(-halfHeight, 0.0f, 0.0f);
		glScalef(2.0f * halfHeight, radius, radius);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glVertexPointer(3, GL_FLOAT, 2 * 3 * sizeof(float), gCylinderData);
		glNormalPointer(GL_FLOAT, 2 * 3 * sizeof(float), gCylinderData + 3);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 13 * 2);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glPopMatrix();
	}
	break;

	case PxGeometryType::eCONVEXMESH:
	{
		const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom);

		//Compute triangles for each polygon.
		const PxVec3& scale = convexGeom.scale.scale;
		PxConvexMesh* mesh = convexGeom.convexMesh;
		const PxU32 nbPolys = mesh->getNbPolygons();
		const PxU8* polygons = mesh->getIndexBuffer();
		const PxVec3* verts = mesh->getVertices();
		PxU32 nbVerts = mesh->getNbVertices();
		PX_UNUSED(nbVerts);

		PxU32 numTotalTriangles = 0;
		for (PxU32 i = 0; i < nbPolys; i++)
		{
			PxHullPolygon data;
			mesh->getPolygonData(i, data);

			const PxU32 nbTris = PxU32(data.mNbVerts - 2);
			const PxU8 vref0 = polygons[data.mIndexBase + 0];
			PX_ASSERT(vref0 < nbVerts);
			for (PxU32 j = 0;j < nbTris;j++)
			{
				const PxU32 vref1 = polygons[data.mIndexBase + 0 + j + 1];
				const PxU32 vref2 = polygons[data.mIndexBase + 0 + j + 2];

				//generate face normal:
				PxVec3 e0 = verts[vref1] - verts[vref0];
				PxVec3 e1 = verts[vref2] - verts[vref0];

				PX_ASSERT(vref1 < nbVerts);
				PX_ASSERT(vref2 < nbVerts);

				PxVec3 fnormal = e0.cross(e1);
				fnormal.normalize();

				if (numTotalTriangles * 6 < MAX_NUM_MESH_VEC3S)
				{
					gVertexBuffer[numTotalTriangles * 6 + 0] = fnormal;
					gVertexBuffer[numTotalTriangles * 6 + 1] = verts[vref0];
					gVertexBuffer[numTotalTriangles * 6 + 2] = fnormal;
					gVertexBuffer[numTotalTriangles * 6 + 3] = verts[vref1];
					gVertexBuffer[numTotalTriangles * 6 + 4] = fnormal;
					gVertexBuffer[numTotalTriangles * 6 + 5] = verts[vref2];
					numTotalTriangles++;
				}
			}
		}
		glPushMatrix();
		glScalef(scale.x, scale.y, scale.z);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glNormalPointer(GL_FLOAT, 2 * 3 * sizeof(float), gVertexBuffer);
		glVertexPointer(3, GL_FLOAT, 2 * 3 * sizeof(float), gVertexBuffer + 1);
		glDrawArrays(GL_TRIANGLES, 0, int(numTotalTriangles * 3));
		glPopMatrix();
	}
	break;

	case PxGeometryType::eTRIANGLEMESH:
	{
		const PxTriangleMeshGeometry& triGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

		const PxTriangleMesh& mesh = *triGeom.triangleMesh;
		const PxVec3 scale = triGeom.scale.scale;

		const PxU32 triangleCount = mesh.getNbTriangles();
		const PxU32 has16BitIndices = mesh.getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES;
		const void* indexBuffer = mesh.getTriangles();

		const PxVec3* vertexBuffer = mesh.getVertices();

		const PxU32* intIndices = reinterpret_cast<const PxU32*>(indexBuffer);
		const PxU16* shortIndices = reinterpret_cast<const PxU16*>(indexBuffer);
		PxU32 numTotalTriangles = 0;
		for (PxU32 i = 0; i < triangleCount; ++i)
		{
			PxVec3 triVert[3];

			if (has16BitIndices)
			{
				triVert[0] = vertexBuffer[*shortIndices++];
				triVert[1] = vertexBuffer[*shortIndices++];
				triVert[2] = vertexBuffer[*shortIndices++];
			}
			else
			{
				triVert[0] = vertexBuffer[*intIndices++];
				triVert[1] = vertexBuffer[*intIndices++];
				triVert[2] = vertexBuffer[*intIndices++];
			}

			PxVec3 fnormal = (triVert[1] - triVert[0]).cross(triVert[2] - triVert[0]);
			fnormal.normalize();

			if (numTotalTriangles * 6 < MAX_NUM_MESH_VEC3S)
			{
				gVertexBuffer[numTotalTriangles * 6 + 0] = fnormal;
				gVertexBuffer[numTotalTriangles * 6 + 1] = triVert[0];
				gVertexBuffer[numTotalTriangles * 6 + 2] = fnormal;
				gVertexBuffer[numTotalTriangles * 6 + 3] = triVert[1];
				gVertexBuffer[numTotalTriangles * 6 + 4] = fnormal;
				gVertexBuffer[numTotalTriangles * 6 + 5] = triVert[2];
				numTotalTriangles++;
			}
		}
		glPushMatrix();
		glScalef(scale.x, scale.y, scale.z);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glNormalPointer(GL_FLOAT, 2 * 3 * sizeof(float), gVertexBuffer);
		glVertexPointer(3, GL_FLOAT, 2 * 3 * sizeof(float), gVertexBuffer + 1);
		glDrawArrays(GL_TRIANGLES, 0, int(numTotalTriangles * 3));
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glPopMatrix();
	}
	break;

	case PxGeometryType::eINVALID:
	case PxGeometryType::eHEIGHTFIELD:
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::ePLANE:
		break;
	}
}


///////////////// INITIALIZE RENDER ACTORS ///////////////
static PX_FORCE_INLINE void renderGeometryHolder(const PxGeometryHolder& h)
{
	renderGeometry(h.any()); // It will render any actor in the scene
}



int main(int argc, char* argv[])
{
	// Initialize video subsystem
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	// Using OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// Create window
	window = SDL_CreateWindow("BasicDrawing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	// Create OpenGL context
	glContext = SDL_GL_CreateContext(window);

	// Initialize glew
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Default color when we clear the color buffer
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	myShader* shader = new myShader("basic-vertexshader.glsl", "basic-fragmentshader.glsl");
	shader->start();

	// Initialize PhysX
	initPhysics();

	// display loop
	while (!quit)
	{
		glViewport(0, 0, window_width, window_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection_matrix = glm::perspective(glm::radians(fovy), static_cast<float>(window_width) / static_cast<float>(window_height), znear, zfar);
		glUniformMatrix4fv(glGetUniformLocation(shader->shaderprogram, "myprojection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

		glm::mat4 view_matrix = glm::lookAt(camera_eye, camera_eye + camera_forward, camera_up);
		glUniformMatrix4fv(glGetUniformLocation(shader->shaderprogram, "myview_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));

		glUniform4fv(glGetUniformLocation(shader->shaderprogram, "input_color"), 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));

		glColor4f(0.4f, 0.4f, 0.4f, 1.0f);

		////////////////////////// IMPLEMENTING AND RENDERING PHYSX /////////////////////////////
		
		// Get the scene
		PxGetPhysics().getScenes(&gScene, 1);

		// Get how many actors are in the scene (with dynamic or rigid)
		PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
		
		// If there is any actor...
		if (nbActors) {
			//cout << "actooors: " << nbActors << endl;
			
			// Get the actors
			std::vector<PxRigidActor*> actors(nbActors);
			gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
			
			// Render the actors
			
			const PxVec3 color(0.0f,1.0f,0.0f); 
			const PxVec3 shadowDir(0.0f, -0.7071067f, -0.7071067f);
			const PxReal shadowMat[] = { 1,0,0,0, -shadowDir.x / shadowDir.y,0,-shadowDir.z / shadowDir.y,0, 0,0,1,0, 0,0,0,1 };

			PxShape* shapes[MAX_NUM_ACTOR_SHAPES];
			for (PxU32 i = 0;i < static_cast<PxU32>(actors.size());i++)
			{
				const PxU32 nbShapes = actors[i]->getNbShapes();
				PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
				actors[i]->getShapes(shapes, nbShapes);
				//const bool sleeping = actors[i]->is<PxRigidDynamic>() ? actors[i]->is<PxRigidDynamic>()->isSleeping() : false;
				
				for (PxU32 j = 0;j < nbShapes;j++)
				{
					const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
					const PxGeometryHolder containerGeometry = shapes[j]->getGeometry();
				
					if (shapes[j]->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

					// render object
					glPushMatrix();
					glMultMatrixf(&shapePose.column0.x);

					// When sleeping is undefined
					const PxVec3 darkColor = color * 0.25f;
					glColor4f(darkColor.x, darkColor.y, darkColor.z, 1.0f);

					/*
					if (sleeping)
					{
						const PxVec3 darkColor = color * 0.25f;
						glColor4f(darkColor.x, darkColor.y, darkColor.z, 1.0f);
					}
					else
					{
						glColor4f(color.x, color.y, color.z, 1.0f);
					}
					*/
					//renderGeometryHolder(containerGeometry);
					/*
					
					glPopMatrix();

					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

					glPushMatrix();
					glMultMatrixf(shadowMat);
					glMultMatrixf(&shapePose.column0.x);
					glDisable(GL_LIGHTING);
					glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
					renderGeometryHolder(h);
					glEnable(GL_LIGHTING);
					glPopMatrix();
					*/
					
				}
				
			}
			
		}

		////////////////////////// END IMPLEMENTING AND RENDERING PHYSX /////////////////////////////


		////////////////////////// ADDING SOME GEOMETRY WITH OPEN GL /////////////////////////////
		// Multi-colored side - FRONT
		glBegin(GL_POLYGON);

		glVertex3f(-0.5, -0.5, -0.5);       // P1
		glVertex3f(-0.5, 0.5, -0.5);       // P2
		glVertex3f(0.5, 0.5, -0.5);       // P3
		glVertex3f(0.5, -0.5, -0.5);       // P4

		glEnd();
		////////////////////////// END ADDING SOME GEOMETRY WITH OPEN GL /////////////////////////////

		SDL_GL_SwapWindow(window);

		SDL_Event current_event;
		while (SDL_PollEvent(&current_event) != 0)
			processEvents(current_event);
	}

	// Destroy window
	cleanupPhysics();
	if (glContext) SDL_GL_DeleteContext(glContext);
	if (window) SDL_DestroyWindow(window);

	// Quit SDL subsystems
	SDL_Quit();

	return 0;
}
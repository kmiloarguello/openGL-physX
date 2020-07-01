// ESIEE Paris - E4 Summer project 2020
// @filename: 1simpledrawing.cpp
// @description: This file contains all the instructions for the main setup of the project

#include "globals.h"
#include "Render.h"
//#include "Engine.h"
#include "ImageLoader.h"

using namespace std;
using namespace physx;

static PxVec3 gVertexBuffer[MAX_NUM_MESH_VEC3S];

// RENDER Variables
Render* render = nullptr;

// PhysX Variables
PxDefaultAllocator      gAllocator;
PxDefaultErrorCallback  gErrorCallback;

PxFoundation* gFoundation = NULL;
PxPvd* gPvd = NULL;
PxPhysics* gPhysics = NULL;
PxPhysics* mPhysics = NULL;
PxCooking* mCooking = NULL;

PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
PxMaterial* gMaterial = NULL;
PxMaterial* gMaterial2 = NULL;
PxRigidDynamic* ball = NULL;

vector<PxRigidActor*> boxes;
PxRigidDynamic* gKinematics;

// OBSTACLES
//Global variables to rotate the paddles
// Right paddle
PxRigidDynamic* paddleRight = NULL;
PxRigidStatic* wall6 = NULL;
PxRevoluteJoint* jointPaddleRight = NULL;
// Left paddle
PxRigidDynamic* paddleLeft = NULL;
PxRigidStatic* wall5 = NULL;
PxRevoluteJoint* jointPaddleLeft = NULL;


// Camera variables
glm::vec3 camera_eye = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);

static int shoulder = 0, elbow = 0;

int n = 20, m = 20;
float r = 1.0, alpha = 0.0, theta = 0.0, delta, h = 1.0;
float xp, yp, zp, puntos[100][100][3], ptosElipses[100][100][2];
float ry = 1.0;
float compZ = -1.5;
float sx = 1.0, sy = 1.0, sz = 1.0;
float tx = 0.0, ty = 0.0;

// Texture variables
unsigned int _id;


// INIT FUNCTIONS
void initPhysics();
void idleCallback();

// Temp
void stepPhysics();
void cleanupPhysics();

/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// RENDERING
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------

/// INITIALIZE OPENGL
void init()
{
    render = new Render();
    render->setupColors();
    render->setupLights();
    glutIdleFunc(idleCallback);
}

void setupTextures()
{
    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void loadTextures(const char* filename)
{
    ImageLoader im(filename);
    setupTextures();
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, im.iWidth, im.iHeight, GL_RGB, GL_UNSIGNED_BYTE,
                      im.textureData);

}

// RENDER Room
void renderRoom()
{

    glEnable(GL_TEXTURE_2D);

    // GROUND
    // Load the ground as QUADS and apply the textures given a defined vertices
    glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// DISPLAY ELEMENTS IN SCENE
void display()
{
    stepPhysics();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get the scene
    PxGetPhysics().getScenes(&gScene, 1);

    PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC |
                                         PxActorTypeFlag::eRIGID_STATIC);
    const PxVec3 color(1.0f, 0.0f, 0.0f);

    //glTranslatef(0.0f, -1.0f, 0.0f);
    //glScalef(sx, sy, sz);
    //glRotatef(90, 0, 1, 0);
    renderRoom();

    if (nbActors)
    {
        vector<PxRigidActor*> actors(nbActors);
        gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC,
                          reinterpret_cast<PxActor**>(&actors[0]), nbActors);
        render->renderActors(&actors[0], static_cast<PxU32>(actors.size()), true, color);

        if (ball->getGlobalPose().p[1] > 5)
        {
            ball->setGlobalPose(PxTransform(PxVec3(ball->getGlobalPose().p[0], 3,
                                                   ball->getGlobalPose().p[2]) ));
        }
    }

    glutSwapBuffers();
}

void idleCallback()
{
    glutPostRedisplay();
}

// SETUP PROJECTION
void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, (GLfloat)w / (GLfloat)h, 1.0, 500.0);
    gluLookAt(0.0, 200.0, -100.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -3.0);
}



/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// EVENTS
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------

/// Function to rotate the Paddles
// It applies a force to the update the pose
void triggerPaddle(PxRigidDynamic* paddle, PxRevoluteJoint* joint, float force) {
    if (paddle->isSleeping()) {
        paddle->wakeUp();
    }
    joint->setDriveVelocity(force);
    joint->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
}

void KeyPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'd': 
        // Rotating the right paddle
        triggerPaddle(paddleRight,jointPaddleRight,10.0f);
        break;
    case 'q':
        // Rotating the left paddle
        triggerPaddle(paddleLeft, jointPaddleLeft, 10.0f);
        break;

    default:
        break;
    }
}


void KeyRelease(unsigned char key, int x, int y)
{
    switch (key)
    {
        //implement your own
    case 'd':
        // Returning to the original position - right paddle
        triggerPaddle(paddleRight, jointPaddleRight, -10.0f);
        break;
    case 'q':
        // Returning to the original position - left paddle
        triggerPaddle(paddleLeft, jointPaddleLeft, -10.0f);
        break;
    default:
        break;
    }
}

/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// PHYSX
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------


PxRigidDynamic* createConvexHull(PxVec3* verts, PxU32 numVerts, PxVec3 position)
{

    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = numVerts;
    convexDesc.points.stride = sizeof(PxVec3);
    convexDesc.points.data = verts;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxDefaultMemoryOutputStream buf;
    PxConvexMeshCookingResult::Enum result;

    if (!mCooking->cookConvexMesh(convexDesc, buf, &result))
    {
        cout << "ERROR NULL" << endl;
    }

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    PxConvexMesh* convexMesh = gPhysics->createConvexMesh(input);
    PxRigidDynamic* aConvexActor = gPhysics->createRigidDynamic(PxTransform(position));
    PxShape* aConvexShape = PxRigidActorExt::createExclusiveShape(*aConvexActor,
                            PxConvexMeshGeometry(convexMesh), *gMaterial);

    return aConvexActor;
}


void initPhysics()
{
    cout << "Initializing PhysX" << endl;

    // Initialization of PhysX
    // The gFoundation instance initialize PhysX
    // This uses some callbacks for memory allocation and errors and also the ID version
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    // PVD is used for visual debugging in PhysX
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // COOKING INIT
    // This is helpful to work with ConvexHull geometries

    // 1. Define the scale
    PxTolerancesScale scale;
    scale.length = 100;
    scale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice

    //  cooking transforms the mesh data into a form which allows the SDK to perform efficient collision detection
    mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(scale));

    if (!mCooking)
    {
        cout << "PxCreateCooking failed!" << endl;
    }


    // Here the library initialize the Physics with some tolerance (this can be updated)
    // ? The tolerance is in animation, maybe gravity, etc
    // This PxCreatePhysics does not have Articulations nor Height fields
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

    // It is good to mention the simulation part and the time slots fetch()

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

    // For handling the Threads, PhysX uses a gDispatcher
    // GPU Optimization
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    // PhysX Works using Tasks, specially for simulation
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

    // The material allows us to define the friction for the objects
    // We define the friction for the two elements and the restitution
    // The friction for dynamic elements should be around .1f and the restitution 1.2f
    // This allows us to have a board with less friction between the ball and the ground
    gMaterial = gPhysics->createMaterial(0.0f, 0.1f, 1.2f);
    gMaterial2 = gPhysics->createMaterial(0.0f, 0.1f, 1.2f);

    // BASE -> Actor -> RigidBody
    // PxRigidStatic simulates a rigid body object
    // PxCreatePlane is method to create planes of equation a.x + b = 0
    // PxPlane Normal Vector - Distance to the origin (last parameter)
    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0.0f, 1.0f, 0.0f, 0.0f), *gMaterial2);
    gScene->addActor(*groundPlane);
    boxes.push_back(groundPlane);

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ----------------- WALLS ---------------------- //
    /////////////////////////////////////////////////////

    //ARRANGED IN A CLOCKWISE ORDER

    //length(x-axis, z-axis, y-axis)
    //position(x-axis, z-axis, y-axis)
    //TOP WALL
    PxShape* wall1Shape = gPhysics->createShape(PxBoxGeometry( 102.0f, 10.0f, 2.0f), *gMaterial);
    PxRigidStatic* wall1 = gPhysics->createRigidStatic(PxTransform(PxVec3( 0.0f, 10.0f, 100.0f )));
    wall1->attachShape(*wall1Shape);
    gScene->addActor(*wall1);

    //RIGHT WALL
    PxShape* wall4Shape = gPhysics->createShape(PxBoxGeometry(2.0f, 10.0f, 102.0f), *gMaterial);
    PxRigidStatic* wall4 = gPhysics->createRigidStatic(PxTransform(PxVec3(-100.0f, 10.0f, 0.0f)));
    wall4->attachShape(*wall4Shape);
    gScene->addActor(*wall4);

    //BOTTOM WALL
    PxShape* wall2Shape = gPhysics->createShape(PxBoxGeometry(102.0f, 10.0f, 2.0f), *gMaterial);
    PxRigidStatic* wall2 = gPhysics->createRigidStatic(PxTransform(PxVec3(0.0f, 10.0f, -100.0f)));
    wall2->attachShape(*wall2Shape);
    gScene->addActor(*wall2);

    //LEFT WALL
    PxShape* wall3Shape = gPhysics->createShape(PxBoxGeometry(2.0f, 10.0f, 102.0f), *gMaterial);
    PxRigidStatic* wall3 = gPhysics->createRigidStatic(PxTransform(PxVec3(100.0f, 10.0f, 0.0f)));
    wall3->attachShape(*wall3Shape);
    gScene->addActor(*wall3);


    //LEFT PADDLE WALL
    PxShape* wall5Shape = gPhysics->createShape(PxBoxGeometry(21.0f, 10.0f, 2.0f), *gMaterial);
    PxRigidStatic* wall5 = gPhysics->createRigidStatic(PxTransform(PxVec3(80.0f, 10.0f, -73.0f)));
    wall5->attachShape(*wall5Shape);
    gScene->addActor(*wall5);

    //RIGHT PADDLE WALL
    PxShape* wall6Shape = gPhysics->createShape(PxBoxGeometry(21.0f, 10.0f, 2.0f), *gMaterial);
    wall6 = gPhysics->createRigidStatic(PxTransform(PxVec3(-80.0f, 10.0f, -73.0f)));
    wall6->attachShape(*wall6Shape);
    gScene->addActor(*wall6);

    /////////////////////////////////////////////////////
    /// ----------------- END WALLS ------------------ //
    /////////////////////////////////////////////////////
    
    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ------------------ BORDERS ------------------- //
    /////////////////////////////////////////////////////

    PxVec3 meshBorder1[] = {
        
        // TOP
        PxVec3(10,0,0),
        PxVec3(16,0,-2),
        PxVec3(20,0,-10),
        PxVec3(20,0,-20),
        PxVec3(14,0,-32),
        PxVec3(12,0,-32),
        PxVec3(10,0,-22),
        PxVec3(0,0,-18),
        PxVec3(0,0,-10),        
        
        // BOTTOM
        PxVec3(10,20,0),
        PxVec3(16,20,-2),
        PxVec3(20,20,-10),
        PxVec3(20,20,-20),
        PxVec3(14,20,-32),
        PxVec3(12,20,-32),
        PxVec3(10,20,-22),
        PxVec3(0,20,-18),
        PxVec3(0,20,-10)
    };

    PxRigidDynamic* Border1 = createConvexHull(meshBorder1, 18, PxVec3(50.f, 0.0f, 60.0f));
    Border1->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gScene->addActor(*Border1);


    PxVec3 meshBorder2[] = {
        
        //TOP
        PxVec3(98,0,-2),
        PxVec3(98,0,-62),
        PxVec3(78,0,-52),
        PxVec3(78,0,-22),

        //BOTTOM
        PxVec3(78,20,-22),
        PxVec3(78,20,-52),
        PxVec3(98,20,-62),
        PxVec3(98,20,-2),
        
    };

    PxRigidDynamic* Border2 = createConvexHull(meshBorder2, 8, PxVec3(0.0f, 0.0f, 25.f));
    Border2->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gScene->addActor(*Border2);

    /////////////////////////////////////////////////////
    /// ------------------ BORDERS ------------------- //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ----------- OBSTACLES  ---------------------- //
    /////////////////////////////////////////////////////

    // Obstacle 1
    PxRigidDynamic* obstacle1 = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(0.0f, 10.0f, 70.0f)),
                                                PxBoxGeometry(10.0f, 10.0f, 10.0f), *gMaterial, 1.0f);
    obstacle1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    obstacle1->setMass(0.f);
    obstacle1->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 10.f));
    gScene->addActor(*obstacle1);

    // Obstacle 2
    PxVec3 convexVerts[] =
    {
        PxVec3(0, 30, 0),
        PxVec3(30, 0, 0),
        PxVec3(-30, 0, 0),
        PxVec3(0, 0, 30),
        PxVec3(0, 0, -30)
    };

    PxRigidDynamic* obstacle2 = createConvexHull(convexVerts, 5, PxVec3(-50.f, 0.1f, 0.f) );
    //obstacle2->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    // obstacle2->setMass(0.f);
    // obstacle2->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 10.f));
    obstacle2->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gScene->addActor(*obstacle2);



    // Obstacle 3
    /*  PxVec3 convexVerts2[] = {
        PxVec3(0,20,0),
        PxVec3(10,0,0),
        PxVec3(-10,0,0),
        PxVec3(0,0,10),
        PxVec3(0,0,-10)
        };
        PxRigidDynamic* obstacle3 = render->createAConvexHull(*gPhysics, convexVerts2, 5, PxVec3(-50.0f, 5.0f, 0.0f), *gMaterial);
        gScene->addActor(*obstacle3);

        PxRigidDynamic* obstacle2 = createConvexHull(convexVerts, 5, PxVec3(-50.f,0.1f,0.f) );
        obstacle2->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
        obstacle2->setMass(0.f);
        obstacle2->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 10.f));
        gScene->addActor(*obstacle2);
    */

    /*float i;
    for (i = 30; i > 3; i--)
    {
        PxRigidDynamic* obstacle4 = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(i+66, 10.0f, 98.0f - i)), PxBoxGeometry(2.0f, 10.0f, i), *gMaterial, 1.0f);
        obstacle4->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
        obstacle4->setMass(0.f);
        obstacle4->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 10.f));
        gScene->addActor(*obstacle4);
    }*/
    
    /////////////////////////////////////////////////////
    /// ----------- END OBSTACLES  ------------------- //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ---------------- PADDLES  -------------------- //
    /////////////////////////////////////////////////////

    // Paddle - Left
    PxVec3 meshPaddleLeft[] =
    {
        //VERTEXES ARE IN CLOCKSWISE ORDER STARTING FROM THE BACK TO THE FRONT

        // BACK-SIDE
        // BACK LEFT TOP
        PxVec3(50, 10, 6),
        // BACK RIGHT TOP
        PxVec3(0, 10, 10),
        // BACK RIGHT BOTTOM
        PxVec3(0, 0, 10),
        // BACK LEFT BOTTOM
        PxVec3(50, 0, 6),

        // FRONT-SIDE   --connecting the back to the front here
        // FRONT LEFT BOTTOM
        PxVec3(50, 0, 4),
        // FRONT LEFT TOP
        PxVec3(50, 10, 4),
        // FRONT RIGHT TOP
        PxVec3(0, 10, 0),
        // FRONT RIGHT BOTTOM
        PxVec3(0, 0, 0)
    };

    //position(x-axis, z-axis, y-axis)
    paddleLeft = createConvexHull(meshPaddleLeft, 8, PxVec3( 60.f, 0.1f, -65.f));
    paddleLeft->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    paddleLeft->setMass(0.f);
    // The global pose for right and left paddle are basically the same (check the vertices)
    // However to create the mirror effect, ther right paddle is multiplied by PI to switch the direction
    // This allows us to create the effect of paddle rotation on right and left side
    paddleLeft->setGlobalPose( PxTransform( paddleLeft->getGlobalPose().p , paddleLeft->getGlobalPose().q * PxQuat(PxPi, PxVec3(0, -1, 0)))  );
    gScene->addActor(*paddleLeft);

    //---------- translation------------
  
    // The value for the rotation is PxPi (Positive)
    jointPaddleLeft = PxRevoluteJointCreate(
        *gPhysics,
        wall5,
        PxTransform(
            paddleLeft->getGlobalPose().p,
            paddleLeft->getGlobalPose().q * PxQuat(PxPi / 2, PxVec3(0, 0, 1)) ),
        paddleLeft,
        PxTransform(PxVec3(0, 0, 0), PxQuat(PxPi / 2, PxVec3(0, 0, 1)))
    );

    jointPaddleLeft->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
    jointPaddleLeft->setLimit(PxJointAngularLimitPair(-PxPi / 16, PxPi / 6));
    jointPaddleLeft->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);

    //----------end translation------------

    // Paddle - Right
    PxVec3 meshPaddleRight[] =
    {
        //VERTEXES ARE IN CLOCKSWISE ORDER STARTING FROM THE BACK TO THE FRONT

        // BACK-SIDE
        // BACK LEFT TOP
        PxVec3(50, 10, 6),
        // BACK RIGHT TOP
        PxVec3(0, 10, 10),
        // BACK RIGHT BOTTOM
        PxVec3(0, 0, 10),
        // BACK LEFT BOTTOM
        PxVec3(50, 0, 6),

        // FRONT-SIDE   --connecting the back to the front here
        // FRONT LEFT BOTTOM
        PxVec3(50, 0, 4),
        // FRONT LEFT TOP
        PxVec3(50, 10, 4),
        // FRONT RIGHT TOP
        PxVec3(0, 10, 0),
        // FRONT RIGHT BOTTOM
        PxVec3(0, 0, 0)
    };

    paddleRight = createConvexHull(meshPaddleRight, 8, PxVec3(-60.f, .1f, -75.f));
    paddleRight->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    paddleRight->setMass(0.f);
    gScene->addActor(*paddleRight);

    //---------- translation------------

    // The way how the paddle rotates is by using a joint i.e an shoulder
    // There is a fixed object (right wall) and a dynamic object (paddle)
    // It must be use create a revolute function
    // Also, it is necessary to set the constraints for visualization and limits
    // This joint is attached to the KeyPress() function event to rotate
    // The rotation axis is defined on PxVec3 inside of PxQuad

    jointPaddleRight = PxRevoluteJointCreate(
        *gPhysics,
        wall6,
        PxTransform(
            paddleRight->getGlobalPose().p,
            paddleRight->getGlobalPose().q * PxQuat(-PxPi / 2, PxVec3(0, 0, 1))),
        paddleRight,
        PxTransform(PxVec3(0, 0, 0), PxQuat(-PxPi / 2, PxVec3(0, 0, 1)))
    );

    jointPaddleRight->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
    jointPaddleRight->setLimit(PxJointAngularLimitPair(-PxPi/ 16, PxPi / 6));
    jointPaddleRight->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);


    //----------end translation------------

    /////////////////////////////////////////////////////
    /// -------------- END PADDLES  ------------------ //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ----------------- BALL  ---------------------- //
    /////////////////////////////////////////////////////

    ball = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(0.0f, 0.0f, 0.0f)), PxSphereGeometry(3.0),
                           *gMaterial, 1.0f);
    ball->setLinearDamping(0.005f);
    ball->setLinearVelocity(PxVec3(-40, 0, 80));
    //ball->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, .5f));
    //ball->addForce(PxVec3(0.f,1000.f,0.f), PxForceMode::eACCELERATION);
    ball->setMass(1000.f);
    gScene->addActor(*ball);

    /////////////////////////////////////////////////////
    /// ----------------- BALL  ---------------------- //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ---------------- MOVINGBAR ------------------ ///
    /////////////////////////////////////////////////////

    const PxQuat rot = PxQuat(PxIdentity);

    PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(3.0f, 3.0f), *gMaterial);

    PxTransform pose(PxVec3(0, 2.0f, -30.0f), rot);
    PxRigidDynamic* bar = gPhysics->createRigidDynamic(pose);
    bar->attachShape(*shape);
    gScene->addActor(*bar);
    bar->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gKinematics = bar;

    /////////////////////////////////////////////////////
    /// ---------------- MOVINGBAR ------------------ ///
    /////////////////////////////////////////////////////

}

void updateKinematics(PxReal timeStep)
{
    PxTransform motion;
    motion.q = PxQuat(PxIdentity);

    static float gTime = 0.0f;
    gTime += timeStep;

    //Motion on X axis
    //sinf(Period = velocity) * Amplitude;
    const float xf = sinf(gTime * 0.5f) * 9.0f;
    motion.p = PxVec3(xf, 2.0f, -30.0f);

    PxRigidDynamic* kine = gKinematics;
    kine->setKinematicTarget(motion);

}

void stepPhysics()
{
    const PxReal timeStep = 1.0f / 60.0f;

    updateKinematics(timeStep);

    for (int i = 0; i < 10; i++)
    {
        gScene->simulate(1.0f / 200.0f);
        gScene->fetchResults(true);
    }
}

void cleanupPhysics()
{
    gPhysics->release();
    gFoundation->release();
    printf("SnippetHelloWorld done.\n");
}



/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// MAIN Function
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    // INIT LIGHTS AND COLORS
    init();

    // TODO: Improve the textures - Add one texture for each object
    loadTextures("./assets/images/scenary.bmp");

    //loadTextures("./assets/images/scenary.bmp");

    glutDisplayFunc(display);

    // Initialize PhysX
    initPhysics();

    glutReshapeFunc(reshape);
    glutKeyboardFunc(KeyPress);
    glutKeyboardUpFunc(KeyRelease);
    glutMainLoop();
    return 0;
}
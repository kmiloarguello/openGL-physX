// ESIEE Paris - E4 Summer project 2020
// @filename: 1simpledrawing.cpp
// @description: This file contains all the instructions for the main setup of the project

#include "globals.h"
#include "Render.h"
#include "ImageLoader.h"
#include <sstream>

using namespace std;
using namespace physx;
using namespace Render;

static PxVec3 gVertexBuffer[MAX_NUM_MESH_VEC3S];

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
PxMaterial* gMaterialPaddles = NULL;
PxRigidDynamic* ball = NULL;

vector<PxRigidActor*> boxes;
PxRigidDynamic* gKinematics;

// OBSTACLES
//Global variables to rotate the paddles
// Right paddle
PxRigidDynamic* paddleRight2 = NULL;
PxRigidStatic* wall6 = NULL;
PxRevoluteJoint* jointPaddleRight = NULL;
// Left paddle
PxRigidDynamic* paddleLeft2 = NULL;
PxRigidStatic* wall5 = NULL;
PxRevoluteJoint* jointPaddleLeft = NULL;

// On collision variable
PxSimulationEventCallback* events = NULL;

PxRigidDynamic* gPlunger = NULL;
Render::Camera* sCamera;


// Camera variables
glm::vec3 camera_eye = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);

int isLaunched = 0;

// Texture variables
unsigned int _id;


// User score

int globalScore = 0;

enum GameState {
    Init,
    Menu,
    Game,
    GameOver,
    Paused
} gameState;

// INIT FUNCTIONS
void initPhysics();
void idleCallback();

// Temp
void stepPhysics();
void cleanupPhysics();
void createANewBall();

/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// RENDERING
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------

/// INITIALIZE OPENGL
void init()
{
   // render = new Render();
    Render::setupColors();
    Render::setupLights();
    glutIdleFunc(idleCallback);

    gameState = GameState::Init;

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

void motionCallback(int x, int y)
{
    sCamera->handleMotion(x, y);
}

void mouseCallback(int button, int state, int x, int y)
{
    sCamera->handleMouse(button, state, x, y);
}

void displayText(float x, float y, int r, int g, int b, const char* string) {
    int j = strlen(string);

    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (int i = 0; i < j; i++) {

        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);
    }
}

string int_to_str(int x) {
    stringstream ss;
    ss << x;
    return ss.str();
}

void renderScore(int x) {

    string str = "Your score: " + int_to_str(x);
    const char* cstr = str.c_str();

    displayText(60.0, 90.0, 1, 1, 1, cstr);
}

void renderInstruction() {

    string str = "Press SPACE BAR to start.";
    const char* cstr = str.c_str();

    displayText(60.0, 90.0, 1, 1, 1, cstr);
}

void renderGameOver() {

    string str = "GAME OVER";
    const char* cstr = str.c_str();

    displayText(30.0, 0.0, 1, 0, 0, cstr);
}


void updateGameState() {
    if (gameState == GameState::Game) {
        renderScore(globalScore++);
    }

    else if (gameState == GameState::Init) {
        renderInstruction();
    }
    else if (gameState == GameState::GameOver) {
        renderGameOver();
        renderScore(globalScore);
        gScene->removeActor(*ball);
        ball = NULL;
        createANewBall();
    }
}

// DISPLAY ELEMENTS IN SCENE
void display()
{

    stepPhysics();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get the scene
    PxGetPhysics().getScenes(&gScene, 1);
    //Render::startRender(sCamera->getEye(), sCamera->getDir(), 1.0f, 1.0000f);

    PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC |
                                         PxActorTypeFlag::eRIGID_STATIC);
    const PxVec3 color(1.0f, 0.0f, 0.4f);

    renderRoom();

    if (nbActors)
    {

        updateGameState();

        vector<PxRigidActor*> actors(nbActors);
        gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC,
                          reinterpret_cast<PxActor**>(&actors[0]), nbActors);
        Render::renderActors(&actors[0], static_cast<PxU32>(actors.size()), ALLOW_SHADOWS, color);

        // To indicate PhysX that the ball always must be below 5 units, 
        // Prevent that the ball flies :v
        if (ball->getGlobalPose().p[1] > 5)
        {
            ball->setGlobalPose(PxTransform(PxVec3(ball->getGlobalPose().p[0], 3,
                                                   ball->getGlobalPose().p[2]) ));
        }
        //if (isLaunched == 1) {
        //    ball->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
        //}

        // Set a constant force to the ball in order to aim its direction towards the paddles
        // This has to be made for each frame

        ball->addForce(PxVec3(0.0f,0.0f,-20.0f), PxForceMode::eACCELERATION);

        if (ball->getGlobalPose().p[2] < -90.f) {
            gameState = GameState::GameOver;
            cout << "POS: " << ball->getGlobalPose().p[2] << endl;
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

/*
 * menu function
 * key - integer representing the chosen action
 */
void menu(int key) {

    switch (key) {
    case 'A':
    case 'a':
        exit(0);
    }
}



/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// EVENTS
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------

/// Function to rotate the Paddles
// It applies a force to the update the pose
void triggerPaddle(PxRigidDynamic* paddle, PxRevoluteJoint* joint, float force)
{
    if (paddle->isSleeping())
    {
        paddle->wakeUp();
    }

    joint->setDriveVelocity(force);
    joint->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
}

void triggerPlunger(PxRigidDynamic* plunger, PxVec3 position)
{

    if (plunger->isSleeping())
    {
        plunger->wakeUp();
    }

    plunger->setKinematicTarget(PxTransform(position, PxQuat(PxHalfPi, PxVec3(0, 1, 0))));

}


void KeyPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'd':
        // Rotating the right paddle
        triggerPaddle(paddleRight2, jointPaddleRight, 10.0f);
        break;

    case 'q':
        // Rotating the left paddle
        triggerPaddle(paddleLeft2, jointPaddleLeft, 10.0f);
        break;

    case ' ':
        // Pulling the plunger
        triggerPlunger(gPlunger, PxVec3(-87.0f, 5.0f, -50.0f));
        gameState = GameState::Game;
        globalScore = 0;
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
        triggerPaddle(paddleRight2, jointPaddleRight, -10.0f);
        break;

    case 'q':
        // Returning to the original position - left paddle
        triggerPaddle(paddleLeft2, jointPaddleLeft, -10.0f);
        break;

    case ' ':
        // Returning the plunger
        triggerPlunger(gPlunger, PxVec3(-87.0f, 5.0f, -28.0f));
        isLaunched = 1;
        //ball->setLinearVelocity(PxVec3(0.0f, 0.0f, -30.0f), true);
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
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, -3.0f);

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

    gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
    gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
    gScene->setVisualizationParameter(PxVisualizationParameter::eBODY_LIN_VELOCITY, 1.0f);
    gScene->setVisualizationParameter(PxVisualizationParameter::eBODY_AXES, 1.0f);
    gScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
    gScene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);

    // The material allows us to define the friction for the objects
    // We define the friction for the two elements and the restitution
    // The friction for dynamic elements should be around .1f and the restitution 1.2f
    // This allows us to have a board with less friction between the ball and the ground
    gMaterial = gPhysics->createMaterial(0.0f, 0.1f, 1.2f);
    gMaterial2 = gPhysics->createMaterial(0.0f, 0.5f, 1.4f);
    gMaterialPaddles = gPhysics->createMaterial(0.0f,0.1f,1.1f);

    // BASE -> Actor -> RigidBody
    // PxRigidStatic simulates a rigid body object
    // PxCreatePlane is method to create planes of equation a.x + b = 0
    // PxPlane Normal Vector - Distance to the origin (last parameter)
    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0.0f, 1.0f, 0.0f, 0.0f), *gMaterial);
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

    // LEFT DIAGONAL WALL
    PxShape* wall4_5Shape = gPhysics->createShape(PxBoxGeometry(20.0f, 10.0f, 2.0f), *gMaterial);
    PxRigidStatic* wall4_5 = gPhysics->createRigidStatic(PxTransform(PxVec3(90.0f, 10.0f, -55.0f)));
    wall4_5->attachShape(*wall4_5Shape);
    gScene->addActor(*wall4_5);
    wall4_5->setGlobalPose(PxTransform( wall4_5->getGlobalPose().p , PxQuat(-45.0, PxVec3(0,1,0) )));

    //LEFT PADDLE WALL
    PxShape* wall5Shape = gPhysics->createShape(PxBoxGeometry(21.0f, 10.0f, 2.0f), *gMaterial);
    PxRigidStatic* wall5 = gPhysics->createRigidStatic(PxTransform(PxVec3(80.0f, 10.0f, -73.0f)));
    wall5->attachShape(*wall5Shape);
    gScene->addActor(*wall5);

    // RIGHT DIAGONAL WALL
    PxShape* wall5_6Shape = gPhysics->createShape(PxBoxGeometry(20.0f, 10.0f, 2.0f), *gMaterial);
    PxRigidStatic* wall5_6 = gPhysics->createRigidStatic(PxTransform(PxVec3(-80.0f, 10.0f, -65.0f)));
    wall5_6->attachShape(*wall5_6Shape);
    gScene->addActor(*wall5_6);
    wall5_6->setGlobalPose(PxTransform(wall5_6->getGlobalPose().p, PxQuat(60.0, PxVec3(0, 1, 0))));

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

    PxVec3 meshBorder1[] =
    {

        // TOP
        PxVec3(10, 0, 0),
        PxVec3(16, 0, -2),
        PxVec3(20, 0, -10),
        PxVec3(20, 0, -20),
        PxVec3(14, 0, -32),
        PxVec3(12, 0, -32),
        PxVec3(10, 0, -22),
        PxVec3(0, 0, -18),
        PxVec3(0, 0, -10),

        // BOTTOM
        PxVec3(10, 20, 0),
        PxVec3(16, 20, -2),
        PxVec3(20, 20, -10),
        PxVec3(20, 20, -20),
        PxVec3(14, 20, -32),
        PxVec3(12, 20, -32),
        PxVec3(10, 20, -22),
        PxVec3(0, 20, -18),
        PxVec3(0, 20, -10)
    };

    PxRigidDynamic* Border1 = createConvexHull(meshBorder1, 18, PxVec3(50.f, 0.0f, 60.0f));
    Border1->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gScene->addActor(*Border1);


    PxVec3 meshBorder2[] =
    {

        //TOP
        PxVec3(98, 0, -2),
        PxVec3(98, 0, -62),
        PxVec3(78, 0, -52),
        PxVec3(78, 0, -22),

        //BOTTOM
        PxVec3(78, 20, -22),
        PxVec3(78, 20, -52),
        PxVec3(98, 20, -62),
        PxVec3(98, 20, -2),

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

    PxRigidDynamic* obstacle2 = createConvexHull(convexVerts, 5, PxVec3(-50.f, 0.1f, 0.f));
    obstacle2->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gScene->addActor(*obstacle2);

    // Obstacle 2
    PxVec3 convexVerts5[] =
    {
        PxVec3(0, 0, 0),
        PxVec3(20, 0, 0),
        PxVec3(0, 0, -20),
        PxVec3(0, 5, 0),
        PxVec3(20, 5, 0),
        PxVec3(0, 5, -20)
    };

    PxRigidDynamic* obstacle5 = createConvexHull(convexVerts5, 6, PxVec3(-100.f, 0.0f, 100.0f));
    obstacle5->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gScene->addActor(*obstacle5);

    /////////////////////////////////////////////////////
    /// ----------- END OBSTACLES  ------------------- //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ---------------- PADDLES  -------------------- //
    /////////////////////////////////////////////////////

    // Paddle - Left
    paddleLeft2 = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(-43.f, .1f, 0.f)), PxBoxGeometry(25.0f, 10.0f, 3.0f), *gMaterialPaddles, 1.0f);

    // The global pose for right and left paddle are basically the same (check the vertices)
    // However to create the mirror effect, ther right paddle is multiplied by PI to switch the direction
    // This allows us to create the effect of paddle rotation on right and left side
    paddleLeft2->setGlobalPose(
        PxTransform(
            paddleLeft2->getGlobalPose().p,
            paddleLeft2->getGlobalPose().q * PxQuat(PxPi, PxVec3(0, 1, 0))
        ));

    gScene->addActor(*paddleLeft2);

    //---------- translation------------

    // The value for the rotation is PxHalfPi (Positive)
    jointPaddleLeft = PxRevoluteJointCreate(
                          *gPhysics,
                          wall5,
                          PxTransform(
                              paddleLeft2->getGlobalPose().p,
                              PxQuat(PxHalfPi, PxVec3(0, 0, 1)) ),
                          paddleLeft2,
                          PxTransform(PxVec3(0, 0, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1)))
                      );

    jointPaddleLeft->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
    jointPaddleLeft->setLimit(PxJointAngularLimitPair(-PxPi / 16, PxPi / 6));
    jointPaddleLeft->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);

    //----------end translation------------

    // Paddle - Right
    paddleRight2 = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(45.f, .1f, -5.f)),  PxBoxGeometry(25.0f, 10.0f, 3.0f), *gMaterialPaddles, 1.0f);

    gScene->addActor(*paddleRight2);

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
                               paddleRight2->getGlobalPose().p,
                               paddleRight2->getGlobalPose().q * PxQuat(-PxHalfPi, PxVec3(0, 0, 1))),
                           paddleRight2,
                           PxTransform(PxVec3(0, 0, -5), PxQuat(-PxHalfPi, PxVec3(0, 0, 1)))
                       );

    jointPaddleRight->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
    jointPaddleRight->setLimit(PxJointAngularLimitPair(-PxPi / 20, PxPi / 6));
    jointPaddleRight->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);


    //----------end translation------------

    /////////////////////////////////////////////////////
    /// -------------- END PADDLES  ------------------ //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ----------------- BALL  ---------------------- //
    /////////////////////////////////////////////////////

    PxMaterial* ballMaterial = NULL;

    ballMaterial = gPhysics->createMaterial(0.5f, 0.2f, 0.597);

    ball = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(-87.0f, 0.0f, -7.0f)), PxSphereGeometry(3.0f),
                           *ballMaterial, 1.0f);
    //ball->setLinearDamping(0.05f);
    ball->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 10.0f));
    ball->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
    ball->setMass(50.f);
    gScene->addActor(*ball);


    /////////////////////////////////////////////////////
    /// ----------------- BALL  ---------------------- //
    /////////////////////////////////////////////////////


    // ------------------------------------------------------------------------------------------------
    /////////////////////////////////////////////////////
    /// ----------------- PLUNGER ------------------- ///
    /////////////////////////////////////////////////////

    const PxQuat rot = PxQuat(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));

    PxShape* shape = gPhysics->createShape(PxCapsuleGeometry(5.0f, 15.0f), *gMaterial);

    PxTransform pose(PxVec3(-87.0f, 5.0f, -30.0f), rot);
    PxRigidDynamic* plunger = gPhysics->createRigidDynamic(pose);
    plunger->attachShape(*shape);
    gScene->addActor(*plunger);
    plunger->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gPlunger = plunger;

    /////////////////////////////////////////////////////
    /// ----------------- PLUNGER ------------------- ///
    /////////////////////////////////////////////////////
    // ------------------------------------------------------------------------------------------------


    /////////////////////////////////////////////////////
    /// ---------------- MOVINGBAR ------------------ ///
    /////////////////////////////////////////////////////


    PxRigidDynamic* bar = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(10.0f, 10.0f, 30.0f)),
                                          PxCapsuleGeometry(3.0f, 3.0f), *gMaterial, 1.0f);
    gScene->addActor(*bar);
    bar->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gKinematics = bar;

    /////////////////////////////////////////////////////
    /// ---------------- MOVINGBAR ------------------ ///
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------


}

void createANewBall() {
    PxMaterial* ballMaterial = NULL;

    ballMaterial = gPhysics->createMaterial(0.5f, 0.2f, 0.597);

    ball = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(-87.0f, 0.0f, -7.0f)), PxSphereGeometry(3.0f),
        *ballMaterial, 1.0f);
    ball->setLinearDamping(0.005f);
    ball->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, .5f));
    ball->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
    ball->setMass(10.f);
    gScene->addActor(*ball);
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
    printf("Pinball done.\n");
}



/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// MAIN Function
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
int main(int argc, char** argv)
{
    Render::Camera* sCamera = new Render::Camera(PxVec3(50.0f, 50.0f, 50.0f), PxVec3(-0.6f, -0.2f, -0.7f));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    // INIT LIGHTS AND COLORS
    init();

    // TODO: Improve the textures - Add one texture for each object
    loadTextures("./assets/images/synth_wave.bmp");

    //loadTextures("./assets/images/more_synth.bmp");
    //loadTextures("./assets/images/cyer_scraper.bmp");
    //loadTextures("./assets/images/digi_punk.bmp");
    //loadTextures("./assets/images/purple_liquid.bmp");
    //loadTextures("./assets/images/scenary.bmp");

    glutDisplayFunc(display);

    // Initialize PhysX
    initPhysics();
    //glutMouseFunc(mouseCallback);
    //glutMotionFunc(motionCallback);

    glutReshapeFunc(reshape);
    glutKeyboardFunc(KeyPress);
    glutKeyboardUpFunc(KeyRelease);
    //motionCallback(0, 0);

    // creates the main menu (right button)
    glutCreateMenu(menu);
    glutAddMenuEntry("Quit", 'q');
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();

    return 0;
}
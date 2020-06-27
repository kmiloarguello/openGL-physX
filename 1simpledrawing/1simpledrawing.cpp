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

vector<PxRigidActor*> boxes;

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
        std::vector<PxRigidActor*> actors(nbActors);
        gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC,
                          reinterpret_cast<PxActor**>(&actors[0]), nbActors);
        render->renderActors(&actors[0], static_cast<PxU32>(actors.size()), true, color);
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
    gluLookAt(0.0,200.0, -100.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -3.0);
}

/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// EVENTS
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 's':   /* escalado  */
        sx = sx + 0.1;
        sy = sy + 0.1;
        sz = sz + 0.1;
        glutPostRedisplay();
        break;

    case 'S':  /* escalado  */
        sx = sx - 0.1;
        sy = sy - 0.1;
        sz = sz - 0.1;
        glutPostRedisplay();
        break;

    case 'e':  /* rotacion  */
        elbow = (elbow + 5) % 360;
        glutPostRedisplay();
        break;

    case 'E':  /* rotacion  */
        elbow = (elbow - 5) % 360;
        glutPostRedisplay();
        break;

    case 't':  /*  translacion en x  */
        tx = tx + 0.1;
        glutPostRedisplay();
        break;

    case 'T':  /*  translacion  en x */
        tx = tx - 0.1;
        glutPostRedisplay();
        break;

    case 'y':  /*  translacion  en y  */
        ty = ty + 0.1;
        glutPostRedisplay();
        break;

    case 'Y':  /*  translacion  en y  */
        ty = ty - 0.1;
        glutPostRedisplay();
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

    gMaterial = gPhysics->createMaterial(0.0f, 0.0f, 1.0f);
    gMaterial2 = gPhysics->createMaterial(20.0f, 0.1f, 1.0f);

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
    PxShape* wall1Shape = gPhysics->createShape(PxBoxGeometry( 100.0f, 10.0f, 2.0f), *gMaterial);
    PxRigidStatic* wall1 = gPhysics->createRigidStatic(PxTransform(PxVec3( 0.0f, 10.0f, 100.0f )));
    wall1->attachShape(*wall1Shape);
    gScene->addActor(*wall1);

    PxShape* wall2Shape = gPhysics->createShape(PxBoxGeometry(100.0f, 10.0f, 2.0f), *gMaterial);
    PxRigidStatic* wall2 = gPhysics->createRigidStatic(PxTransform(PxVec3(0.0f, 10.0f, -100.0f)));
    wall2->attachShape(*wall2Shape);
    gScene->addActor(*wall2);

    PxShape* wall3Shape = gPhysics->createShape(PxBoxGeometry(2.0f, 10.0f, 100.0f), *gMaterial);
    PxRigidStatic* wall3 = gPhysics->createRigidStatic(PxTransform(PxVec3(100.0f, 10.0f, 0.0f)));
    wall3->attachShape(*wall3Shape);
    gScene->addActor(*wall3);

    PxShape* wall4Shape = gPhysics->createShape(PxBoxGeometry(2.0f, 10.0f, 100.0f), *gMaterial);
    PxRigidStatic* wall4 = gPhysics->createRigidStatic(PxTransform(PxVec3(-100.0f, 10.0f, 0.0f)));
    wall4->attachShape(*wall4Shape);
    gScene->addActor(*wall4);
    /////////////////////////////////////////////////////
    /// ----------------- END WALLS ------------------ //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ----------- OBSTACLES  ---------------------- //
    /////////////////////////////////////////////////////

    // Obstacle 1
    PxRigidDynamic* obstacle1 = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(0.0f, 10.0f, 70.0f)), PxBoxGeometry(10.0f, 10.0f, 10.0f), *gMaterial, 1.0f);
    //obstacle1->addForce(PxVec3(0.0f, 0.0f, 5.0f), PxForceMode::eIMPULSE);
    obstacle1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    obstacle1->setMass(0.f);
    obstacle1->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 10.f));
    gScene->addActor(*obstacle1);


    // Obstacle 2
    PxVec3 convexVerts[] = {
        PxVec3(0,30,0),
        PxVec3(30,0,0),
        PxVec3(-30,0,0),
        PxVec3(0,0,30),
        PxVec3(0,0,-30)
    };

 
    //PxRigidDynamic* obstacle2 = render->createConvexMesh(convexVerts, 5, *gPhysics, PxVec3(-50.0f, 5.0f, 0.0f), *gMaterial2);
    
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = 5;
    convexDesc.points.stride = sizeof(PxVec3);
    convexDesc.points.data = convexVerts;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxDefaultMemoryOutputStream buf;
    PxConvexMeshCookingResult::Enum result;

    if (!mCooking->cookConvexMesh(convexDesc, buf, &result)) {
        cout << "ERROR NULL" << endl;
    }

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    PxConvexMesh* convexMesh = gPhysics->createConvexMesh(input);
    PxRigidDynamic* aConvexActor = gPhysics->createRigidDynamic(PxTransform(PxVec3(-50.0f, .1f, 0.0f)));
    PxShape* aConvexShape = PxRigidActorExt::createExclusiveShape(*aConvexActor, PxConvexMeshGeometry(convexMesh), *gMaterial);
    aConvexActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    aConvexActor->setMass(0.f);
    aConvexActor->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 10.f));
    gScene->addActor(*aConvexActor);
    
    
    //gScene->addActor(*obstacle2);

    // Obstacle 3
    /*PxVec3 convexVerts2[] = {
        PxVec3(0,20,0),
        PxVec3(10,0,0),
        PxVec3(-10,0,0),
        PxVec3(0,0,10),
        PxVec3(0,0,-10)
    };
    PxRigidDynamic* obstacle3 = render->createAConvexHull(*gPhysics, convexVerts2, 5, PxVec3(-50.0f, 5.0f, 0.0f), *gMaterial);
    gScene->addActor(*obstacle3);
    */
    /////////////////////////////////////////////////////
    /// ----------- END OBSTACLES  ------------------- //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------
    /////////////////////////////////////////////////////
    /// ----------------- BALL  ---------------------- //
    /////////////////////////////////////////////////////

    PxRigidDynamic* ball = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(0.0f, 5.0f, 0.0f)), PxSphereGeometry(3.0), *gMaterial, 1.0f);
    ball->setLinearDamping(0.05f);
    ball->setLinearVelocity(PxVec3(-40, 0, 80));
    gScene->addActor(*ball);
  
    /////////////////////////////////////////////////////
    /// ----------------- BALL  ---------------------- //
    /////////////////////////////////////////////////////

}

void stepPhysics()
{
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
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
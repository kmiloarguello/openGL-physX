
#include "globals.h"

//#include "myShader.h"
#include "ImageLoader.h"

using namespace std;
using namespace physx;

static PxVec3 gVertexBuffer[MAX_NUM_MESH_VEC3S];

// SDL variables
SDL_Window* window;
SDL_GLContext glContext;

// PhysX Variables
PxDefaultAllocator      gAllocator;
PxDefaultErrorCallback  gErrorCallback;

PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxCooking* mCooking = NULL;

PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;

PxMaterial* gMaterial = NULL;
PxMaterial* gMaterial2 = NULL;

PxPvd* gPvd = NULL;

vector<PxRigidActor*> boxes;

PxRigidDynamic* gKinematics;

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

void stepPhysics();
void initPhysics();
void cleanupPhysics();
static PX_FORCE_INLINE void renderGeometryHolder(const PxGeometryHolder& h);
static void renderGeometry(const PxGeometry& geom);
void renderActors(PxRigidActor** actors, const PxU32 numActors, bool shadows, const PxVec3& color);
void idleCallback();

/// --------------------------------------------------------------------
/// --------------------------------------------------------------------
/// RENDERING
/// --------------------------------------------------------------------
/// --------------------------------------------------------------------

/// INITIALIZE OPENGL
void init(void)
{
    //// ADD LIGHTS
    glClearColor(0.3f, 0.4f, 0.5f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glDepthFunc(GL_LEQUAL);

    // Setup lighting
    glEnable(GL_LIGHTING);
    PxReal ambientColor[] = { 0.0f, 0.1f, 0.2f, 0.0f };
    PxReal diffuseColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    PxReal specularColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    PxReal position[] = { 0.0f, 100.0f, -120.f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glEnable(GL_LIGHT0);

    //// END ADD LIGHTS

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
void display(void)
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
        renderActors(&actors[0], static_cast<PxU32>(actors.size()), true, color);
    }

    glutSwapBuffers();

}

// RENDER PHYSX ACTORS
void renderActors(PxRigidActor** actors, const PxU32 numActors, bool shadows, const PxVec3& color)
{
    const PxVec3 shadowDir(0.0f, -0.7071067f, -0.7071067f);
    const PxReal shadowMat[] = { 1, 0, 0, 0, -shadowDir.x / shadowDir.y, 0, -shadowDir.z / shadowDir.y, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

    PxShape* shapes[MAX_NUM_ACTOR_SHAPES];

    for (PxU32 i = 0; i < numActors; i++)
    {
        const PxU32 nbShapes = actors[i]->getNbShapes();
        PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
        actors[i]->getShapes(shapes, nbShapes);
        const bool sleeping = actors[i]->is<PxRigidDynamic>() ?
                              actors[i]->is<PxRigidDynamic>()->isSleeping() : false;

        for (PxU32 j = 0; j < nbShapes; j++)
        {
            const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
            const PxGeometryHolder h = shapes[j]->getGeometry();

            if (shapes[j]->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }

            // render object
            glPushMatrix();
            glMultMatrixf(&shapePose.column0.x);

            if (sleeping)
            {
                const PxVec3 darkColor = color * 0.25f;
                glColor4f(darkColor.x, darkColor.y, darkColor.z, 1.0f);
            }
            else
            {
                glColor4f(color.x, color.y, color.z, 1.0f);
            }

            renderGeometryHolder(h);
            glPopMatrix();

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if (shadows)
            {
                glPushMatrix();
                glMultMatrixf(shadowMat);
                glMultMatrixf(&shapePose.column0.x);
                glDisable(GL_LIGHTING);
                glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
                renderGeometryHolder(h);
                glEnable(GL_LIGHTING);
                glPopMatrix();
            }
        }
    }
}

// RENDER PHYSX GEOMETRY HOLDER
static PX_FORCE_INLINE void renderGeometryHolder(const PxGeometryHolder& h)
{
    renderGeometry(h.any());
}

// RENDER PHYSX GEOMETRY
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

                for (PxU32 j = 0; j < nbTris; j++)
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
    gluLookAt(-200.0, 200.0, -100.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
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

void createHull(PxVec3 convexVerts[], PxI32 numberVertices, PxVec3 position) {
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = numberVertices;
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
    PxRigidDynamic* aConvexActor = gPhysics->createRigidDynamic(PxTransform(position));
    PxShape* aConvexShape = PxRigidActorExt::createExclusiveShape(*aConvexActor, PxConvexMeshGeometry(convexMesh), *gMaterial);

    gScene->addActor(*aConvexActor);
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
    PxRigidDynamic* obstacle1 = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(0.0f, 10.0f, 70.0f)), PxBoxGeometry(10.0f,10.0f,10.0f), *gMaterial, 1.0f);
    obstacle1->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
    obstacle1->setMass(0.f);
    obstacle1->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 10.f));
    gScene->addActor(*obstacle1);
    //obstacle1->addForce(PxVec3(0.0f, 0.0f, 5.0f), PxForceMode::eIMPULSE);

    

    // Obstacle 2
    PxVec3 convexVerts[] = {
        PxVec3(0,10,0),
        PxVec3(10,0,0),
        PxVec3(-10,0,0),
        PxVec3(0,0,10),
        PxVec3(0,0,-10)
    };
    createHull(convexVerts, 5, PxVec3(50.0f, 0.0f, 0.0f));

    // Obstacle 3
    PxVec3 convexVerts2[] = {
        PxVec3(0,20,0),
        PxVec3(10,0,0),
        PxVec3(-10,0,0),
        PxVec3(0,0,10),
        PxVec3(0,0,-10)
    };
    createHull(convexVerts2, 5, PxVec3(-50.0f, 0.0f, 0.0f));
    
    /////////////////////////////////////////////////////
    /// ----------- END OBSTACLES  ------------------- //
    /////////////////////////////////////////////////////

    // ------------------------------------------------------------------------------------------------

    /////////////////////////////////////////////////////
    /// ----------------- BALL  ---------------------- //
    /////////////////////////////////////////////////////

    PxRigidDynamic* ball = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(0.0f, 5.0f, 0.0f)), PxSphereGeometry(3.0), *gMaterial, 1.0f);
    ball->setLinearDamping(0.05f);
    ball->setLinearVelocity(PxVec3(0, 0, 30));
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
    PxRigidDynamic* body = gPhysics->createRigidDynamic(pose);
    body->attachShape(*shape);
    gScene->addActor(*body);
    body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    gKinematics = body;

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
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
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
//#include "../../../../PhysX/physx/snippets/snippetrender/SnippetRender.h";
//#include "../../../../PhysX/physx/snippets/snippetrender/SnippetCamera.h";


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

static int shoulder = 0, elbow = 0;

# define PI 3.1415
int n = 20, m = 20;
float r = 1.0, alpha = 0.0, theta = 0.0, delta, h = 1.0;
float xp, yp, zp, puntos[100][100][3], ptosElipses[100][100][2];
float ry = 1.0;
float compZ = -1.5;
float sx = 1.0, sy = 1.0, sz = 1.0;
float tx = 0.0, ty = 0.0;

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
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);

    //// ADD LIGHTS

    glClearColor(0.3f, 0.4f, 0.5f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    // Setup lighting
    glEnable(GL_LIGHTING);
    PxReal ambientColor[] = { 0.0f, 0.1f, 0.2f, 0.0f };
    PxReal diffuseColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    PxReal specularColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    PxReal position[] = { 100.0f, 100.0f, 400.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glEnable(GL_LIGHT0);

    //// END ADD LIGHTS

    glutIdleFunc(idleCallback);
}

// RENDER CUBE
void cube(void) {
    // Multi-colored side - FRONT
    //glBegin(GL_QUADS);

    // Vertices will be added in the next step
    //glVertex3f(-0.5, -0.5, -0.5);       // P1
    //glVertex3f(-0.5, 0.5, -0.5);       // P2
    //glVertex3f(0.5, 0.5, -0.5);       // P3
    //glVertex3f(0.5, -0.5, -0.5);       // P4

    //glEnd();

    // White side - BACK
    glBegin(GL_POLYGON);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(-0.5, -0.5, 0.5);
    glEnd();

    // Purple side - RIGHT
    glBegin(GL_POLYGON);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(0.5, 0.5, -0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(0.5, -0.5, 0.5);
    glEnd();

    // Green side - LEFT
    glBegin(GL_POLYGON);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glEnd();

    // Blue side - TOP
    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(0.5, 0.5, -0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glEnd();

    // Red side - BOTTOM
    glBegin(GL_POLYGON);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glEnd();
}

// RENDER SPHERE
void esfera(void)
{
    // se generan los puntos de la esfera y se guardan en una matriz
    // para esto se emplean coordenadas esfericas
    theta = 2 * PI / m;

    //alpha=PI/lat
    delta = r / m;
    for (int i = 0; i <= n; i++) {
        //cout<<"latitud: "<<i<<endl;
        for (int j = 0; j <= m; j++) {

            puntos[i][j][0] = (r - (delta * i) * cos(theta * j));
            puntos[i][j][1] = (r - (delta * i) * sin(theta * j));
            puntos[i][j][2] = (h / n) * i;
            //cout<<"puntos(x,y,z): "<< puntos[i][j][0]<<", "<<puntos[i][j][1]<<", "<<puntos[i][j][2]<<endl;
        }
        //cout<<endl;
    }

    for (int i = 0; i < n; i++) {
        //cout<<"latitud: "<<i<<endl;
        glColor3f(1 - (float)i / (float)n, 1 - (float)i / (float)n, 1 - (float)i / (float)n);
        for (int j = 0; j < m; j++) {
            glBegin(GL_QUADS);
            //glColor3f((float)i/(float)lat,(float)j/(float)lon,0.5);
            glVertex3f(puntos[i][j][0], puntos[i][j][1], puntos[i][j][2]);
            // glColor3f((float)(i+1)/(float)lat,(float)j/(float)lon,0.5);
            glVertex3f(puntos[i + 1][j][0], puntos[i + 1][j][1], puntos[i + 1][j][2]);
            glColor3f((float)(i + 1) / (float)n, (float)(j + 1) / (float)m, 0.5);
            glVertex3f(puntos[i + 1][j + 1][0], puntos[i + 1][j + 1][1], puntos[i + 1][j + 1][2]);
            // glColor3f((float)i/(float)lat,(float)(j+1)/(float)lon,0.5);
            glVertex3f(puntos[i][j + 1][0], puntos[i][j + 1][1], puntos[i][j + 1][2]);
            glEnd();
        }
    }
}

// DISPLAY ELEMENTS IN SCENE
void display(void)
{
    stepPhysics();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get the scene
    PxGetPhysics().getScenes(&gScene, 1);

    PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
    const PxVec3 color(1.0f, 1.0f, 0.0f);

    if (nbActors)
    {
        std::vector<PxRigidActor*> actors(nbActors);
        gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
        renderActors(&actors[0], static_cast<PxU32>(actors.size()), true, color);
    }

    glutSwapBuffers();

}

// RENDER PHYSX ACTORS
void renderActors(PxRigidActor** actors, const PxU32 numActors, bool shadows, const PxVec3& color)
{
    const PxVec3 shadowDir(0.0f, -0.7071067f, -0.7071067f);
    const PxReal shadowMat[] = { 1,0,0,0, -shadowDir.x / shadowDir.y,0,-shadowDir.z / shadowDir.y,0, 0,0,1,0, 0,0,0,1 };

    PxShape* shapes[MAX_NUM_ACTOR_SHAPES];
    for (PxU32 i = 0;i < numActors;i++)
    {
        const PxU32 nbShapes = actors[i]->getNbShapes();
        PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
        actors[i]->getShapes(shapes, nbShapes);
        const bool sleeping = actors[i]->is<PxRigidDynamic>() ? actors[i]->is<PxRigidDynamic>()->isSleeping() : false;

        for (PxU32 j = 0;j < nbShapes;j++)
        {
            const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
            const PxGeometryHolder h = shapes[j]->getGeometry();

            if (shapes[j]->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            // render object
            glPushMatrix();
            glMultMatrixf(&shapePose.column0.x);

            //glTranslatef(tx, ty, 0);
            //glRotatef(elbow, 1, 0, 0);
            //glScalef(sx, sy, sz);
            //cube();

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
    gluLookAt(-50.0,100.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
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
    switch (key) {
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

    gMaterial = gPhysics->createMaterial(0.0f, 1.0f, 1.0f);
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

void stepPhysics()
{
    for (int i = 0;i < 10;i++) {
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
    init();
 
    glutDisplayFunc(display);

    // Initialize PhysX
    initPhysics();

    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
   
    
    glutMainLoop();
    return 0;
}
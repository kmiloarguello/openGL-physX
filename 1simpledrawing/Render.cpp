// ESIEE Paris - E4 Summer project 2020
// @filename: Render.h
// @description: This file contains all the instructions for the render and camera settings

#include "Render.h"
#include <stdlib.h>
#include <iostream>
#include "PxPhysicsAPI.h"

using namespace std;

using namespace physx;

static PxVec3 gVertexBuffer[MAX_NUM_MESH_VEC3S];
extern void initPhysics();
extern void stepPhysics();
extern void cleanupPhysics();
extern void KeyPress(unsigned char key, int x, int y);

//Render::Render() {}

void Render::setupColors()
{
    glClearColor(0.1f, 0.0f, 0.25f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glDepthFunc(GL_LEQUAL);
}

void Render::setupLights()
{

    // Setup lighting
    glEnable(GL_LIGHTING);

        PxReal ambientColor[] = { 0.0f, 0.1f, 0.2f, 0.0f };
        PxReal diffuseColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };
        PxReal specularColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        PxReal position[] = { 30.0f, 50.0f, -50.0f, 1.0f };
        
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
        glLightfv(GL_LIGHT0, GL_POSITION, position);

    glEnable(GL_LIGHT0);
}

// RENDER PHYSX ACTORS
void Render::renderActors(PxRigidActor** actors, const PxU32 numActors, bool shadows,
                          const PxVec3& color)
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
           
            glColor4f(color.x, color.y, color.z, 1.0f);
            

            Render::renderGeometryHolder(h);
            glPopMatrix();

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if (shadows)
            {
                glPushMatrix();
                glMultMatrixf(shadowMat);
                glMultMatrixf(&shapePose.column0.x);
                glDisable(GL_LIGHTING);
                glColor4f(0.1f, 0.2f, 0.3f, 1.0f);
                Render::renderGeometryHolder(h);
                glEnable(GL_LIGHTING);
                glPopMatrix();
            }
        }
    }
}

// RENDER PHYSX GEOMETRY HOLDER
PX_FORCE_INLINE void Render::renderGeometryHolder(const PxGeometryHolder& h)
{
    Render::renderGeometry(h.any());
}

// RENDER PHYSX GEOMETRY
void Render::renderGeometry(const PxGeometry& geom)
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

void Render::startRender(const PxVec3& cameraEye, const PxVec3& cameraDir, PxReal clipNear, PxReal clipFar)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, GLdouble(glutGet(GLUT_WINDOW_WIDTH)) / GLdouble(glutGet(GLUT_WINDOW_HEIGHT)), GLdouble(clipNear), GLdouble(clipFar));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(GLdouble(cameraEye.x), GLdouble(cameraEye.y), GLdouble(cameraEye.z), GLdouble(cameraEye.x + cameraDir.x), GLdouble(cameraEye.y + cameraDir.y), GLdouble(cameraEye.z + cameraDir.z), 0.0, 1.0, 0.0);

    glColor4f(0.4f, 0.4f, 0.4f, 1.0f);
}

void Render::finishRender()
{
    glutSwapBuffers();
}

// RENDER A CONVEX HULL
PxRigidDynamic* Render::createConvexMesh(PxPhysics& physics, PxVec3* verts, PxU32 numVerts,
                                         PxVec3& position, PxMaterial& material)
{
    // Create descriptor for convex mesh
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = numVerts;
    convexDesc.points.stride = sizeof(PxVec3);
    convexDesc.points.data = verts;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxConvexMesh* convexMesh = NULL;
    PxDefaultMemoryOutputStream buf;
    PxConvexMeshCookingResult::Enum result;

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    convexMesh = physics.createConvexMesh(input);

    PxRigidDynamic* aConvexActor = physics.createRigidDynamic(PxTransform(position));
    PxShape* aConvexShape = PxRigidActorExt::createExclusiveShape(*aConvexActor,
                            PxConvexMeshGeometry(convexMesh), material);
    return aConvexActor;
}

PxConvexMesh* Render::createConvexMesh2(PxPhysics& physics, PxVec3* verts, PxU32 numVerts)
{
    // Create descriptor for convex mesh
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = numVerts;
    convexDesc.points.stride = sizeof(PxVec3);
    convexDesc.points.data = verts;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxConvexMesh* convexMesh = NULL;
    PxDefaultMemoryOutputStream buf;
    PxConvexMeshCookingResult::Enum result;

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    convexMesh = physics.createConvexMesh(input);

    return convexMesh;
}


using namespace physx;

namespace Render
{
    Camera::Camera(const PxVec3& eye, const PxVec3& dir)
    {
        mEye = eye;
        mDir = dir.getNormalized();
        mMouseX = 0;
        mMouseY = 0;
    }

    void Camera::handleMouse(int button, int state, int x, int y)
    {
        PX_UNUSED(state);
        PX_UNUSED(button);
        mMouseX = x;
        mMouseY = y;
    }

    bool Camera::handleKey(unsigned char key, int x, int y, float speed)
    {
        PX_UNUSED(x);
        PX_UNUSED(y);

        PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
        switch (key)
        {
        case 'W':	mEye += mDir * 2.0f * speed;		break;
        case 'S':	mEye -= mDir * 2.0f * speed;		break;
        case 'A':	mEye -= viewY * 2.0f * speed;		break;
        case 'F':	mEye += viewY * 2.0f * speed;		break;
        default:							return false;
        }
        return true;
    }

    void Camera::handleAnalogMove(float x, float y)
    {
        PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();
        mEye += mDir * y;
        mEye += viewY * x;
    }

    void Camera::handleMotion(int x, int y)
    {
        int dx = mMouseX - x;
        int dy = mMouseY - y;

        PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0)).getNormalized();

        PxQuat qx(PxPi * dx / 180.0f, PxVec3(0, 1, 0));
        mDir = qx.rotate(mDir);
        PxQuat qy(PxPi * dy / 180.0f, viewY);
        mDir = qy.rotate(mDir);

        mDir.normalize();
        
        mMouseX = x;
        mMouseY = y;
    }

    PxTransform Camera::getTransform() const
    {
        PxVec3 viewY = mDir.cross(PxVec3(0, 1, 0));

        if (viewY.normalize() < 1e-6f)
            return PxTransform(mEye);

        PxMat33 m(mDir.cross(viewY), viewY, -mDir);
        return PxTransform(mEye, PxQuat(m));
    }

    PxVec3 Camera::getEye() const
    {
        return mEye;
    }

    PxVec3 Camera::getDir() const
    {
        return mDir;
    }


}

namespace Render
{
    Render::Camera* sCamera;

    void keyboardCallback(unsigned char key, int x, int y)
    {
        if (key == 27)
            exit(0);

        //if (!sCamera->handleKey(key, x, y))
          //  keyPress(key, sCamera->getTransform());
    }


    void idleCallback()
    {
        glutPostRedisplay();
    }

    void exitCallback(void)
    {
        delete sCamera;
        cleanupPhysics();
    }
}
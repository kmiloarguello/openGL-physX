// ESIEE Paris - E4 Summer project 2020
// @filename: Engine.cpp
// @description: This file contains all the instructions for the initialization of PhysX library

#include "Engine.h"
/*
PxDefaultAllocator      gAllocator;
PxDefaultErrorCallback  gErrorCallback;

PxFoundation* gFoundation = NULL;
PxPvd* gPvd = NULL;
PxPhysics* gPhysics = NULL;
PxCooking* mCooking = NULL;

PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;*/

// The gFoundation instance initialize PhysX
// This uses some callbacks for memory allocation and errors and also the ID version
PxFoundation* GetFoundation() {
    /*cout << "Initializing Foundation " << endl;
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    return gFoundation;*/

    PxFoundation* a = NULL;
    return a;
}

// PVD is used for visual debugging in PhysX
PxPvd* GetPvd() {
    /*cout << "Initialiwing Pvd" << endl;
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    return gPvd;*/
    PxPvd* a = NULL;
    return a;
}

PxCooking* GetCooking() {/*
    cout << "Initializing Cooking" << endl;
    // Define the scale
    PxTolerancesScale scale;
    scale.length = 100;
    scale.speed = 981; // typical speed of an object, gravity*1s is a reasonable choice

    //  cooking transforms the mesh data into a form which allows the SDK to perform efficient collision detection
    mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(scale));

    if (!mCooking) {
        cout << "PxCreateCooking failed!" << endl;
        return NULL;
    }

    return mCooking;*/
    PxCooking* a = NULL;
    return a;
}

// Initialization of PhysX
PxPhysics* GetPhysX() {
    /*
    cout << "Initializing PhysX" << endl;

    // Here the library initialize the Physics with some tolerance (this can be updated) 
    // ? The tolerance is in animation, maybe gravity, etc
    // This PxCreatePhysics does not have Articulations nor Height fields
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

    return gPhysics;*/

    PxPhysics* a = NULL;
    return a;
}

/*Scene::Scene() {
    Start();
}*/

PxScene* Start() {
    /*
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

    return gScene;*/

    PxScene* a = NULL;
    return a;

}

void Update() {
    /*for (int i = 0; i < 10; i++)
    {
        gScene->simulate(1.0f / 200.0f);
        gScene->fetchResults(true);
    }*/
}

void End() {
    /*gPhysics->release();
    gFoundation->release();
    printf("Pinball ESIEE game finished.\n");*/
}




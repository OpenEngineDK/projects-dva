// main
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// OpenEngine stuff
#include <Meta/Config.h>
#include <Logging/Logger.h>
#include <Logging/StreamLogger.h>
#include <Core/Engine.h>
#include <Display/Frustum.h>
#include <Display/SDLEnvironment.h>
#include <Display/PerspectiveViewingVolume.h>
#include <Devices/IKeyboard.h>
#include <Resources/ResourceManager.h>
#include <Resources/AssimpResource.h>
#include <Scene/SceneNode.h>
#include <Scene/RenderStateNode.h>

#include <Utils/SimpleSetup.h>
#include <Utils/MoveHandler.h>


// DVA stuff
#include "setup.h"
#include "Scene/GridNode.h"

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Display;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Resources;
using namespace dva;

// Global stuff only used for setup.
IEngine* engine;
IEnvironment* env;
SimpleSetup* setup;
Camera* camera;
IMouse* mouse;
IKeyboard* keyboard;
ISceneNode* scene;

void SetupEngine();
void SetupScene();
void SetupDevices();
void SetupResources();

int main(int argc, char** argv) {
    // Setup logging facilities.
    Logger::AddLogger(new StreamLogger(&std::cout));
    logger.info << "========= OpenEngine Warm Up =========" << logger.end;

    //
    SetupEngine();

    //
    SetupScene();

    //
    SetupDevices();

    //
    SetupResources();
 
    // Start the engine.
    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}


void SetupEngine() {
    // Create SDL environment handling display and input
    env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create simple setup
    setup = new SimpleSetup("Det Virtuelle Akvarium", env);
    setup->GetRenderer().SetBackgroundColor(Vector<4, float>(1.0, 1.0, 1.0, 1.0));

    // Get Engine
    engine = &setup->GetEngine();
}

void SetupScene() {
    // Build scene
    scene = new SceneNode();
    setup->SetScene(*scene);

    RenderStateNode *rsn = new RenderStateNode();
    rsn->EnableOption(RenderStateNode::LIGHTING);
    rsn->EnableOption(RenderStateNode::COLOR_MATERIAL);
    rsn->EnableOption(RenderStateNode::BACKFACE);
    scene->AddNode(rsn);
    
    // Just for test
    GridNode* grid = new GridNode(100, 10, Vector<3,float>(0.5, 0.5, 0.5));
    rsn->AddNode(grid);
    
    // Setup camera
    camera  = new Camera(*(new PerspectiveViewingVolume(1, 4000)));
    camera->SetPosition(Vector<3, float>(5.0, 10.0, -1.0));
    camera->SetDirection(Vector<3,float>(1,0,0),Vector<3,float>(0,1,0));
    setup->SetCamera(*camera);
}

void SetupDevices() {
   // Setup move handlers
    mouse = env->GetMouse();
    keyboard = env->GetKeyboard();

    MoveHandler* move = new MoveHandler(*camera, *mouse);
    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    keyboard->KeyEvent().Attach(*move);
}

void SetupResources() {
    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());

    DirectoryManager::AppendPath("resources/models/");
 
    string boxPath = DirectoryManager::FindFileInPath("box/box.dae");
    IModelResourcePtr box = ResourceManager<IModelResource>::Create(boxPath);
    box->Load();

    TransformationNode* boxTrans = new TransformationNode();
    boxTrans->AddNode(box->GetSceneNode());
    //boxTrans->Rotate(0, 0, 0);

    scene->AddNode(boxTrans);
}

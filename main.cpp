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
using namespace dva;

int main(int argc, char** argv) {
    IEngine* engine;

    // Setup logging facilities.
    Logger::AddLogger(new StreamLogger(&std::cout));
    logger.info << "========= OpenEngine Warm Up =========" << logger.end;

    // Create SDL environment handling display and input
    IEnvironment* env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create simple setup
    SimpleSetup* setup = new SimpleSetup("Det Virtuelle Akvarium", env);
    setup->GetRenderer().SetBackgroundColor(Vector<4, float>(1.0, 1.0, 1.0, 1.0));

    // Get Engine
    engine = &setup->GetEngine();

    // Build scene
    SceneNode *root = new SceneNode();
    RenderStateNode *rsn = new RenderStateNode();
    root->AddNode(rsn);

    GridNode* grid = new GridNode(100, 10, Vector<3,float>(1.0, 0.0, 0.0));
    rsn->AddNode(grid);
    setup->SetScene(*root);


    // Setup camera
    Camera*  camera  = new Camera(*(new PerspectiveViewingVolume(1, 4000)));
    camera->SetPosition(Vector<3, float>(5.0, 10.0, -1.0));
    camera->SetDirection(Vector<3,float>(1,0,0),Vector<3,float>(0,1,0));
    setup->SetCamera(*camera);
    
    // Setup move handlers
    IMouse* mouse = env->GetMouse();
    IKeyboard* keyboard = env->GetKeyboard();

    MoveHandler* move = new MoveHandler(*camera, *mouse);
    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    keyboard->KeyEvent().Attach(*move);

    // Start the engine.
    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}



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
#include <Resources/OpenGLShader.h>
#include <Resources/ResourceManager.h>
#include <Resources/AssimpResource.h>
#include <Scene/AnimationNode.h>
#include <Scene/SceneNode.h>
#include <Scene/DotVisitor.h>
#include <Scene/RenderStateNode.h>
#include <Scene/PostProcessNode.h>
#include <Scene/ChainPostProcessNode.h>
#include <Scene/SunNode.h>
#include <Scene/PostProcessNode.h>

#include <Utils/SimpleSetup.h>
#include <Utils/MoveHandler.h>

// Terrain stuff
#include <Utils/TerrainUtils.h>
#include <Utils/TerrainTexUtils.h>
#include <Renderers/OpenGL/TerrainRenderingView.h>

// DVA stuff
#include "setup.h"
#include "Scene/GridNode.h"
#include "Scene/OceanFloorNode.h"
#include "Utils/UserDefaults.h"
#include "Utils/CustomKeyHandler.h"
#include <Animations/Animator.h>

#include <Animations/Flock.h>
#include <Animations/SeperationRule.h>
#include <Animations/AlignmentRule.h>
#include <Animations/CohersionRule.h>

#include <Animations/GotoRule.h>

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Display;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Animations;
using namespace dva;

// Global stuff only used for setup.
IEngine* engine;
IEnvironment* env;
SimpleSetup* setup;
Camera* camera;
IMouse* mouse;
IKeyboard* keyboard;
//ISceneNode* scene;
RenderStateNode *rsn;
IRenderer* renderer;

vector<ISceneNode*> sceneNodes;

ISceneNode* postEffects;
ISceneNode* fog;
ISceneNode* light;

OceanFloorNode* oceanFloor;
ISceneNode* fish = NULL;
ISceneNode* boat;

AnimationNode* animations;
ISceneNode* animated = NULL;

Flock* flock = NULL;

// Forward declarations
void SetupEngine();
void SetupScene();
void SetupDevices();
void SetupBoids();
void LoadResources();


int main(int argc, char** argv) {
    // Print start message
    logger.info << "========= OpenEngine Warm Up =========" << logger.end;

    //
    SetupEngine();

    //
    SetupDevices();

    //
    LoadResources();

    //
    SetupScene();

    //
    SetupBoids();

    // Write dot graph    
    DotVisitor dv;
    ofstream os("graph.dot", ofstream::out);
    dv.Write(*(setup->GetScene()), &os);

    // Start the engine.
    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}


void SetupEngine() {
    // Create SDL environment handling display and input
    env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT);
    //env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT, 32, FRAME_FULLSCREEN);

    // Create rendering view.
    IRenderingView* rv = new TerrainRenderingView();

    // Create simple setup
    setup = new SimpleSetup("Det Virtuelle Akvarium", env, rv);
    renderer = &setup->GetRenderer();
    //    renderer->SetBackgroundColor(Vector<4, float>(0.1, 0.1, 0.3, 1.0));
    renderer->SetBackgroundColor(Vector<4, float>(0.4, 0.6, 0.8, 1.0));

    // Setup camera
    camera  = new Camera(*(new PerspectiveViewingVolume(1, 8000)));
    camera->SetPosition(Vector<3, float>(60.0, 30.0, -10.0));
    camera->LookAt(0,0,0);
    //camera->SetPosition(Vector<3, float>(0.0, 220.0, -2100.0));
    //camera->SetDirection(Vector<3,float>(0,0,1),Vector<3,float>(0,1,0));
    //camera->LookAt(0,0,-1500);
    
    setup->SetCamera(*camera);

    // Get Engine
    engine = &setup->GetEngine();

    // Show frame pr. second.
    setup->ShowFPS();
}

void SetupScene() {
    // Start by setting the root node in the scene graph.
    ISceneNode* sceneRoot = new SceneNode();
    setup->SetScene(*sceneRoot);

    // scene represents where to insert next node.
    ISceneNode* scene = sceneRoot;

    // Create caustics post process
    Vector<2, int> dimension(SCREEN_WIDTH, SCREEN_HEIGHT);
    IShaderResourcePtr caustics = ResourceManager<IShaderResource>::Create("projects/dva/effects/caustics.glsl");
    PostProcessNode* causticsNode = new PostProcessNode(caustics, dimension); 
    renderer->InitializeEvent().Attach(*causticsNode);
    scene->AddNode(causticsNode); scene = causticsNode;

    // Create sun
    Vector<3, float> origo = Vector<3, float>(0.0, 100.0, 0.0);
    Vector<3, float> sunDir = Vector<3, float>(1448, 2048, 1448);
    SunNode* sun = new SunNode(sunDir, origo);
    sun->SetRenderGeometry(false);
    sun->SetDayLength(0.0f);
    sun->SetTimeOfDay(11.0f);
    sun->SetAmbient(Vector<4, float>(0.06, 0.12, 0.17, 1.0));
    sun->SetDiffuse(Vector<4, float>(0.8, 1.0, 0.8, 1.0));
    setup->GetEngine().ProcessEvent().Attach(*sun);
    scene->AddNode(sun);
    scene = sun;

    rsn = new RenderStateNode();
    //    rsn->DisableOption(RenderStateNode::BACKFACE);
    rsn->EnableOption(RenderStateNode::LIGHTING);
    scene->AddNode(rsn);
    scene = rsn;

    //rsn->AddNode(fog);
    //ISceneNode* oceanScene = fog;


    // Add all scene nodes.
    vector<ISceneNode*>::iterator itr;
    for(itr=sceneNodes.begin(); itr!=sceneNodes.end(); itr++){
        ISceneNode* node = *itr;
        scene->AddNode(node);
    }

    // Ocean floor as height map.
    //    if( oceanFloor ) oceanScene->AddNode(oceanFloor);
    
    //
    //oceanScene->AddNode(fog);
    //    fog->AddNode(boat);
    //oceanScene->AddNode(boat);

    // Add single fish
    //    if(fish) oceanScene->AddNode(fish);

    // Just for debug
    GridNode* grid = new GridNode(100, 10, Vector<3,float>(0.5, 0.5, 0.5));
    scene->AddNode(grid);
}

void SetupBoids() {
    flock = new Flock();
    
    flock->AddRule(new SeperationRule());
    flock->AddRule(new AlignmentRule());
    flock->AddRule(new CohersionRule());
    flock->AddRule(new GotoRule(Vector<3,float>(0,0,0)));
    

    vector<ISceneNode*>::iterator itr;
    for(itr=sceneNodes.begin(); itr!=sceneNodes.end(); itr++){
        ISceneNode* node = *itr;
        for (int i=0;i<10;i++) {
            flock->AddBoid(node->Clone());
        }
    }
    engine->ProcessEvent().Attach(*flock);
    

    rsn->AddNode(flock->GetRootNode());
}

void SetupDevices() {
   // Setup move handlers
    mouse = env->GetMouse();
    keyboard = env->GetKeyboard();

    MoveHandler* move = new MoveHandler(*camera, *mouse);
    move->SetMoveScale(0.002);
    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    keyboard->KeyEvent().Attach(*move);

    CustomKeyHandler* ckh = new CustomKeyHandler(*setup);
    keyboard->KeyEvent().Attach(*ckh);
}

void LoadResources() {
    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());
    ResourceManager<IShaderResource>::AddPlugin(new GLShaderPlugin());
    DirectoryManager::AppendPath("resources/");
    string path;

//     // Load Shaders
    Vector<2, int> dimension(SCREEN_WIDTH, SCREEN_HEIGHT);
//     std::list<IShaderResourcePtr> effects;
//     IShaderResourcePtr glow = ResourceManager<IShaderResource>::Create("shaders/Glow.glsl");
//     IShaderResourcePtr blur = ResourceManager<IShaderResource>::Create("shaders/HorizontalBoxBlur.glsl");
//     effects.push_back(glow);
//     effects.push_back(blur);
//  
//     ChainPostProcessNode* glowNode = new ChainPostProcessNode(effects, dimension, 1, true);
//     glow->SetTexture("scene", glowNode->GetPostProcessNode(1)->GetSceneFrameBuffer()->GetTexAttachment(0));
//     renderer->InitializeEvent().Attach(*glowNode);
//     postEffects = glowNode;
//     IShaderResourcePtr underwater = ResourceManager<IShaderResource>::Create("shaders/UnderWater.glsl");
//     PostProcessNode* underwaterNode = new PostProcessNode(underwater, dimension);
//     underwaterNode->SetEnabled(true);
//     renderer->InitializeEvent().Attach(*underwaterNode);
//     postEffects = underwaterNode;

//     IShaderResourcePtr grayscale = ResourceManager<IShaderResource>::Create("shaders/grayscale/grayscale.glsl");
//     PostProcessNode* grayScaleNode = new PostProcessNode(grayscale, dimension);
//     grayScaleNode->SetEnabled(true);
//     renderer->InitializeEvent().Attach(*grayScaleNode);
//     postEffects = grayScaleNode;


//     IShaderResourcePtr lightPtr = ResourceManager<IShaderResource>::Create("shaders/dir_light.glsl");
//     PostProcessNode* lightNode = new PostProcessNode(lightPtr, dimension);
//     lightNode->SetEnabled(true);
//     renderer->InitializeEvent().Attach(*lightNode);
//     light = lightNode;


//     IShaderResourcePtr fogPtr = ResourceManager<IShaderResource>::Create("shaders/fog/fog.glsl");
//     PostProcessNode* fogNode = new PostProcessNode(fogPtr, dimension);
//     fogNode->SetEnabled(true);
//     renderer->InitializeEvent().Attach(*fogNode);
//     fog = fogNode;


    // Load ocean floor
//     path = DirectoryManager::FindFileInPath("models/environment/environment.DAE");
//     IModelResourcePtr model = ResourceManager<IModelResource>::Create(path);
//     model->Load();
//     ISceneNode* oceanFloor = model->GetSceneNode();
//     oceanFloor->SetNodeName("Ocean Floor Model\n[ISceneNode]");
//     sceneNodes.push_back(oceanFloor);
    
    // Create ocean floor
//     FloatTexture2DPtr map = FloatTexture2DPtr(new FloatTexture2D(1024, 1024, 1));
//     Empty(map);
//     // map, steps, radius, disp
//     map = CreateSmoothTerrain(map, 10, 160, 6);
//     map = CreateSmoothTerrain(map, 1, 500, -40);
//     map = CreateSmoothTerrain(map, 8000, 50, 3.0);
//     //    map = MakePlateau(map, 100, 100);
//     float widthScale = 4.0;
//     Vector<3, float> origo = Vector<3, float>(map->GetHeight() * widthScale / 2, 0, map->GetWidth() * widthScale / 2);
//     OceanFloorNode* node = new OceanFloorNode(map);
//     node->SetWidthScale(widthScale);
//     node->SetOffset(origo * -1);
//     setup->GetRenderer().InitializeEvent().Attach(*node);
//     setup->GetEngine().ProcessEvent().Attach(*node);
//     sceneNodes.push_back(node);
    
    
    // Load boat
    //    path = DirectoryManager::FindFileInPath("models/environment/boat.dae");
//     path = DirectoryManager::FindFileInPath("models/environment/Environment_org.dae");
//     IModelResourcePtr boat_model = ResourceManager<IModelResource>::Create(path);
//     boat_model->Load();
//     TransformationNode* boat_trans = new TransformationNode();
//     boat_trans->Scale(0.1, 0.1, 0.1);
//     //boat_trans->Move(0, 0, 0);
//     //boat_trans->Rotate(0, -PI, 0);
//     boat_trans->AddNode(boat_model->GetSceneNode());
//     sceneNodes.push_back(boat_trans);

    // Load fish
    //path = DirectoryManager::FindFileInPath("models/finn/Finn08.DAE");
    path = DirectoryManager::FindFileInPath("models/finn/Finn08_org.DAE");
    //path = DirectoryManager::FindFileInPath("models/sharky/Sharky07.DAE");
    //path = DirectoryManager::FindFileInPath("models/finn/Finn10_org.DAE");
    //path = DirectoryManager::FindFileInPath("models/finn/Finn10.DAE");
    //path = DirectoryManager::FindFileInPath("models/sharky/Animationstest med env.DAE");
    //path = DirectoryManager::FindFileInPath("models/oe_logo/Bull_002_cea.dae");

    IModelResourcePtr fish_model = ResourceManager<IModelResource>::Create(path);
    fish_model->Load();
    ISceneNode* fish_node = fish_model->GetSceneNode();
    fish_node->SetNodeName("Collada Model\n[ISceneNode]");

    

    AnimationNode* animations = fish_model->GetAnimations();
     if( animations ){
        Animator* animator = new Animator(animations);
        UserDefaults::GetInstance()->map["Animator"] = animator;
        if( animator->GetSceneNode() ){
            TransformationNode* fishA = new TransformationNode();
            fishA->Rotate(PI/2.0,0,0);
            //            fishA->Move(0,0,-100);
            //            fishA->Scale(10,10,10);
            fishA->AddNode(animator->GetSceneNode());
            sceneNodes.push_back(fishA);

//             TransformationNode* fishB = new TransformationNode();
//             fishB->Rotate(PI/2.0,0,0);
//             fishB->Move(0,0,-10);
//             fishB->AddNode(animator->GetSceneNode()->Clone());
//             sceneNodes.push_back(fishB);
        }
        setup->GetEngine().ProcessEvent().Attach(*animator);
        animator->SetActiveAnimation(0);
        animator->Play();
    }
}


void SetupTerrain(SimpleSetup* setup){
}


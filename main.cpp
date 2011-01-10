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
#include <Scene/DirectionalLightNode.h>
#include <Scene/RenderStateNode.h>
#include <Scene/PostProcessNode.h>
#include <Scene/ChainPostProcessNode.h>
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
#include <Animations/FlockPropertyReloader.h>
#include <Animations/SeperationRule.h>
#include <Animations/AlignmentRule.h>
#include <Animations/CohersionRule.h>
#include <Animations/GotoRule.h>
#include <Animations/SpeedRule.h>
#include <Animations/FollowRule.h>
#include <Animations/BoxRule.h>
#include <Animations/RandomRule.h>

#include <Utils/PropertyTree.h>

// HUD stuff
#include <Display/OpenGL/BlendCanvas.h>
#include <Display/OpenGL/TextureCopy.h>
// #include <Display/CanvasQueue.h>
#include <Renderers/TextureLoader.h>
// #include <Display/OpenGL/FadeCanvas.h>
#include "Utils/Stages.h"

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Display;
using namespace OpenEngine::Display::OpenGL;
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

Stages* stages = NULL;
// CanvasQueue* cq = NULL;

Flock* flock = NULL;
TransformationNode* flockFollow = NULL;

// Forward declarations
void SetupEngine();
void SetupScene();
void SetupDevices();
void SetupBoids();
void LoadResources();


class CircleMover : public IListener<Core::ProcessEventArg> {
    TransformationNode* node;
    float pos;
    Vector<2,float> offset;
    float speed;
public:
    CircleMover(TransformationNode *n, 
                Vector<2,float> of=(Vector<2,float>(100,100)),
                float s=2)
        : node(n),pos(0),offset(of),speed(s) {}
    void Handle(Core::ProcessEventArg arg) {
        float delta = arg.approx/1000000.0;
        pos += delta;
        node->SetPosition(Vector<3,float>(offset[0]*sin(pos*speed),
                                          0,
                                          offset[1]*cos(pos*speed)));
    }
};

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

    // Create rendering view.
    IRenderingView* rv = new TerrainRenderingView();

    // Create simple setup
    setup = new SimpleSetup("Det Virtuelle Akvarium", env, rv);
    renderer = &setup->GetRenderer();
    //    renderer->SetBackgroundColor(Vector<4, float>(0.1, 0.1, 0.3, 1.0));
    renderer->SetBackgroundColor(Vector<4, float>(0.4, 0.6, 0.8, 1.0));

    // Setup camera
    camera  = new Camera(*(new PerspectiveViewingVolume(1, 8000)));
    camera->SetPosition(Vector<3, float>(70.0, 30.0, -10.0));
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
    // Setup stage fading Stuff
    stages = new Stages(setup->GetFrame(), setup->GetTextureLoader(), setup->GetCanvas());
    engine->InitializeEvent().Attach(*stages);
    engine->DeinitializeEvent().Attach(*stages);
    engine->ProcessEvent().Attach(*stages);
    
    // Setup HUD stuff
    // hud = new BlendCanvas(new TextureCopy());
    // ITexture2DPtr img = ResourceManager<ITextureResource>::Create("projects/dva/data/small.jpg");
    // setup->GetTextureLoader().Load(img);
    // hud->AddTexture(setup->GetCanvas(), 0, 0, Vector<4,float>(1.0, 1.0, 1.0, 1.0));
    // hud->AddTexture(img, 0, 0, Vector<4,float>(1.0, 1.0, 1.0, 1.0));
    // hud->SetBackground(Vector<4,float>(1.0,1.0,1.0,1.0));
    
    // Start by setting the root node in the scene graph.
    ISceneNode* sceneRoot = new SceneNode();
    setup->SetScene(*sceneRoot);

    // scene represents where to insert next node.
    ISceneNode* scene = sceneRoot;

    // Create fog post process
    Vector<2, int> dimension(SCREEN_WIDTH, SCREEN_HEIGHT);
    IShaderResourcePtr fog = ResourceManager<IShaderResource>::Create("projects/dva/effects/fog.glsl");
    PostProcessNode* fogNode = new PostProcessNode(fog, dimension); 
    renderer->InitializeEvent().Attach(*fogNode);
    scene->AddNode(fogNode); scene = fogNode;

    // Create caustics post process
    IShaderResourcePtr caustics = ResourceManager<IShaderResource>::Create("projects/dva/effects/caustics.glsl");
    caustics->SetUniform("lightDir", Vector<3, float>(0, -1, 0));
    PostProcessNode* causticsNode = new PostProcessNode(caustics, dimension); 
    renderer->InitializeEvent().Attach(*causticsNode);
    scene->AddNode(causticsNode); scene = causticsNode;

    // Create point light
    TransformationNode* lightTrans = new TransformationNode();
    DirectionalLightNode* lightNode = new DirectionalLightNode();
    lightNode->ambient = Vector<4,float>(0.6,0.8,0.5,1.0);
    scene->AddNode(lightTrans);
    lightTrans->AddNode(lightNode);

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
    string confPath = DirectoryManager::FindFileInPath("boids.yaml");
    PropertyTree* ptree = new PropertyTree(confPath);    

    engine->InitializeEvent().Attach(*ptree);
    engine->ProcessEvent().Attach(*ptree);
    engine->DeinitializeEvent().Attach(*ptree);


    flockFollow = new TransformationNode();
    flockFollow->SetPosition(Vector<3,float>(0,0,-100));
    CircleMover *cm = new CircleMover(flockFollow,Vector<2,float>(10,100),1);
    engine->ProcessEvent().Attach(*cm);


    flock = new Flock();    
    flock->AddRule(new SeperationRule());
    flock->AddRule(new CohersionRule());
    //flock->AddRule(new GotoRule());
    flock->AddRule(new SpeedRule());
    flock->AddRule(new AlignmentRule());
    flock->AddRule(new FollowRule(flockFollow));
    flock->AddRule(new RandomRule());
    flock->AddRule(new BoxRule(Box(Vector<3,float>(0.0),
                                   Vector<3,float>(100.0))));
        


    FlockPropertyReloader *rl = new FlockPropertyReloader(flock, ptree, "flock1");
    

    vector<ISceneNode*>::iterator itr;
    for(itr=sceneNodes.begin(); itr!=sceneNodes.end(); itr++){
        ISceneNode* node = *itr;
        for (int i=0;i<100;i++) {
            flock->AddBoid(node->Clone());
        }
    }
    engine->ProcessEvent().Attach(*flock);
    

    rsn->AddNode(flock->GetRootNode());
    flock->GetRootNode()->AddNode(flockFollow);

    flockFollow->AddNode(sceneNodes[0]->Clone());
}

void SetupDevices() {
   // Setup move handlers
    mouse = env->GetMouse();
    keyboard = env->GetKeyboard();

    MoveHandler* move = new MoveHandler(*camera, *mouse);
    move->SetMoveScale(0.001);
    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    keyboard->KeyEvent().Attach(*move);

    CustomKeyHandler* ckh = new CustomKeyHandler(*setup);
    keyboard->KeyEvent().Attach(*ckh);
}

void LoadResources() {
    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());
    DirectoryManager::AppendPath("resources/");
    DirectoryManager::AppendPath("projects/dva/");
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
    //path = DirectoryManager::FindFileInPath("models/environment/boat.dae");
    //path = DirectoryManager::FindFileInPath("models/environment/Environment_org.dae");
//     path = DirectoryManager::FindFileInPath("models/boat/Wreck01.DAE");
    
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


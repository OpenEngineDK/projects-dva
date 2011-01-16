
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
#include <Scene/PointLightNode.h>
#include <Scene/RenderStateNode.h>
#include <Scene/PostProcessNode.h>
#include <Scene/ChainPostProcessNode.h>
#include <Scene/PostProcessNode.h>
#include <Scene/SearchTool.h>

#include <Utils/SimpleSetup.h>
#include <Utils/MoveHandler.h>

// Terrain stuff
#include <Utils/TerrainUtils.h>
#include <Utils/TerrainTexUtils.h>
#include <Renderers/OpenGL/TerrainRenderingView.h>

// DVA stuff
#include "setup.h"
#include "Scene/GridNode.h"
#include "Utils/UserDefaults.h"
#include "Utils/CustomKeyHandler.h"
#include "Devices/LaserSensor.h"
#include "Predator.h"
#include "LaserDebug.h"

#include <Animations/Animator.h>
#include <Animations/Animation.h>
#include <Animations/AnimatedTransformation.h>
#include <Animations/Flock.h>
#include <Animations/FlockPropertyReloader.h>
#include <Animations/SeperationRule.h>
#include <Animations/AlignmentRule.h>
#include <Animations/CohersionRule.h>
#include <Animations/GotoRule.h>
#include <Animations/SpeedRule.h>
#include <Animations/FollowRule.h>
#include <Animations/BoxLimitRule.h>
#include <Animations/BoxRule.h>
#include <Animations/RandomRule.h>
#include <Animations/FleeRule.h>

#include <Utils/PropertyTree.h>

#include <Display/FollowCamera.h>
#include <Display/TrackingCamera.h>
#include <Display/TrackingFollowCamera.h>
#include <Display/InterpolatedViewingVolume.h>

// HUD stuff
#include <Display/OpenGL/BlendCanvas.h>
#include <Display/OpenGL/TextureCopy.h>
// #include <Display/CanvasQueue.h>
#include <Renderers/TextureLoader.h>
// #include <Display/OpenGL/FadeCanvas.h>
#include "Utils/Stages.h"

// temporary hack ... remove next two lines when animation branch has been merged with main branch
#include <Renderers/OpenGL/ShaderLoader.h>
#include <Renderers/OpenGL/LightRenderer.h>

#include <Scene/ShadowLightPostProcessNode.h>

#include <Renderers/DataBlockBinder.h>

#include <boost/serialization/weak_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>

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

class CameraSwitcher : public IListener<KeyboardEventArg> {
    SimpleSetup* setup;
    vector<Camera*> cams;
    unsigned int idx;
    
public:
    CameraSwitcher(SimpleSetup* setup)
        : setup(setup), idx(0) {
        cams.push_back(setup->GetCamera());
    }

    void AddCamera(Camera* c) {
        cams.push_back(c);
    }

    void Handle(KeyboardEventArg arg) {
        if (arg.type != EVENT_RELEASE) return;
        if (arg.sym == KEY_p && cams.size() > (idx+1)) {
            ++idx;
            setup->SetCamera(*cams[idx]);
        } else if (arg.sym == KEY_o && cams.size() > (idx-1)) {
            --idx;
            setup->SetCamera(*cams[idx]);
        }
    }
};

class CircleMover : public IListener<Core::ProcessEventArg> {
    TransformationNode* node;
    float pos;
    Vector<3,float> offset;
    Vector<2,float> circle;
    float speed;
public:
    CircleMover(TransformationNode *n, 
                Vector<2,float> cir=(Vector<2,float>(10,100)),
                float s=2)
        : node(n),pos(0),offset(n->GetPosition()), circle(cir),speed(s) {}
    void Handle(Core::ProcessEventArg arg) {
        float delta = arg.approx/1000000.0;
        pos += delta;
        node->SetPosition(Vector<3,float>(offset[0]+circle[0]*sin(pos*speed),
                                          offset[1],
                                          offset[2]+circle[1]*cos(pos*speed)));
    }
};


// Global stuff only used for setup.
IEngine* engine;
IEnvironment* env;
SimpleSetup* setup;
Camera* camera;
IMouse* mouse;
IKeyboard* keyboard;
RenderStateNode *rsn;
IRenderer* renderer;

vector<ISceneNode*> sceneNodes;

ISceneNode* postEffects;
ISceneNode* fog;
ISceneNode* light;

ISceneNode* fish = NULL;
TransformationNode* human = NULL;  // Laser input for predator instance.
TransformationNode* shark = NULL;
TransformationNode* sharkHead = NULL;
TransformationNode* box = NULL;

ISceneNode* sharkAnimRoot = NULL;

LaserDebugPtr laserDebug;

AnimationNode* animations;
ISceneNode* animated = NULL;

Stages* stages = NULL;
// CanvasQueue* cq = NULL;

Flock* flock = NULL;
TransformationNode* flockFollow = NULL;

CameraSwitcher* camSwitch = NULL;

// Forward declarations
void SetupEngine();
void SetupScene();
void SetupDevices();
void SetupBoids();
void LoadResources();


AnimationNode* GetAnimationNode(ISceneNode* node) {
    SearchTool st;
    return st.DescendantAnimationNode(node);
}

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

    DataBlockBinder* bob = new DataBlockBinder(setup->GetRenderer(), 
                                               DataBlockBinder::RELOAD_IMMEDIATE);
    bob->Bind(*setup->GetScene());


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
    renderer->SetBackgroundColor(Vector<4, float>(0.4, 0.6, 0.8, 1.0));

    // Get Engine
    engine = &setup->GetEngine();

    // Show frame pr. second.
    //    setup->ShowFPS();
}


void SetupDevices() {
   // Setup move handlers
    mouse = env->GetMouse();
    keyboard = env->GetKeyboard();

    UserDefaults::GetInstance()->map["Mouse"] = mouse;
    UserDefaults::GetInstance()->map["Keyboard"] = keyboard;

    // Setup cameras
    camera  = new Camera(*(new PerspectiveViewingVolume(1, 8000)));
    camera->SetPosition(Vector<3, float>(0.0, 54.0, 0.0));
    camera->LookAt(0,190,-2000);
    setup->SetCamera(*camera);

    camSwitch = new CameraSwitcher(setup); // CameraSwitcher adds the current cam from setup
    keyboard->KeyEvent().Attach(*camSwitch);


    MoveHandler* move = new MoveHandler(*camera, *mouse);
    move->SetMoveScale(0.001);
    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    keyboard->KeyEvent().Attach(*move);

    CustomKeyHandler* ckh = new CustomKeyHandler(*setup);
    keyboard->KeyEvent().Attach(*ckh);

    laserDebug = LaserDebug::Create(SCREEN_WIDTH, SCREEN_HEIGHT);
    setup->GetTextureLoader().Load(laserDebug, Renderers::TextureLoader::RELOAD_IMMEDIATE);

    // Setup laser sensor device.
    LaserSensor* laserSensor = new LaserSensor(LASER_SENSOR_IP, LASER_SENSOR_PORT);
    laserSensor->SetLaserDebug(laserDebug);
    engine->InitializeEvent().Attach(*laserSensor);
    engine->ProcessEvent().Attach(*laserSensor);
    
    
}


void LoadResources() {
    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());
    DirectoryManager::AppendPath("resources/");
    DirectoryManager::AppendPath("projects/dva/");
    string path;

    // Load place holder box.
    path = DirectoryManager::FindFileInPath("models/box/box.dae");
    IModelResourcePtr boxModel = ResourceManager<IModelResource>::Create(path);
    boxModel->Load();
    TransformationNode* boxTrans = new TransformationNode();
    boxTrans->SetScale(Vector<3,float>(0.8));
    boxTrans->AddNode(boxModel->GetSceneNode());
    box = boxTrans;

    // Load model representing human interaction.
    path = DirectoryManager::FindFileInPath("models/box/box.dae");
    IModelResourcePtr humanModel = ResourceManager<IModelResource>::Create(path);
    humanModel->Load();
    human = new TransformationNode();
    human->SetScale(Vector<3,float>(0.1));
    human->AddNode(humanModel->GetSceneNode());

    // Load environment.
    path = DirectoryManager::FindFileInPath("models/environment/Environment03.DAE");
    IModelResourcePtr envModel = ResourceManager<IModelResource>::Create(path);
    envModel->Load();
    ISceneNode* env = envModel->GetSceneNode();
    env->SetInfo("Environment Model\n[ISceneNode]");
    TransformationNode* envTrans = new TransformationNode();
    envTrans->SetPosition(Vector<3,float>(0, 0, 0));
    envTrans->AddNode(env);
    sceneNodes.push_back(envTrans);

    // Load shark.
    path = DirectoryManager::FindFileInPath("models/sharky/Sharky09.DAE");
    IModelResourcePtr sharkModel = ResourceManager<IModelResource>::Create(path);
    sharkModel->Load();
    ISceneNode* sharky = sharkModel->GetSceneNode();
    sharky->SetInfo("Sharky the not so friendly shark\n[ISceneNode]");
    AnimationNode* sharkAnim = GetAnimationNode(sharky);
    if( sharkAnim ){
        Animator* animator = new Animator(sharkAnim);
        UserDefaults::GetInstance()->map["SharkAnimator"] = animator;
        if( animator->GetSceneNode() ){
            sharkAnimRoot = sharkAnim;
            TransformationNode* sharkTrans = new TransformationNode();
            sharkTrans->SetInfo("SHARK TRANSFORMATION");
            sharkTrans->AddNode(animator->GetSceneNode());
            sceneNodes.push_back(sharkTrans);
        }
        setup->GetEngine().ProcessEvent().Attach(*animator);
        animator->SetActiveAnimation(0);
        //animator->Play();
    }
    
    // Load fish
    path = DirectoryManager::FindFileInPath("models/finn/Finn08_org.DAE");
    IModelResourcePtr model = ResourceManager<IModelResource>::Create(path);
    model->Load();
    ISceneNode* fishModel = model->GetSceneNode();
    fishModel->SetInfo("Finn the fish model\n[ISceneNode]");
    AnimationNode* animations = GetAnimationNode(fishModel);
     if( animations ){
        Animator* animator = new Animator(animations);
        UserDefaults::GetInstance()->map["Animator"] = animator;
        if( animator->GetSceneNode() ){
            TransformationNode* fishTrans = new TransformationNode();
            fishTrans->AddNode(animator->GetSceneNode());
            fish = fishTrans;
        }
        setup->GetEngine().ProcessEvent().Attach(*animator);
        animator->SetActiveAnimation(0);
        animator->Play();
    }
}


void SetupScene() {
    // Setup stage fading Stuff
    BlendCanvas* b = new BlendCanvas(new TextureCopy());
    b->AddTexture(setup->GetCanvas()->GetTexture(), 0, 0, Vector<4,float>(1.0, 1.0, 1.0, 1.0));
    b->SetBackground(Vector<4,float>(1.0,1.0,1.0,1.0));
    //b->AddTexture(laserDebug, 0, 0, Vector<4,float>(1.0, 1.0, 1.0, 1.0));
    b->InitCanvas(setup->GetCanvas());

    stages = new Stages(setup->GetFrame(), setup->GetTextureLoader(), b);
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

    // temporary hack ... remove next line when animation branch has been merged with main branch
    setup->GetShaderLoader()->SetLightRenderer(setup->GetLightRenderer());

    // scene represents where to insert next node.
    ISceneNode* scene = sceneRoot;
    Vector<2, int> dimension(SCREEN_WIDTH, SCREEN_HEIGHT);
 



    // Create fog post process   
    IShaderResourcePtr fog = ResourceManager<IShaderResource>::Create("projects/dva/effects/fog.glsl");
    PostProcessNode* fogNode = new PostProcessNode(fog, dimension); 
    //fogNode->SetEnabled(false);
    renderer->InitializeEvent().Attach(*fogNode);
    scene->AddNode(fogNode); 
    scene = fogNode;

    //Create Shadow post process
    IShaderResourcePtr shadow = ResourceManager<IShaderResource>::Create("projects/dva/effects/shadowmap.glsl");
    ShadowLightPostProcessNode* shadowPost = 
        new ShadowLightPostProcessNode(shadow, 
                                       dimension,
                                       //dimension
                                       Vector<2,int>(800,600)
                                       );
    renderer->InitializeEvent().Attach(*shadowPost);
    renderer->PreProcessEvent().Attach(*shadowPost);
    scene->AddNode(shadowPost); 
    scene = shadowPost;
    Camera* cam = new Camera(*(new PerspectiveViewingVolume(100,2000)));
    cam->SetPosition(Vector<3,float>(0,1000,-200));
    cam->LookAt(Vector<3,float>(0,0,-400));
    camSwitch->AddCamera(cam);    
    shadowPost->SetViewingVolume(cam);


    // Create caustics post process
    IShaderResourcePtr caustics = ResourceManager<IShaderResource>::Create("projects/dva/effects/caustics.glsl");
    caustics->SetUniform("lightDir", Vector<3, float>(0, -1, 0));
    PostProcessNode* causticsNode = new PostProcessNode(caustics, dimension); 
    //causticsNode->SetEnabled(false);
    renderer->InitializeEvent().Attach(*causticsNode);
    scene->AddNode(causticsNode); 
    scene = causticsNode;

    // Create point light
    TransformationNode* lightTrans = new TransformationNode();
    PointLightNode* lightNode = new PointLightNode();
    lightTrans->SetPosition(Vector<3,float>(0.0,100.0,0.0));
    lightNode->ambient = Vector<4,float>(0.6,0.8,0.5,1.0);
//     lightNode->ambient = Vector<4,float>(0.0,0.4,0.0,1.0);
//     lightNode->diffuse = Vector<4,float>(0.0,1.0,0.0,1.0);
//     lightNode->specular = Vector<4,float>(.2,.2,.2,1.0);
    //lightNode->linearAtt = 0.01;
    scene->AddNode(lightTrans);
    lightTrans->AddNode(lightNode);

//     TransformationNode* lightTrans1 = new TransformationNode();
//     // lightTrans1->SetRotation(Quaternion<float>(Math::PI, Vector<3,float>(1.0,0.0,0.0)));
//     lightTrans1->SetPosition(Vector<3,float>(0.0,-100.0,0.0));
//     PointLightNode* lightNode1 = new PointLightNode();
//     lightNode1->ambient = Vector<4,float>(0.4,0.0,0.0,1.0);
//     lightNode1->diffuse = Vector<4,float>(1.0,0.0,0.0,1.0);
//     lightNode1->linearAtt = 0.01;
//     scene->AddNode(lightTrans1);
//     lightTrans1->AddNode(lightNode1);

    rsn = new RenderStateNode();
    rsn->DisableOption(RenderStateNode::BACKFACE);
    rsn->EnableOption(RenderStateNode::LIGHTING);
    rsn->EnableOption(RenderStateNode::COLOR_MATERIAL);
    rsn->DisableOption(RenderStateNode::SHADER);
    scene->AddNode(rsn);
    scene = rsn;

    // Add all scene nodes.
    vector<ISceneNode*>::iterator itr;
    for(itr=sceneNodes.begin(); itr!=sceneNodes.end(); itr++){
        ISceneNode* node = *itr;
        scene->AddNode(node);
    }

    // Add predator to the scene.
    Predator* humanPredator = new Predator(human); // same shark trans as given to the flee rule.
    mouse->MouseMovedEvent().Attach(*humanPredator);
    scene->AddNode(human);

    SearchTool search;
    std::list<AnimationNode*> animNodeRes;
    animNodeRes = search.DescendantAnimationNodes(sharkAnimRoot);
    if( animNodeRes.size() > 0 ){
        shark = animNodeRes.front()->GetAnimation()->GetAnimatedTransformation(0)->GetAnimatedNode();
        sharkHead = animNodeRes.front()->GetAnimation()->GetAnimatedTransformation(4)->GetAnimatedNode(); 
        logger.info << "ADDED SHARK TRANS: " << shark->GetInfo() << logger.end;
    }

    //Predator* sharkPredator = new Predator(shark);
    //engine->ProcessEvent().Attach(*sharkPredator);
    //shark = sharkPredator->GetTransformationNode();
    
//     ISceneNode* test = sharkPredator->GetTransformationNode();
//     test->AddNode(box);
//     scene->AddNode(test);
//     scene->AddNode(sharkAnimRoot);

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
    flockFollow->SetPosition(Vector<3,float>(0,100,-300));
    CircleMover *cm = new CircleMover(flockFollow,Vector<2,float>(100,10),0.7);
    engine->ProcessEvent().Attach(*cm);

     // Setup flock rules.
    flock = new Flock();    
    flock->AddRule(new SeperationRule());
    flock->AddRule(new CohersionRule());
    flock->AddRule(new SpeedRule());
    flock->AddRule(new AlignmentRule());
    flock->AddRule(new FollowRule(flockFollow));
    flock->AddRule(new RandomRule());
    flock->AddRule(new BoxLimitRule(Vector<3,float>(-400,30,-400), 
                                    Vector<3,float>(400,400,400)));
    flock->AddRule(new BoxRule(Vector<3,float>(-400,30,-400),  // The two corners
                               Vector<3,float>(400,400,400))); // - must be axis aligned
    
    flock->AddRule(new FleeRule(human, 100.0, 1.0));
    flock->AddRule(new FleeRule(shark, 150.0, 20.0));

    FlockPropertyReloader *rl = new FlockPropertyReloader(flock, ptree, "flock1");

    vector<ISceneNode*>::iterator itr;
    int size = ptree->GetNode("flock1").GetPath<int>("size", 100);
    for (int i=0;i<size;i++) {
        flock->AddBoid(fish->Clone());
    }
    engine->ProcessEvent().Attach(*flock);
   
    // Add flock to the scene.
    rsn->AddNode(flock->GetRootNode());
    // Add visual rep of the object the flock is following.
//     flock->GetRootNode()->AddNode(flockFollow);
//     flockFollow->AddNode(box->Clone());

    // Static default view
//     Camera* stc  = new Camera(*(new PerspectiveViewingVolume(1, 8000)));
//     camera->SetPosition(Vector<3, float>(0, 54, 0));
//     camera->LookAt(-1000,0,0);
//     camSwitch->AddCamera(stc);
//     setup->SetCamera(*stc);


    // Follow a fish, look at target
    TrackingFollowCamera *tfc = 
        new TrackingFollowCamera(*(new InterpolatedViewingVolume(*(new PerspectiveViewingVolume(1,8000)))));
    tfc->SetPosition(Vector<3,float>(20,20,20));
    tfc->Track(flockFollow);
    tfc->Follow(flock->GetTransformationNode(0));
    camSwitch->AddCamera(tfc);

    // Follow target, look at a fish
    tfc = new TrackingFollowCamera(*(new InterpolatedViewingVolume(*(new PerspectiveViewingVolume(1,8000)))));
    tfc->SetPosition(Vector<3,float>(20,20,20));
    tfc->Follow(flockFollow);
    tfc->Track(flock->GetTransformationNode(0));
    camSwitch->AddCamera(tfc);

    // Follow a fish, look straight
    FollowCamera *fc = new FollowCamera(*(new InterpolatedViewingVolume(*(new PerspectiveViewingVolume(1,8000)))));
    fc->SetDirection(Vector<3,float>(1,0,0),Vector<3,float>(0,1,0));
    fc->SetPosition(Vector<3,float>(10,10,10));
    fc->Follow(flock->GetTransformationNode(0));
    camSwitch->AddCamera(fc);
    
    // Stationary, look at a fish
    TrackingCamera *tc = new TrackingCamera(*(new InterpolatedViewingVolume(*(new PerspectiveViewingVolume(1,8000)))));
    tc->SetPosition(Vector<3,float>(120,150,0));
    tc->Track(flock->GetTransformationNode(0));
    camSwitch->AddCamera(tc);

    // Shark cam
    FollowCamera* fcs = new FollowCamera(*(new InterpolatedViewingVolume(*(new PerspectiveViewingVolume(1,8000)))));
    fcs->SetDirection(Vector<3,float>(1,0,0),Vector<3,float>(0,-1,0));    
    fcs->SetPosition(Vector<3,float>(-150,-80,0));
    fcs->Follow(shark);
    camSwitch->AddCamera(fcs);


    // Shark mouth cam
    FollowCamera* fcsm = new FollowCamera(*(new PerspectiveViewingVolume(1,8000)));
    //FollowCamera* fcs = new FollowCamera(*(new InterpolatedViewingVolume(*(new PerspectiveViewingVolume(1,8000)))));
    fcsm->SetDirection(Vector<3,float>(1,0,0),Vector<3,float>(0,-1,0));    
    fcsm->SetPosition(Vector<3,float>(-12,22,0));
    fcsm->Follow(sharkHead);
    camSwitch->AddCamera(fcsm);
}

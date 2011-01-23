
// main
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// OpenEngine stuff
#include <Animations/Animator.h>
#include <Animations/Animation.h>
#include <Animations/AnimatedTransformation.h>
#include <Animations/Flock.h>
#include <Animations/FlockPropertyReloader.h>
#include <Animations/SeperationRule.h>
#include <Animations/AlignmentRule.h>
#include <Animations/CohersionRule.h>
#include <Animations/SpeedRule.h>
#include <Animations/BoxLimitRule.h>
#include <Animations/BoxRule.h>
#include <Animations/RandomRule.h>
#include <Animations/FleeSphereRule.h>

#include <Core/Engine.h>

#include <Display/SDLEnvironment.h>
#include <Display/PerspectiveViewingVolume.h>
#include <Display/FollowCamera.h>
#include <Display/TrackingCamera.h>
#include <Display/TrackingFollowCamera.h>
#include <Display/InterpolatedViewingVolume.h>
#include <Display/OpenGL/BlendCanvas.h>
#include <Display/OpenGL/TextureCopy.h>

#include <Resources/ResourceManager.h>
#include <Resources/AssimpResource.h>
#include <Renderers/TextureLoader.h>
#include <Renderers/OpenGL/ShaderLoader.h>
#include <Renderers/DataBlockBinder.h>

#include <Scene/SearchTool.h>
#include <Scene/AnimationNode.h>
#include <Scene/SceneNode.h>
#include <Scene/DotVisitor.h>
#include <Scene/DirectionalLightNode.h>
#include <Scene/PointLightNode.h>
#include <Scene/RenderStateNode.h>
#include <Scene/ShadowLightPostProcessNode.h>

#include <Utils/PropertyTree.h>
#include <Utils/PropertyTreeNode.h>

#include <Utils/MoveHandler.h>

// DVA stuff
#include "DVASetup.h"
#include "Utils/UserDefaults.h"
#include "Utils/CustomKeyHandler.h"
#include "Devices/LaserSensor.h"
#include "LaserDebug.h"
#include "InputController.h"
#include "Utils/Stages.h"
#include "CameraSwitcher.h"
#include "HandHeldCamera.h"
#include "LightAnimator.h"

using namespace OpenEngine::Logging;
using namespace OpenEngine::Core;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Display;
using namespace OpenEngine::Display::OpenGL;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Animations;
using namespace dva;


// Global variables used for setup.
IEngine* engine            = NULL;
IEnvironment* env          = NULL;
SimpleSetup* setup         = NULL;
Camera* camera             = NULL;
InputController* inputCtrl = NULL;
IMouse* mouse              = NULL;
IKeyboard* keyboard        = NULL;
RenderStateNode *rsn       = NULL;

vector<ISceneNode*> sceneNodes;

// Model pointers.
ISceneNode* fish              = NULL;
TransformationNode* human     = NULL; 
TransformationNode* shark     = NULL;
TransformationNode* sharkHead = NULL;
TransformationNode* box       = NULL;
ISceneNode* sharkAnimRoot     = NULL;

// Custom stuff
LaserDebugPtr laserDebug;
CameraSwitcher* camSwitch = NULL;
FrameOption frameOption = FRAME_NONE;
FlockPropertyReloader *rl = NULL;

// Forward declarations
void SetupEngine();
void SetupScene();
void SetupDevices();
void SetupBoids();
void LoadResources();

// Helper function.
AnimationNode* GetAnimationNode(ISceneNode* node) {
    SearchTool st;
    return st.DescendantAnimationNode(node);
}


int main(int argc, char** argv) {

    // Parse command line arguments.
    if( argc > 1 && (strcmp(argv[1],"--fullscreen") == 0)){
        frameOption = FRAME_FULLSCREEN;
    }

    // Setup main frame.
    SetupEngine();

    // Setup mouse, keyboard and sensor input devices.
    SetupDevices();

    // Load all model resources.
    LoadResources();

    // Setup scene graph and visual effects.
    SetupScene();

    // Setup flock behaviour rules.
    SetupBoids();

    
    // Update data blocks in case they change due to VBO support.
    DataBlockBinder* bob = new DataBlockBinder(setup->GetRenderer(), 
                                               DataBlockBinder::RELOAD_IMMEDIATE);
    bob->Bind(*setup->GetScene());


    // Generate dot graph representation of the scene.    
    DotVisitor dv;
    ofstream os("graph.dot", ofstream::out);
    dv.Write(*(setup->GetScene()), &os);

    // Start the engine, loop until quit.
    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}


void SetupEngine() {
    // Create SDL environment handling display and input
    env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT, 32, frameOption);

    // Create simple setup
    setup = new SimpleSetup("Det Virtuelle Akvarium", env);

     // Get Engine
    engine = &setup->GetEngine();
}


void SetupDevices() {
   // Setup move handlers
    mouse = env->GetMouse();
    keyboard = env->GetKeyboard();

    // Add main user input controller
    inputCtrl = new InputController();
    inputCtrl->SetInputDevice(mouse);
    inputCtrl->SetInputDevice(keyboard);

    // Setup laser sensor device.
    if( LASER_SENSOR_ENABLED ) {
        LaserSensor* laserSensor = new LaserSensor(LASER_SENSOR_IP, LASER_SENSOR_PORT);
        engine->InitializeEvent().Attach(*laserSensor);
        engine->ProcessEvent().Attach(*laserSensor);
        inputCtrl->SetInputDevice(laserSensor);
        logger.info << "Enabling Laser Sensor Input." << logger.end;

        if( LASER_DEBUG_ENABLED ) {
            // Setup screen overlay to visualise laser sensor readings.
            laserDebug = LaserDebug::Create(SCREEN_WIDTH, SCREEN_HEIGHT);
            laserSensor->SetLaserDebug(laserDebug);
            setup->GetTextureLoader().Load(laserDebug, Renderers::TextureLoader::RELOAD_IMMEDIATE);
            logger.info << "Enabling Laser Sensor Input Visualiser." << logger.end;
        }
    }



    inputCtrl->SetMode(INPUT_CTRL_MODE);
    engine->InitializeEvent().Attach(*inputCtrl);
    engine->ProcessEvent().Attach(*inputCtrl);
    engine->DeinitializeEvent().Attach(*inputCtrl);
    
    // Static default view
    Camera* stc  = new Camera(*(new PerspectiveViewingVolume(1, 8000)));
    stc->SetPosition(Vector<3, float>(0, 56, 0));
    stc->LookAt(0,190,-2000);
    setup->SetCamera(*stc);

    // TEST Hand-held camera.
    HandHeldCamera* hhc = new HandHeldCamera(stc);
    engine->InitializeEvent().Attach(*hhc);
    engine->ProcessEvent().Attach(*hhc);

    // CameraSwitcher adds the current cam from setup
    camSwitch = new CameraSwitcher(setup); 
    keyboard->KeyEvent().Attach(*camSwitch);

    // Add movable camera
//     camera  = new Camera(*(new PerspectiveViewingVolume(1, 8000)));
//     camera->SetPosition(Vector<3, float>(0.0, 54.0, 0.0));
//     camera->LookAt(0,190,-2000);
//     camSwitch->AddCamera(camera);
//     MoveHandler* move = new MoveHandler(*camera, *mouse);
//     move->SetMoveScale(0.001);
//     engine->InitializeEvent().Attach(*move);
//     engine->ProcessEvent().Attach(*move);
//     keyboard->KeyEvent().Attach(*move);

    CustomKeyHandler* ckh = new CustomKeyHandler(*setup);
    keyboard->KeyEvent().Attach(*ckh);
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
    boxTrans->SetScale(Vector<3,float>(0.1));
    boxTrans->AddNode(boxModel->GetSceneNode());
    box = boxTrans;

    // Load model representing human interaction.
    path = DirectoryManager::FindFileInPath("models/box/box.dae");
    IModelResourcePtr humanModel = ResourceManager<IModelResource>::Create(path);
    humanModel->Load();
    human = new TransformationNode();
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

    // SearchTool st;
    // TransformationNode* hest = st.DescendantTransformationNode(fishModel);
    // while (hest->GetNumberOfNodes() > 0) {
    //     ISceneNode* n = hest->GetNode(0);
    //     hest->RemoveNode(n);
    //     fishModel->AddNode(n);
    // }
    // fishModel->RemoveNode(hest);

    AnimationNode* animations = GetAnimationNode(fishModel);
    if( animations ){
        Animator* animator = new Animator(animations);
        UserDefaults::GetInstance()->map["Animator"] = animator;
        if( animator->GetSceneNode() ){
            TransformationNode* fishTrans = new TransformationNode();
            //TransformationNode* fishTrans = hest;
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
    if( LASER_SENSOR_ENABLED && LASER_DEBUG_ENABLED )
        b->AddTexture(laserDebug, 0, 0, Vector<4,float>(1.0, 1.0, 1.0, 1.0));
    b->InitCanvas(setup->GetCanvas());

    Stages* stages = new Stages(setup->GetFrame(), setup->GetTextureLoader(), b);
    engine->InitializeEvent().Attach(*stages);
    engine->DeinitializeEvent().Attach(*stages);
    engine->ProcessEvent().Attach(*stages);

    // Apply Heads Up Display
//     string path = DirectoryManager::FindFileInPath("textures/hud/hud_default.tga");
//     ITexture2DPtr hud = ResourceManager<ITextureResource>::Create(path);
//     setup->GetTextureLoader().Load(hud);
//     b->AddTexture(hud, 0, 0, Vector<4,float>(1.0,1.0,1.0,0.5));
    
    // Start by setting the root node in the scene graph.
    ISceneNode* sceneRoot = new SceneNode();
    setup->SetScene(*sceneRoot);

    // Set background color to white
    setup->GetRenderer().SetBackgroundColor(Vector<4,float>(1.0,1.0,1.0,1.0));

    // scene represents where to insert next node.
    ISceneNode* scene = sceneRoot;
    Vector<2, int> dimension(SCREEN_WIDTH, SCREEN_HEIGHT);
 
    // Create fog post process   
    IShaderResourcePtr fog = ResourceManager<IShaderResource>::Create("projects/dva/effects/fog.glsl");
    PostProcessNode* fogNode = new PostProcessNode(fog, dimension); 
    setup->GetRenderer().InitializeEvent().Attach(*fogNode);
    scene->AddNode(fogNode); 
    scene = fogNode;

    //Create Shadow post process
    IShaderResourcePtr shadow = ResourceManager<IShaderResource>::Create("projects/dva/effects/shadowmap.glsl");
    ShadowLightPostProcessNode* shadowPost = 
        new ShadowLightPostProcessNode(shadow, 
                                       dimension,
                                       //dimension
                                       Vector<2,int>(1024,1024));
    setup->GetRenderer().InitializeEvent().Attach(*shadowPost);

    scene->AddNode(shadowPost); 
    scene = shadowPost;

    // TODO adjust shadow camera...
    IViewingVolume* shadowView = new PerspectiveViewingVolume(100,2000);
    Camera* shadowCam = new Camera(*(shadowView));
    shadowCam->SetPosition(Vector<3,float>(0,800,500));
    shadowCam->LookAt(Vector<3,float>(0,0,-1000));
    camSwitch->AddCamera(shadowCam);    
    shadowPost->SetViewingVolume(shadowCam);


    // Create caustics post process
    IShaderResourcePtr caustics = ResourceManager<IShaderResource>::Create("projects/dva/effects/caustics.glsl");
    caustics->SetUniform("lightDir", Vector<3, float>(0, -1, 0));
    PostProcessNode* causticsNode = new PostProcessNode(caustics, dimension); 
    setup->GetRenderer().InitializeEvent().Attach(*causticsNode);
    scene->AddNode(causticsNode); 
    scene = causticsNode;

    // Create point light
//     TransformationNode* lightTrans = new TransformationNode();
//     PointLightNode* lightNode = new PointLightNode();
//     lightTrans->SetPosition(Vector<3,float>(0.0,100.0,0.0));
//     lightNode->ambient = Vector<4,float>(0.6,0.8,0.5,1.0);
//     lightNode->ambient = Vector<4,float>(0.4,0.4,0.4,1.0);
//     lightNode->diffuse = Vector<4,float>(0.0,1.0,0.0,1.0);
//     lightNode->specular = Vector<4,float>(.2,.2,.2,1.0);
//     lightNode->linearAtt = 0.01;
//     scene->AddNode(lightTrans);
//     lightTrans->AddNode(lightNode);

    TransformationNode* lightTrans1 = new TransformationNode();
    // lightTrans1->SetRotation(Quaternion<float>(Math::PI, Vector<3,float>(1.0,0.0,0.0)));
    lightTrans1->SetPosition(Vector<3,float>(0.0, 100.0,0.0));
    PointLightNode* lightNode1 = new PointLightNode();
    // lightNode1->ambient = Vector<4,float>(0.6,0.8,0.5,1.0);
    lightNode1->ambient = Vector<4,float>(0.4,0.4,0.4,1.0);
    lightNode1->diffuse = Vector<4,float>(0.7,0.7,0.7,1.0);
    lightNode1->specular = Vector<4,float>(0.5,0.5,0.5,1.0);
    lightNode1->linearAtt = 0.001;
    lightNode1->constAtt = 1.0;
    scene->AddNode(lightTrans1);
    lightTrans1->AddNode(lightNode1);

    LightAnimator* lightAnim = new LightAnimator(lightNode1);
    engine->ProcessEvent().Attach(*lightAnim);

    rsn = new RenderStateNode();
    rsn->DisableOption(RenderStateNode::BACKFACE);
    rsn->EnableOption(RenderStateNode::LIGHTING);
    rsn->EnableOption(RenderStateNode::COLOR_MATERIAL);
    rsn->EnableOption(RenderStateNode::SHADER);
    scene->AddNode(rsn);
    scene = rsn;

    // Add all scene nodes.
    vector<ISceneNode*>::iterator itr;
    for(itr=sceneNodes.begin(); itr!=sceneNodes.end(); itr++){
        ISceneNode* node = *itr;
        scene->AddNode(node);
    }

    // Just for debug
//     GridNode* grid = new GridNode(100, 10, Vector<3,float>(0.5, 0.5, 0.5));
//     scene->AddNode(grid);
}

void SetupBoids() {
    string confPath = DirectoryManager::FindFileInPath("boids.yaml");
    PropertyTree* ptree = new PropertyTree(confPath);    

    // engine->InitializeEvent().Attach(*ptree);
    engine->ProcessEvent().Attach(*ptree);
    // engine->DeinitializeEvent().Attach(*ptree);

     // Setup flock rules.
    Flock* flock = new Flock();    
    flock->AddRule(new SeperationRule());
    flock->AddRule(new CohersionRule());
    flock->AddRule(new SpeedRule());
    flock->AddRule(new AlignmentRule());
    flock->AddRule(new RandomRule());
    flock->AddRule(new BoxLimitRule(Vector<3,float>(-400,30,-400), 
                                    Vector<3,float>(400,400,400)));
    flock->AddRule(new BoxRule(Vector<3,float>(-400,30,-400),  // The two corners
                               Vector<3,float>(400,400,400))); // - must be axis aligned
    
    // Locate shark transformation node and add a flee rule to the boid system.
    SearchTool search;
    std::list<AnimationNode*> animNodeRes;
    animNodeRes = search.DescendantAnimationNodes(sharkAnimRoot);
    if( animNodeRes.size() > 0 ){
        shark     = animNodeRes.front()->GetAnimation()->GetAnimatedTransformation(0)->GetAnimatedNode();
        sharkHead = animNodeRes.front()->GetAnimation()->GetAnimatedTransformation(4)->GetAnimatedNode(); 
    }
    flock->AddRule(new FleeSphereRule(sharkHead, 100.0, 20.0));

    // Set flock in input controller, enabling user interactions.
    inputCtrl->SetFlock(flock);
    inputCtrl->SetDebugMesh(box);
    rsn->AddNode(inputCtrl->GetSceneNode());

    // Set Flock property reloader listening on changes in boid.yaml
    rl = new FlockPropertyReloader(flock, ptree, "flock1");

    vector<ISceneNode*>::iterator itr;
    int size = ptree->GetRootNode()->GetNode("flock1")->GetPath("size", 100);
    RandomGenerator random;
    Vector<3,float> offset(100, 0, -300);
    for (int i=0;i<size;i++) {
        Boid* b = new Boid(fish->Clone(), &random);
        Vector<3,float> pos = b->GetPosition() + offset;
        b->SetPosition(pos);
        flock->AddBoid(b);
    }
    engine->ProcessEvent().Attach(*flock);
    // Add flock to the scene.
    rsn->AddNode(flock->GetRootNode());

    // Follow a fish, look at target
    TrackingFollowCamera *tfc = 
        new TrackingFollowCamera(*(new InterpolatedViewingVolume(*(new PerspectiveViewingVolume(1,8000)))));
    tfc->SetPosition(Vector<3,float>(20,20,20));
    tfc->Follow(flock->GetTransformationNode(0));
    camSwitch->AddCamera(tfc);

    // Follow target, look at a fish
    tfc = new TrackingFollowCamera(*(new InterpolatedViewingVolume(*(new PerspectiveViewingVolume(1,8000)))));
    tfc->SetPosition(Vector<3,float>(20,20,20));
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

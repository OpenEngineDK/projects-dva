
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
#include <Animations/SeparationRule.h>
#include <Animations/MultiGotoRule.h>
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

#include <Utils/BetterMoveHandler.h>

#include <Display/AntTweakBar.h>
#include <Utils/PropertyBar.h>

// Sound 
#include <Sound/OpenALSoundSystem.h>
#include <Sound/MusicPlayer.h>
#include <Resources/VorbisResource.h>

// DVA stuff
#include "DVASetup.h"
#include "Utils/UserDefaults.h"
#include "Utils/CustomKeyHandler.h"
#include "Devices/LaserSensor.h"
#include "RelayBox.h"
#include "LaserDebug.h"
#include "InputController.h"
#include "Utils/Stages.h"
#include "CameraSwitcher.h"
#include "HandHeldCamera.h"
#include "LightAnimator.h"
#include "ScreenplayController.h"


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
using namespace OpenEngine::Sound;
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
ISoundSystem* soundsystem  = NULL;
MusicPlayer* musicplayer   = NULL;

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

PropertyTree* ptree;
AntTweakBar *atb;



// Setup Screenplay controller handling the sequence of events.
ScreenplayController* screenplayCtrl;

// Forward declarations
void SetupEngine();
void SetupDevices();
void LoadResources();
void SetupSound();
void SetupScene();
void SetupBoids();
Animator* LoadAnimatedModel(string filename);

class DebugKeyHandler : public IListener<KeyboardEventArg> {
    AntTweakBar* bar;
public:
    DebugKeyHandler(AntTweakBar* bar, bool enabled=true) : bar(bar) {
        if (!enabled)
            bar->ToggleEnabled();
    }
    void Handle(KeyboardEventArg arg) {
        if (arg.type != EVENT_RELEASE)
            return;
        
        if (arg.sym == KEY_F5)
            bar->ToggleEnabled();
    }
};


// Helper function.
AnimationNode* GetAnimationNode(ISceneNode* node) {
    SearchTool st;
    return st.DescendantAnimationNode(node);
}

class LightNodeChanged : public IListener<PropertiesChangedEventArg> {
private:
    PointLightNode* lightNode;
public:
    LightNodeChanged(PointLightNode* pln) : lightNode(pln) {}
    ~LightNodeChanged() {}

    void Handle(PropertiesChangedEventArg arg){
        //        float val = arg.GetNode()->Get(0.0f);
        logger.info << "Properties changed event" << logger.end;
    }
};


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

    // Load sound system
    SetupSound();

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
    //
    string confPath = DirectoryManager::FindFileInPath("projects/dva/boids.yaml");
    ptree = new PropertyTree(confPath);    

    PropertyTreeNode* screenConf = ptree->GetRootNode()->GetNode("screen");
    SCREEN_WIDTH = screenConf->GetPath("width",SCREEN_WIDTH);
    SCREEN_HEIGHT = screenConf->GetPath("height",SCREEN_HEIGHT);
    

    // Create SDL environment handling display and input
    env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT, 32, frameOption);

    // Create simple setup
    setup = new SimpleSetup("Det Virtuelle Akvarium", env);

     // Get Engine
    engine = &setup->GetEngine();

    atb = new AntTweakBar();
    mouse = env->GetMouse();
    keyboard = env->GetKeyboard();

    DebugKeyHandler *dbg = new DebugKeyHandler(atb, false);
    keyboard->KeyEvent().Attach(*dbg);

    keyboard->KeyEvent().Attach(*atb);
    mouse->MouseMovedEvent().Attach(*atb);
    mouse->MouseButtonEvent().Attach(*atb);
    atb->AttachTo(setup->GetRenderer());
    atb->AddBar(new PropertyBar("ptree",ptree, ptree->GetRootNode()->GetNode("meta")));

}



void SetupDevices() {
   // Setup move handlers

    mouse = env->GetMouse();
    keyboard = env->GetKeyboard();

    // Add main user input controller
    inputCtrl = new InputController();
    inputCtrl->SetInputDevice(mouse);
    inputCtrl->SetInputDevice(keyboard);
    engine->InitializeEvent().Attach(*inputCtrl);
    engine->ProcessEvent().Attach(*inputCtrl);
    engine->DeinitializeEvent().Attach(*inputCtrl);

    // 
    screenplayCtrl = new ScreenplayController();
    engine->InitializeEvent().Attach(*screenplayCtrl);
    engine->ProcessEvent().Attach(*screenplayCtrl);

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

    // Configure Relay box controlling wind, water and room light.
    if( RELAY_BOX_ENABLED ) {
        RelayBox* relayBox = new RelayBox(RELAY_BOX_IP, RELAY_BOX_PORT);
        relayBox->Start();
        screenplayCtrl->SetRelayBox(relayBox);
    }

    // Static default view
    Camera* stc  = new Camera(*(new PerspectiveViewingVolume(1, 8000)));
    stc->SetPosition(Vector<3, float>(0, 54, 0));
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
    camera  = new Camera(*(new PerspectiveViewingVolume(1, 8000)));
    camera->SetPosition(Vector<3, float>(0.0, 54.0, 0.0));
    camera->LookAt(0,190,-2000);
    camSwitch->AddCamera(camera);

    BetterMoveHandler* move = new BetterMoveHandler(*camera, *mouse, true);
    move->SetInverted(true);
    move->SetMoveScale(0.001);
    atb->MouseMovedEvent().Attach(*move);
    atb->MouseButtonEvent().Attach(*move);
    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    keyboard->KeyEvent().Attach(*move);

    CustomKeyHandler* ckh = new CustomKeyHandler(*setup);
    keyboard->KeyEvent().Attach(*ckh);
}


void LoadResources() {
    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());
    // ResourceManager<ISoundResource>::AddPlugin(new VorbisResourcePlugin());
    ResourceManager<IStreamingSoundResource>::AddPlugin(new StreamingVorbisResourcePlugin());
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
    path = DirectoryManager::FindFileInPath("models/environment/Environment04.DAE");
    IModelResourcePtr envModel = ResourceManager<IModelResource>::Create(path);
    envModel->Load();
    ISceneNode* env = envModel->GetSceneNode();
    env->SetInfo("Environment Model\n[ISceneNode]");
    TransformationNode* envTrans = new TransformationNode();
    envTrans->AddNode(env);
    sceneNodes.push_back(envTrans);

    // Load Seaweed
    path = DirectoryManager::FindFileInPath("models/seaweed/Seaweed02.DAE");

 //    Animator* seaweed0 = LoadAnimatedModel(path);
//     Animator* seaweed1 = LoadAnimatedModel(path);
//     Animator* seaweed2 = LoadAnimatedModel(path);

//    seawee0->SetSpeed(0.8);

    IModelResourcePtr seaweedModel = ResourceManager<IModelResource>::Create(path);
    seaweedModel->Load();
    ISceneNode* weed = seaweedModel->GetSceneNode();
    weed->SetInfo("Seaweed Model\n[ISceneNode]");

    AnimationNode* weedAnim = GetAnimationNode(weed);
    if( weedAnim ){
        Animator* animator = new Animator(weedAnim);
        UserDefaults::GetInstance()->map["WeedAnimator"] = animator;
        if( animator->GetSceneNode() ){
            TransformationNode* weedTrans = new TransformationNode();
            weedTrans->AddNode(animator->GetSceneNode());
            weedTrans->SetPosition(Vector<3,float>(0,0,0));
            sceneNodes.push_back(weedTrans);
        }
        setup->GetEngine().ProcessEvent().Attach(*animator);
        animator->SetActiveAnimation(0);
        //animator->SetSpeed(1.0);
        animator->Play();
     }

    // Load shark.
    path = DirectoryManager::FindFileInPath("models/sharky/Sharky09.DAE");
    IModelResourcePtr sharkModel = ResourceManager<IModelResource>::Create(path);
    sharkModel->Load();
    ISceneNode* sharky = sharkModel->GetSceneNode();
    sharky->SetInfo("Sharky the not so friendly shark\n[ISceneNode]");
    AnimationNode* sharkAnim = GetAnimationNode(sharky);
    if( sharkAnim ){
        Animator* sharkAnimator = new Animator(sharkAnim);
        UserDefaults::GetInstance()->map["SharkAnimator"] = sharkAnimator;
        if( sharkAnimator->GetSceneNode() ){
            sharkAnimRoot = sharkAnim;
            TransformationNode* sharkTrans = new TransformationNode();
            sharkTrans->SetInfo("SHARK TRANSFORMATION");
            sharkTrans->AddNode(sharkAnimator->GetSceneNode());
            sceneNodes.push_back(sharkTrans);

            // Locate shark transformation node and add a flee rule to the boid system.
            SearchTool search;
            std::list<AnimationNode*> animNodeRes;
            animNodeRes = search.DescendantAnimationNodes(sharkAnimRoot);
            if( animNodeRes.size() > 0 ){
                shark     = animNodeRes.front()->GetAnimation()->GetAnimatedTransformation(0)->GetAnimatedNode();
                sharkHead = animNodeRes.front()->GetAnimation()->GetAnimatedTransformation(4)->GetAnimatedNode(); 
            }

        }
        setup->GetEngine().ProcessEvent().Attach(*sharkAnimator);
        sharkAnimator->SetActiveAnimation(0);
        sharkAnimator->LoopAnimation(false);
        //sharkAnimator->Play();
        screenplayCtrl->SetSharkAnimator(sharkAnimator);
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


void SetupSound() {
    soundsystem = new OpenALSoundSystem();
    soundsystem->SetDevice(0);
    musicplayer = new MusicPlayer(NULL, soundsystem);
    bool enableSound = false;
    if (enableSound) {
        // setup the sound system
        soundsystem->SetMasterGain(1.0);
        engine->InitializeEvent().Attach(*soundsystem);
        engine->ProcessEvent().Attach(*soundsystem);
        engine->DeinitializeEvent().Attach(*soundsystem);

        // setup the music player
        musicplayer->SetGain(1.0);
        
        musicplayer->AddSound("jaws.ogg");

        musicplayer->Shuffle(true);
        //musicplayer->Next();
        musicplayer->Play();
        engine->ProcessEvent().Attach(*musicplayer);

        setup->GetRenderer().PreProcessEvent().Attach(*soundsystem);
    }
}


void SetupScene() {
    // Hook Screenplay controller into engine.
    engine->ProcessEvent().Attach(*screenplayCtrl);

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

    // Set background color to "white"
    setup->GetRenderer().SetBackgroundColor(Vector<4,float>(8.0, 1.0, 0.8, 1.0));

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

    // Setup shadow perspective
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

//    Create point light
//     TransformationNode* lightTrans = new TransformationNode();
//     PointLightNode* lightNode = new PointLightNode();
//     lightTrans->SetPosition(Vector<3,float>(0.0,100.0,0.0));
//     lightNode->ambient  = Vector<4,float>(0.6, 0.5, 0.5, 1.0);
//     lightNode->ambient  = Vector<4,float>(0.4, 0.4, 0.4, 1.0);
//     lightNode->diffuse  = Vector<4,float>(0.0, 0.3, 0.0, 1.0);
//     lightNode->specular = Vector<4,float>(0.2, 0.2, 0.2, 1.0);
//     lightNode->linearAtt = 0.01;
//     scene->AddNode(lightTrans);
//     lightTrans->AddNode(lightNode);

//     LightNodeChanged* lnc = new LightNodeChanged(lightNode);
//     ptree->PropertiesChangedEvent().Attach(*lnc);

    TransformationNode* lightTrans1 = new TransformationNode();
    // lightTrans1->SetRotation(Quaternion<float>(Math::PI, Vector<3,float>(1.0,0.0,0.0)));
    lightTrans1->SetPosition(Vector<3,float>(0.0, 100.0,0.0));
    PointLightNode* lightNode1 = new PointLightNode();
    // lightNode1->ambient = Vector<4,float>(0.6,0.8,0.5,1.0);
    lightNode1->ambient  = Vector<4,float>(0.4, 0.9, 0.4, 1.0);
    lightNode1->diffuse  = Vector<4,float>(0.7, 0.8, 0.7, 1.0);
    lightNode1->specular = Vector<4,float>(0.5, 0.6, 0.5, 1.0);
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

    // engine->InitializeEvent().Attach(*ptree);
    engine->ProcessEvent().Attach(*ptree);
    // engine->DeinitializeEvent().Attach(*ptree);

     // Setup flock rules.
    Flock* flock = new Flock();
    flock->AddRule(new SeparationRule());
    flock->AddRule(new CohersionRule());
    flock->AddRule(new SpeedRule());
    flock->AddRule(new AlignmentRule());
    flock->AddRule(new RandomRule());
        {
        MultiGotoRule* mgr = new MultiGotoRule();
        // Lets generate a logo || OpenEngine
        // int array[] = {0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        //                1,0,0,0,1,0,1,1,0,0,1,1,1,0,1,0,1,0,2,0,0,0,1,0,1,0,1,1,1,0,0,1,0,0,1,0,1,0,1,1,1,
        //                1,0,0,0,1,0,1,0,1,0,1,0,0,0,2,0,1,0,2,1,1,0,2,0,1,0,1,0,0,0,0,0,0,0,2,0,1,0,1,0,0,
        //                1,0,0,0,1,0,1,1,0,0,1,1,1,0,1,1,1,0,2,1,1,0,1,1,1,0,1,1,0,0,0,1,0,0,1,1,1,0,1,1,1,
        //                1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,2,0,2,0,0,0,1,0,2,0,1,0,1,0,0,1,0,0,1,0,2,0,1,0,0,
        //                0,1,1,1,0,0,1,0,0,0,1,1,1,0,1,0,1,0,2,2,2,0,1,0,1,0,1,1,1,0,0,1,0,0,1,0,1,0,1,1,1
        // };
        int array[] = {0,1,1,1,0,0,2,2,2,2,
                       1,0,0,0,1,0,2,0,0,0,
                       1,0,0,0,1,0,2,1,1,1,
                       1,0,0,0,1,0,2,1,1,1,
                       1,0,0,0,1,0,2,0,0,0,
                       0,1,1,1,0,0,2,2,2,2
        };

        int elms = sizeof(array)/sizeof(int);
        int h = 6;
        int w = elms/h;
        float maxW = 200;
        float maxH = 200;

        Vector<3,float> startP(-80,maxH-30,-300);
        //Vector<3,float> delta(maxW/w,maxH/h,0);
        //Vector<3,float> delta(maxw/h,maxH/h,0);
        int min = std::min(maxW/w, maxH/h);
        Vector<3,float> delta(min,min,0);
        
        //Vector<3,float> delta(40,40,0);
        for (int i=0; i<h; i++) {
            for (int j=0; j<w; j++) {
                int idx = i*w+j;
                int v = array[idx];
                Vector<3,float> p = startP;
                p[0] += delta[0]*j;
                p[1] -= delta[1]*i;
                for (int k=0; k<v; k++)
                    mgr->AddPosition(p);
                
            }
        }

        flock->AddRule(mgr);
    }// done
    flock->AddRule(new BoxLimitRule(Vector<3,float>(-400,30,-400), 
                                    Vector<3,float>(400,400,400)));

    flock->AddRule(new BoxRule(Vector<3,float>(-400,30,-400),  // The two corners
                               Vector<3,float>(400,400,400))); // - must be axis aligned
    
    flock->AddRule(new FleeSphereRule(sharkHead, 100.0, 20.0));
    
    flock->SetPropertyNode(ptree->GetRootNode()->GetNode("flock1"));

    // Set flock in input controller, enabling user interactions.
    inputCtrl->SetFlock(flock);

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


Animator* LoadAnimatedModel(string path){
    IModelResourcePtr model = ResourceManager<IModelResource>::Create(path);
    model->Load();
    AnimationNode* animNode = GetAnimationNode(model->GetSceneNode());
    Animator* animator = NULL;
    if( animNode ){
        animator = new Animator(animNode);
        setup->GetEngine().ProcessEvent().Attach(*animator);
        animator->SetActiveAnimation(0);
        animator->Play();
     }
    return animator;
}

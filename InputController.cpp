// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include "InputController.h"

#include <Devices/IMouse.h>
#include <Scene/SceneNode.h>
#include <Animations/Flock.h>
#include <Animations/Boid.h>
#include <Animations/FollowRule.h>
#include <Animations/FleeSphereRule.h>
#include <Animations/FleeCylinderRule.h>
#include <Animations/SeparationRule.h>
#include <Logging/Logger.h>
#include "CylinderNode.h"
#include "DVASetup.h"
#include "RuleHandlers/FleeRuleHandler.h"
#include "RuleHandlers/SeparationRuleHandler.h"
#include "RuleHandlers/FlockFollowCircleRuleHandler.h"

#include <vector>

Vector<3,float> cylinderP0(0,54,0);
Vector<3,float> cylinderP1(0,190,-8000);
float CYLINDER_RADIUS = 25.0f;
float SCARE_FACTOR = 10.0f;

using namespace OpenEngine::Animations;
using namespace std;

namespace dva {


void InputController::Init() {
    laser = NULL;
    keyboard = NULL;
    mouse = NULL;
    numMousePoints = 0;
    numLaserPoints = 0;
    flock = NULL;
}

InputController::InputController() {
    Init();
}


InputController::InputController(LaserSensor* sensor) {
    Init();
    this->laser = sensor;
}

InputController::InputController(IKeyboard* keyboard) {
    Init();
    this->keyboard = keyboard;
}

InputController::InputController(IMouse* mouse) {
    Init();
    this->mouse = mouse;
}

InputController::~InputController() {}

void InputController::SetInputDevice(LaserSensor* sensor) {
    this->laser = sensor;
}

void InputController::SetInputDevice(IKeyboard* keyboard) {
    this->keyboard = keyboard;
}

void InputController::SetInputDevice(IMouse* mouse) {
    this->mouse = mouse;
}

void InputController::SetFlock(Flock* flock) {
    this->flock = flock;
}

// Initialize devices
void InputController::Handle(Core::InitializeEventArg arg) {
    // Add flock follow rule.
    TransformationNode* trans = new TransformationNode();
    trans->SetPosition(Vector<3,float>(0,100,-300));
    followCircle = new FlockFollowCircleRuleHandler(flock, trans, Vector<2,float>(120,50),0.7); 
    flock->AddRule(followCircle->GetRule());
}

void InputController::Handle(Core::ProcessEventArg arg) {

    // Clear list of tracking points.
    mousePoints.clear();
    laserPoints.clear();

    // Handle input
    if( mouse ) HandleMouseInput();
    if( laser ) HandleLaserInput();
    //    logger.info << "LASER POINTS: " << laserPoints.size() << logger.end;

    unsigned int numTrackingPoints = laserPoints.size();

    // Check if number of tracking points have changed since last processing.
    if( numLaserPoints != numTrackingPoints ){
        // Remove all existing laser controlled rules.
        vector<IRuleHandler*>::iterator itr;
        for(itr=ruleHandlers.begin(); itr!=ruleHandlers.end(); itr++){
            delete *itr;
        }
        ruleHandlers.clear();

        
        // Add new set of rules according to number of tracking points.
        if( numTrackingPoints > 0 ){
            SetupRules();
        }
        
        // Set current number of tracking points.
        numLaserPoints = laserPoints.size();
    }


    if( numTrackingPoints > 0 ){
        UpdateRules();
    }

    // Update flock follow rule.
    followCircle->Handle(arg);
}


void InputController::SetupRules() {
    // Setup rules based on number of laser tracking points.
    switch( laserPoints.size() ) {
    case 1:
        {
            TransformationNode* trans = new TransformationNode();
            trans->SetPosition(Vector<3,float>(0,0,-300));
            IRuleHandler* flee = new FleeRuleHandler(new FleeSphereRule(trans, 100.0, 10.0), flock);
            ruleHandlers.push_back(flee);
        }
        break;

    case 2:
        {
            IRuleHandler* separation = new SeparationRuleHandler(flock);
            ruleHandlers.push_back(separation);
        }
        break;

    case 3:
        {
            IRuleHandler* separation = new SeparationRuleHandler(flock);
            ruleHandlers.push_back(separation);

            TransformationNode* trans = new TransformationNode();
            trans->SetPosition(Vector<3,float>(0,0,-300));
            IRuleHandler* flee = new FleeRuleHandler(new FleeSphereRule(trans, 100.0, 10.0), flock);
            ruleHandlers.push_back(flee);
        }
        break;

    default:
         break;
    }
}


void InputController::UpdateRules() {
    // Remove all existing laser controlled rules.
    vector<IRuleHandler*>::iterator itr;
    for(itr=ruleHandlers.begin(); itr!=ruleHandlers.end(); itr++){
        (*itr)->HandleInput(laserPoints);
    }
}

void InputController::Handle(Core::DeinitializeEventArg arg) {
}


void InputController::HandleMouseInput(){
    Devices::MouseState arg = mouse->GetState();
    mousePoints.push_back(Vector<2,int>(arg.x, arg.y));
    laserPoints.push_back(Vector<2,int>(arg.x, arg.y));
}


void InputController::HandleLaserInput() {
    // Get laser readings.
    vector< Vector<2,float> > trackingPoints = laser->GetState();
    for( unsigned int i=0 ; i<trackingPoints.size(); i++ ){
        laserPoints.push_back(LaserPointToScreenCoordinates(trackingPoints[i]));
    }
}

Vector<3,float> InputController::ScreenToSceneCoordinates(int x, int y) {
    float xRange = 370.0f;
    float yRange = 110.0f;
    float yTop = 170.0;

    Vector<3,float> pos;
    pos[0] = (-xRange/2.0f) + ((x / (float)SCREEN_WIDTH) * xRange);
    pos[1] = yTop - ((y / (float)SCREEN_HEIGHT) * yRange);
    return pos;
}

Vector<2,int> InputController::LaserPointToScreenCoordinates(Vector<2,float> p) {
    Vector<2,int> point;
    point[0] = ((p[0]+1)/2.0) * SCREEN_WIDTH;
    point[1] = SCREEN_HEIGHT - ((p[1]+1)/2.0) * SCREEN_HEIGHT;
    return point;
}


} // NS dva


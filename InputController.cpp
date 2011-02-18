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
#include <Utils/PropertyBinder.h>
#include "CylinderNode.h"
#include "DVASetup.h"
#include "RuleHandlers/FleeRuleHandler.h"
#include "RuleHandlers/SeparationRuleHandler.h"
#include "RuleHandlers/FlockFollowCircleRuleHandler.h"
#include <vector>

#include <Core/Exceptions.h>

using namespace OpenEngine::Animations;
using namespace std;

namespace dva {


void InputController::Init() {
    ptNode = NULL;
    laser = NULL;
    keyboard = NULL;
    mouse = NULL;
    numMousePoints = 0;
    numLaserPoints = 0;
    flock = NULL;
    followCircleRule = NULL;
    fleeRule = NULL;
    separationRule = NULL;
    oe = NULL;
    oeLastVal = 0;
    oeTabs = 0;
    oeRunning = false;
}

InputController::InputController(PropertyTreeNode* ptNode) {
    Init();
    this->ptNode = ptNode;
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
    // Hide the mouse cursor.
    if( mouse )  mouse->HideCursor();

    // Add flock follow rule.
    TransformationNode* followTrans = new TransformationNode();
    followTrans->SetPosition(Vector<3,float>(0,100,-300));
    followCircleRule = new FlockFollowCircleRuleHandler(flock, followTrans, Vector<3,float>(120,0,20),0.7);
    new Utils::PropertyBinder<FlockFollowCircleRuleHandler,Vector<3,float> >
        (ptNode->GetNodePath("input.circle"), 
        *(FlockFollowCircleRuleHandler*)followCircleRule,
        &FlockFollowCircleRuleHandler::SetCircle,
        Vector<3,float>(120,0,20));

    followCircleRule->GetRule()->SetEnabled(false);
    ruleHandlers.push_back(followCircleRule);

    // Add flee rule, disabled by default.
    TransformationNode* fleeTrans = new TransformationNode();
    fleeTrans->SetPosition(Vector<3,float>(0,0,-300));    
    fleeRule = new FleeRuleHandler(new FleeSphereRule(fleeTrans, 100.0, 10.0), flock);
    fleeRule->GetRule()->SetEnabled(false);
    ruleHandlers.push_back(fleeRule);

    // Add separation rule, disabled by default.
    separationRule = new SeparationRuleHandler(flock);
    separationRule->GetRule()->SetEnabled(false);
    ruleHandlers.push_back(separationRule);

    // Get oe rule.
    oe = flock->GetRuleNamed("MultiGoto");
    if( oe ) timer.Start();
}

void InputController::Handle(Core::ProcessEventArg arg) {
    // Clear list of tracking points.
    mousePoints.clear();
    laserPoints.clear();

    // Handle input
    if( mouse ) HandleMouseInput();
    if( laser ) HandleLaserInput();
    //    logger.info << "LASER POINTS: " << laserPoints.size() << logger.end;

    if( oe ) {
        
        if( laserPoints.size() == 1 && oeLastVal == 0 && oeTabs < 42 ) {
            float elaps = timer.GetElapsedTime().sec*1000000 + timer.GetElapsedTime().usec;
            if( elaps > 100000 && elaps < 1000000 ){
                oeTabs++;
                oeLastVal = laserPoints.size();
                timer.Reset();
                timer.Start();
            } else {
                timer.Reset();
                timer.Start();
            }

            if( elaps > 3000000 ){
                oeTabs = 0;
            }

            //logger.info << "Tabs: " << oeTabs << " elaps: " << elaps << logger.end;
        }else if( laserPoints.size() == 0 ){
            oeLastVal = 0;
        }

        if( !oeRunning && oeTabs >= 42 ){
            oe->SetEnabled(true);
            timer.Reset();
            timer.Start();
            oeRunning = true;
        }
        if( oeRunning && timer.GetElapsedTime().sec > 5.0 ){
            oe->SetEnabled(false);
            oeTabs = 0;
            oeRunning = false;
        }
    }

    SetupRules();
    
    UpdateRules(arg);
}


void InputController::SetupRules() {
    // By default disable all rules.
    vector<IRuleHandler*>::iterator itr;
    for(itr=ruleHandlers.begin(); itr!=ruleHandlers.end(); itr++){
        (*itr)->GetRule()->SetEnabled(false);
    }    
    // Always enable circle follower.
    followCircleRule->GetRule()->SetEnabled(true);

    // Setup rules based on number of laser tracking points.
    switch( laserPoints.size() ) {
    case 0:
        break;

    case 1: 
        fleeRule->GetRule()->SetEnabled(true); 
        break;

    case 2: 
        separationRule->GetRule()->SetEnabled(true); 
        break;

    // If 3 points or more, use both rules.. 
    default: 
        separationRule->GetRule()->SetEnabled(true);
        fleeRule->GetRule()->SetEnabled(true);
        break;
    }
}


void InputController::UpdateRules(Core::ProcessEventArg arg) {
    vector<IRuleHandler*>::iterator itr;
    for(itr=ruleHandlers.begin(); itr!=ruleHandlers.end(); itr++){
        // Handle time propagation.
        (*itr)->Handle(arg);
        // Handle input.
        (*itr)->HandleInput(laserPoints);
    }
}

void InputController::Handle(Core::DeinitializeEventArg arg) {
}


void InputController::HandleMouseInput(){
    Devices::MouseState arg = mouse->GetState();
    mousePoints.push_back(Vector<2,int>(arg.x, arg.y));
}


void InputController::HandleLaserInput() {
    // Get laser readings.
    vector< Vector<2,float> > trackingPoints = laser->GetState();

    if( trackingPoints.size() == 2 ){
        cacheP0 += trackingPoints[0];
        cacheP0 /= 2.0;
        cacheP1 += trackingPoints[1];
        cacheP1 /= 2.0;
        if( countTendency < 15 ) countTendency++;
    }else{
        countTendency--;
    }

    if( trackingPoints.size() == 1 && countTendency > 0 ){
        float dist0 = (cacheP0 - trackingPoints[0]).GetLength();
        float dist1 = (cacheP1 - trackingPoints[0]).GetLength();

        if( dist0 > dist1 ){
            trackingPoints.push_back(cacheP0);
        }else{
            trackingPoints.push_back(cacheP1);
        }
    }

    if( countTendency <= 0 ){
        cacheP0 *= 0;
        cacheP1 *= 0;
        countTendency = 0;
    }
    //logger.info << "numTracking: "<<trackingPoints.size() << ", tendency: " << countTendency << logger.end;

    for( unsigned int i=0 ; i<trackingPoints.size(); i++ ){
        laserPoints.push_back(LaserPointToScreenCoordinates(trackingPoints[i]));
    }
}

Vector<3,float> InputController::ScreenToSceneCoordinates(int x, int y) {
    float xRange = 370.0f;
    float yRange = 180.0f;
    //float yTop = 170.0;

    Vector<3,float> pos;
    pos[0] = (-xRange/2.0f) + ((x / (float)SCREEN_WIDTH) * xRange);
    pos[1] = ((y / (float)SCREEN_HEIGHT) * yRange);

    //    logger.info << "Y: " << y << ", SceneY: " << pos[1] << logger.end;
    return pos;
}

Vector<2,int> InputController::LaserPointToScreenCoordinates(Vector<2,float> p) {
    Vector<2,int> point;
    point[0] = ((p[0]+1)/2.0) * SCREEN_WIDTH;
    //logger.info << "point[1]: " << p[1] << logger.end;
    point[1] = SCREEN_HEIGHT - (p[1] * SCREEN_HEIGHT);
    return point;
}


} // NS dva


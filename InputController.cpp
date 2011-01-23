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
#include <Animations/SeperationRule.h>
#include <Logging/Logger.h>
#include "CylinderNode.h"
#include "DVASetup.h"
#include <vector>

Vector<3,float> cylinderP0(0,54,0);
Vector<3,float> cylinderP1(0,190,-8000);
float CYLINDER_RADIUS = 25.0f;
float SCARE_FACTOR = 10.0f;

using namespace OpenEngine::Animations;
using namespace std;

namespace dva {

void InputController::Init() {
    sensor = NULL;
    keyboard = NULL;
    mouse = NULL;
    ctrlMode = NONE;
    flock = NULL;
    flockFollowTrans = NULL;
    cm = NULL;
    mouseCtrlRule = NULL;
    separationRule = NULL;
    sceneNode = new SceneNode();
    mouseCtrlTrans = NULL;
    debugMesh = NULL;
}

InputController::InputController() {
    Init();
}

InputController::InputController(LaserSensor* sensor) {
    Init();
    this->sensor = sensor;
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
    this->sensor = sensor;
}

void InputController::SetInputDevice(IKeyboard* keyboard) {
    this->keyboard = keyboard;
}

void InputController::SetInputDevice(IMouse* mouse) {
    this->mouse = mouse;
}

// Initialize devices
void InputController::Handle(Core::InitializeEventArg arg) {
    // Set default controller mode.
    if( ctrlMode == NONE ) ctrlMode = MOUSE_CYLINDER_FLEE;

    if( mouse ) {
        /*
         * In this mode the flock is following a point moved in circles
         * and the mouse is controlling a cylinder flee rule.
         */
        if( ctrlMode == MOUSE_CYLINDER_FLEE ) {
            // Initialize mouse controlled cylinder flee.
            mouseCtrlTrans = new TransformationNode();
            mouseCtrlRule = new FleeCylinderRule(mouseCtrlTrans, 
                                                 cylinderP0, cylinderP1, 
                                                 CYLINDER_RADIUS, SCARE_FACTOR);
            flock->AddRule(mouseCtrlRule);
            
            flockFollowTrans = new TransformationNode();
            flockFollowTrans->SetPosition(Vector<3,float>(0,100,-300));
            cm = new CircleMover(flockFollowTrans,Vector<2,float>(100,10),0.7);
            flock->AddRule(new FollowRule(flockFollowTrans));
            sceneNode->AddNode(mouseCtrlTrans);
        }

        /*
         * In this mode the flock is following a point controlled 
         * by the mouse.
         */
        else if( ctrlMode == MOUSE_FLOCK_FOLLOW ) {
            mouseCtrlTrans = new TransformationNode();
            mouseCtrlRule = new FollowRule(mouseCtrlTrans);
            mouseCtrlTrans->SetPosition(Vector<3,float>(0,100,-300));
            flock->AddRule(mouseCtrlRule);
            sceneNode->AddNode(mouseCtrlTrans);
        }

        
        // Visualise mouse controlled transformation node.
        if( debugMesh && mouseCtrlTrans )
            mouseCtrlTrans->AddNode(debugMesh->Clone());
    }


    if( sensor ) {
        /*
         * In this mode a single tracking point controls the flock follow
         * rule, while two tracing points controls the flock separation parameter.
         */
        if( ctrlMode == LASER_FOLLOW_FLOCK_AND_RESIZE ) {
            // Add flock follow rule since this should always be there.
            flockFollowTrans = new TransformationNode();
            flockFollowTrans->SetPosition(Vector<3,float>(0,100,-300));
            flock->AddRule(new FollowRule(flockFollowTrans));
            sceneNode->AddNode(flockFollowTrans);

            // Find separation rule.
            separationRule = (SeperationRule*)flock->GetRuleNamed("Separation");
        }


        // Visualise laser controlled transformation node.
        if( debugMesh && flockFollowTrans )
           flockFollowTrans->AddNode(debugMesh->Clone());
 
        
    }

}

void InputController::Handle(Core::ProcessEventArg arg) {

    switch( ctrlMode ) {
    case MOUSE_CYLINDER_FLEE:
        HandleMouseInput();
        break;
    case MOUSE_FLOCK_FOLLOW:
        HandleMouseInput();
        break;
    case LASER_FOLLOW_FLOCK_AND_RESIZE:
        HandleLaserSensorInput();
        break;


    default: break;
    }

    // If circle mover is defined, process it.
    if( cm ) cm->Handle(arg);
}

void InputController::Handle(Core::DeinitializeEventArg arg) {
}


void InputController::HandleMouseInput(){
    if( mouseCtrlTrans ){
        Devices::MouseState arg = mouse->GetState();
        Vector<3,float> pos = ScreenToSpaceCoordinate(arg.x, arg.y);
        pos[2] = mouseCtrlTrans->GetPosition()[2];
        mouseCtrlTrans->SetPosition(pos);
        //logger.info << "Mouse x,y: " << arg.x << ", " << arg.y << logger.end;
    }
}


void InputController::HandleLaserSensorInput() {

    // Get laser readings.
    vector< Vector<2,float> > points = sensor->GetState();
    
    // Determine tracking mode
    if( points.size() == 1 ){
        // Single tracking mode.
        Vector<2,float> point = points[0];

        int x = ((point[0]+1)/2.0) * SCREEN_WIDTH;
        int y = SCREEN_HEIGHT - ((point[1]+1)/2.0) * SCREEN_HEIGHT;

        Vector<3,float> pos = ScreenToSpaceCoordinate(x, y);
        pos[2] = flockFollowTrans->GetPosition()[2];

        //logger.info << "Laser x,y: " << x << ", " << y << logger.end;
        flockFollowTrans->SetPosition(pos);
        

    } else if( points.size() == 2 ){
        // Multi tracking mode.
        if( separationRule ){
            // Find center of the two points
            Vector<2,float> point0 = points[0];
            int x0 = ((point0[0]+1)/2.0) * SCREEN_WIDTH;
            int y0 = SCREEN_HEIGHT - ((point0[1]+1)/2.0) * SCREEN_HEIGHT;

            Vector<2,float> point1 = points[1];
            int x1 = ((point1[0]+1)/2.0) * SCREEN_WIDTH;
            int y1 = SCREEN_HEIGHT - ((point1[1]+1)/2.0) * SCREEN_HEIGHT;

            Vector<2,float> center((x0+x1)/2.0, (y0+y1)/2.0);

            Vector<3,float> pos = ScreenToSpaceCoordinate(center[0], center[1]);
            pos[2] = flockFollowTrans->GetPosition()[2];

            //logger.info << "Laser x,y: " << x << ", " << y << logger.end;
            flockFollowTrans->SetPosition(pos);

            // Distance between points defines separation distance on the flock.
            float distBetweenPoints = (point0-point1).GetLength();
            //logger.info << "Separation Dist: " << distBetweenPoints << logger.end;

            distBetweenPoints *= 20.0f;
            separationRule->SetDistance(distBetweenPoints);
            //            logger.info << "Separation Dist: " << distBetweenPoints << logger.end;



        }

    }

    /*
    // Adjust number of rules according to number of avatars.
    if( laserAvatars.size() == points.size() ){
        for( unsigned int i=0; i<points.size(); i++){
            TransformationNode* trans = laserAvatars[i]->GetTransformationToFleeFrom();
            Vector<2,float> point = points[i];
            Vector<3,float> pos;

            int x = ((point[0]+1)/2.0) * SCREEN_WIDTH;
            int y = ((point[1]+1)/2.0) * SCREEN_HEIGHT;

            float xRange = 370.0f;
            float yRange = 160.0f;
            float yTop = 170.0;
            
            pos[0] = (-xRange/2.0f) + ((x / (float)SCREEN_WIDTH) * xRange);
            pos[1] = yTop - ((y / (float)SCREEN_HEIGHT) * yRange);
            pos[2] = -300;
            trans->SetPosition(pos);
            //            logger.info << "POS: " << pos << logger.end;

        }
        
    } else if( laserAvatars.size() > points.size() ) {
        // Remove flee rule from flock.
//         SearchTool search;
//         stringstream ss;
//         ss << laserAvatars.size();
//         PropertyNode* p = search.DescendantPropertyNodeWith("id", ss.str(), scene );
      
//         if( p ){
//             p->GetParent()->RemoveNode(p);
//             //            logger.info << "FOUND" << logger.end;
//         }

        flock->RemoveRule(laserAvatars.back());
        laserAvatars.pop_back();
        

        //        logger.info << "Removed Avatar" << logger.end;
    } else if( laserAvatars.size() < points.size() ) {
        // Add flee rule to flock.
        TransformationNode* trans = new TransformationNode();
        FleeRule* newAvatar = new FleeSphereRule(trans, 75.0, 20.0);
        flock->AddRule(newAvatar);
        laserAvatars.push_back(newAvatar);

//         if( scene ){
//             trans->AddNode(object->Clone());
//             PropertyNode* p = new PropertyNode();
//             p->SetProperty("id", (int)laserAvatars.size());
//             p->AddNode(trans);
//             scene->AddNode(p);
//         }

        //        logger.info << "Added Avatar" << logger.end;
    }    


    //    logger.info << "NumAvatars: " << laserAvatars.size() << logger.end;

//     for( unsigned int i=0; i<points.size(); i++){
//         logger.info << "LaserPoint: " << points[i] << logger.end;
    
//         //    flock->AddRule(new FleeRule(human, 100.0, 1.0));

//     }  
*/    
}

Vector<3,float> InputController::ScreenToSpaceCoordinate(int x, int y) {
    float xRange = 370.0f;
    float yRange = 300.0f;
    float yTop = 170.0;

    Vector<3,float> pos;
    pos[0] = (-xRange/2.0f) + ((x / (float)SCREEN_WIDTH) * xRange);
    pos[1] = yTop - ((y / (float)SCREEN_HEIGHT) * yRange);
    return pos;
}


void InputController::SetFlock(Flock* flock) {
    this->flock = flock;
}

void InputController::SetMode(CtrlMode mode) {
    if( mode == ctrlMode ) {
        logger.info << "[InputController] Mode " << mode << " is already set." << logger.end;
        return;
    }

//     if( mouse && mouseCtrlTrans && flock ){
//         flock->RemoveRule(mouseCtrlRule);
//     }

    ctrlMode = mode;
}

void InputController::SetDebugMesh(TransformationNode* mesh){
    debugMesh = mesh;
}

ISceneNode* InputController::GetSceneNode() {
    return sceneNode;
}

} // NS dva


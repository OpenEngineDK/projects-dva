// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _INPUT_CONTROLLER_H_
#define _INPUT_CONTROLLER_H_

#include <Core/IModule.h>
#include "Devices/LaserSensor.h"
#include <Devices/IKeyboard.h>
#include <Devices/IMouse.h>
#include "CircleMover.h"
#include "DVASetup.h"

namespace OpenEngine {
    namespace Devices {
        class LaserSensor;
        class IKeyboard;
        class IMouse;
    }
    namespace Animations {
        class Flock;
        class IRule;
        class SeparationRule;
    }
    namespace Scene {
        class ISceneNode;
        class TransformationNode;
    }
}


namespace dva {

using OpenEngine::Core::IModule;
using OpenEngine::Devices::LaserSensor;
using OpenEngine::Devices::IKeyboard;
using OpenEngine::Devices::IMouse;
using OpenEngine::Animations::Flock;
using OpenEngine::Animations::IRule;
using OpenEngine::Animations::SeparationRule;
using OpenEngine::Scene::ISceneNode;
using OpenEngine::Scene::TransformationNode;


/**
 * Short description.
 *
 * @class InputController InputController.h ts/dva/InputController.h
 */
class InputController : public IModule {
private:
    LaserSensor* sensor;
    IKeyboard* keyboard;
    IMouse* mouse;

    // Defines current controller mode.
    CtrlMode ctrlMode;

    Flock* flock;
    TransformationNode* flockFollowTrans;
    CircleMover* cm;

    IRule* mouseCtrlRule;
    std::vector<IRule*> laserCtrlRules;
    SeparationRule* separationRule;

    ISceneNode* sceneNode;
    TransformationNode* mouseCtrlTrans;
    TransformationNode* debugMesh;

    void Init();

    void HandleMouseInput();
    void HandleLaserSensorInput();

    Vector<3,float> ScreenToSpaceCoordinate(int x, int y);

public:
    InputController();
    InputController(LaserSensor* sensor);
    InputController(IKeyboard* keyboard);
    InputController(IMouse* mouse);
    ~InputController();

    void SetInputDevice(LaserSensor* sensor);
    void SetInputDevice(IKeyboard* keyboard);
    void SetInputDevice(IMouse* mouse);
    

    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);


    
    void SetFlock(Flock* flock);
    void SetMode(CtrlMode mode);
    void SetDebugMesh(TransformationNode* debugMesh);

    ISceneNode* GetSceneNode();
   
};

} // NS dva


#endif

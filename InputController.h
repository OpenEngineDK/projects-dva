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
#include <Utils/PropertyTree.h>
#include <Utils/PropertyTreeNode.h>
#include <Utils/Timer.h>
#include "Devices/LaserSensor.h"
#include <Devices/IKeyboard.h>
#include <Devices/IMouse.h>
#include "DVASetup.h"
#include <vector>

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
using OpenEngine::Utils::PropertyTreeNode;

class IRuleHandler;

/**
 * Short description.
 *
 * @class InputController InputController.h ts/dva/InputController.h
 */
class InputController : public IModule {
private:
    LaserSensor* laser;
    IKeyboard* keyboard;
    IMouse* mouse;
    unsigned int numMousePoints;
    unsigned int numLaserPoints;
    Flock* flock;

    Utils::Timer timer;
    // List of all mouse tracking points in screen coordinates.
    std::vector< Vector<2,int> > mousePoints;
    // List of all laser tracking points in screen coordinates.
    std::vector< Vector<2,int> > laserPoints;
    // List of all laser controlled flock rules.
    std::vector<IRuleHandler*> ruleHandlers;
    // The flock will always follow a point controlled by this rule. 
    IRuleHandler* followCircle;

    void Init();

    void HandleMouseInput();
    void HandleLaserInput();

    void SetupRules();
    void UpdateRules();

public:
    InputController();
    InputController(LaserSensor* sensor);
    InputController(IKeyboard* keyboard);
    InputController(IMouse* mouse);
    ~InputController();

    void SetInputDevice(LaserSensor* sensor);
    void SetInputDevice(IKeyboard* keyboard);
    void SetInputDevice(IMouse* mouse);

    void SetFlock(Flock* flock);

    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);

    static Vector<3,float> ScreenToSceneCoordinates(int x, int y);
    static Vector<2,int>   LaserPointToScreenCoordinates(Vector<2,float> p);
   
};

} // NS dva


#endif

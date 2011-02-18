// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_SCREENPLAY_CONTROLLER_H_
#define _OE_SCREENPLAY_CONTROLLER_H_

#include <Core/IModule.h>
#include <Core/IListener.h>
#include <Devices/IKeyboard.h>
#include <Utils/PropertyTree.h>
#include <Utils/Timer.h>
#include <map>
#include "Devices/LaserSensor.h"


namespace OpenEngine {
    namespace Animations {
        class Animator;
    }
    namespace Scene {
        class TransformationNode;
    }
    namespace Utils {
        class Timer;
    }
    namespace Sound {
        class ISound;
    }
}

namespace dva {

class RelayBox;
class Projector;

using OpenEngine::Core::IModule;
using OpenEngine::Core::IListener;
using OpenEngine::Utils::Timer;
using OpenEngine::Utils::PropertiesChangedEventArg;
using OpenEngine::Devices::KeyboardEventArg;
using OpenEngine::Devices::LaserInputEventArg;
using OpenEngine::Animations::Animator;
using OpenEngine::Scene::TransformationNode;
using OpenEngine::Sound::ISound;

typedef enum {
    IDLE,
    FLOCK_INTERACTION,
    SHARK_ANIMATION_DID_START,
    SHARK_ANIMATION_IS_PLAYING,
    SHARK_ANIMATION_DID_END,
    WATER_SPLASH,
    WAIT_FOR_SHARK_ANIMATION_TO_END,
    RESET_SCENE
} ScreenplayState;

enum {
    PNEUMATICS = 1,
    WATER_NOZZEL_1 = 2,
    WATER_NOZZEL_2 = 3,
    LIGHT_SOURCE = 4    
};

/**
 * Short description.
 *
 * @class ScreenplayController ScreenplayController.h ts/dva/ScreenplayController.h
 */
class ScreenplayController : public IModule, 
                             public IListener<KeyboardEventArg>,
                             public IListener<LaserInputEventArg>,
                             public IListener<PropertiesChangedEventArg> {
private:
    Utils::PropertyTreeNode* ptNode;
    Animator* sharkAnimator;
    TransformationNode* sharkTrans;
    RelayBox* relayBox;
    Projector* projector;
    std::map<std::string,ISound*> sounds;

    ScreenplayState state; 
    Timer timer;
    float secsBeforeSharkAnimation;
    float secsBeforeSharkAnimationMin;
    float secsBeforeSharkAnimationMax;

    float secsBeforeSharkReset;
    float secsBeforeWaterSplash;
    float secsOfWaterSplash;


    void ReloadProperties();

public:
    ScreenplayController(Utils::PropertyTreeNode* ptNode);
    virtual ~ScreenplayController();

    void Handle(KeyboardEventArg arg);
    void Handle(LaserInputEventArg arg);
    void Handle(PropertiesChangedEventArg arg);
    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);


    void SetSharkAnimator(Animator* anim);
    void SetRelayBox(RelayBox* relay);
    void SetProjector(Projector* projector);
    void SetSounds(std::map<std::string,ISound*> sounds);
};

} // NS dva


#endif // _OE_SCREENPLAY_CONTROLLER_H_

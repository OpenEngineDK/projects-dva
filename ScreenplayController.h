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
#include <Utils/Timer.h>
#include "Devices/LaserSensor.h"


namespace OpenEngine {
    namespace Animations {
        class Animator;
    }
    namespace Utils {
        class Timer;
    }
}

namespace dva {

class RelayBox;

using OpenEngine::Core::IModule;
using OpenEngine::Core::IListener;
using OpenEngine::Utils::Timer;
using OpenEngine::Devices::LaserInputEventArg;
using OpenEngine::Animations::Animator;

/**
 * Short description.
 *
 * @class ScreenplayController ScreenplayController.h ts/dva/ScreenplayController.h
 */
class ScreenplayController : public IModule, public IListener<LaserInputEventArg> {
private:
    Animator* sharkAnimator;
    RelayBox* relayBox;

    Timer timer;
    
    double elapsed;

public:
    ScreenplayController();
    virtual ~ScreenplayController();

    void Handle(LaserInputEventArg arg);
    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);


    void SetSharkAnimator(Animator* anim);
    void SetRelayBox(RelayBox* relay);

};

} // NS dva


#endif // _OE_SCREENPLAY_CONTROLLER_H_

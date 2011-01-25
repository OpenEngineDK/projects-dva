// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include "ScreenplayController.h"
#include <Animations/Animator.h>
#include <Logging/Logger.h>
#include "RelayBox.h"
#include "DVASetup.h"

namespace dva {


ScreenplayController::ScreenplayController() 
    : sharkAnimator(NULL)
    , relayBox(NULL) 
    , elapsed(0) {
}

ScreenplayController::~ScreenplayController() {
}

void ScreenplayController::Handle(LaserInputEventArg arg) {
    logger.info << "[ScreenplayController] Laser Input recv" << logger.end;
}

void ScreenplayController::Handle(Core::InitializeEventArg arg) {
    timer.Start();
}

void ScreenplayController::Handle(Core::ProcessEventArg arg) {
    //if( sharkAnimator->IsPlaying() )
        //logger.info << "SHARK ANIM PLAYING" << logger.end;

    //    elapsed += ((float)arg.approx / 1000.0f);


    static bool state = true;

    if( timer.GetElapsedTime().sec > 5 ){
        relayBox->SetRelayState(1,state);
        state = !state;
        timer.Reset();
    }
    
}

void ScreenplayController::Handle(Core::DeinitializeEventArg arg) {
}


void ScreenplayController::SetSharkAnimator(Animator* anim) {
    this->sharkAnimator = anim;
}

void ScreenplayController::SetRelayBox(RelayBox* relay) {
    this->relayBox = relay;
}


} // NS dva

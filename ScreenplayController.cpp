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
#include <Animations/Animation.h>
#include <Animations/AnimatedTransformation.h>
#include <Scene/TransformationNode.h>
#include <Utils/PropertyTreeNode.h>
#include <Logging/Logger.h>
#include "RelayBox.h"
#include "DVASetup.h"

namespace dva {

using namespace Animations;

ScreenplayController::ScreenplayController(Utils::PropertyTreeNode* ptNode) 
    : ptNode(ptNode)
    , sharkAnimator(NULL)
    , sharkTrans(NULL)
    , relayBox(NULL)
    , state(IDLE)
    , interactionSecs(100) {

    ptNode->PropertiesChangedEvent().Attach(*this);
}

ScreenplayController::~ScreenplayController() {
}

void ScreenplayController::ReloadProperties(){
    interactionSecs = ptNode->GetPath("interactionSecs", 100);
}

void ScreenplayController::Handle(LaserInputEventArg arg) {
    if( state == IDLE ){
        state = FLOCK_INTERACTION;
        timer.Start();
        logger.info << "[ScreenplayController] Wake up.." << logger.end;
    }
}

void ScreenplayController::Handle(PropertiesChangedEventArg arg) {
    // Load properties.
    ReloadProperties();
}


void ScreenplayController::Handle(Core::InitializeEventArg arg) {
    // Load properties.
    ReloadProperties();

    if( !sharkAnimator ){
        logger.error << "[ScreenplayController] Warning: Shark Animator not found!" << logger.end;
        return;
    }
    
    Animation* animation = sharkAnimator->GetAnimation(0);
    if( animation ){
        AnimatedTransformation* animTrans = animation->GetAnimatedTransformation(0);
        if( animTrans ){
            sharkTrans = animTrans->GetAnimatedNode();
            if( !sharkTrans ){
                logger.error << "[ScreenplayController] Could not locate the sharks transformation node." << logger.end;
            }
        }
    }
        
}

void ScreenplayController::Handle(Core::ProcessEventArg arg) {
    
    switch( state ) {
    case IDLE:
        // Shuffle screen saver stuff..
        break;

    case FLOCK_INTERACTION:
        // Check timer
        if( timer.GetElapsedTime().sec > interactionSecs ){
            timer.Stop();
            // Start the shark animation.
            if( sharkAnimator ){
                sharkAnimator->Play();
                state = SHARK_ANIMATION_DID_START;
                logger.info << "Shark animation started.." << logger.end;
            }
        }
        break;

    case SHARK_ANIMATION_DID_START:
        // Start shark sound effect.
        break;

    case SHARK_ANIMATION_DID_END:
        break;

    case CLEAN_UP_SCENE:
        break;

    default:
        break;

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

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
#include <Math/RandomGenerator.h>
#include <Sound/ISound.h>
#include <Devices/Symbols.h>
#include <Utils/PropertyTreeNode.h>
#include <Logging/Logger.h>
#include "RelayBox.h"
#include "Projector.h"
#include "DVASetup.h"

namespace dva {

using namespace Animations;
using namespace Devices;


ScreenplayController::ScreenplayController(Utils::PropertyTreeNode* ptNode) 
    : ptNode(ptNode)
    , sharkAnimator(NULL)
    , sharkTrans(NULL)
    , relayBox(NULL)
    , projector(NULL)
    , state(IDLE)
    , secsBeforeSharkAnimation(30)
    , secsBeforeSharkAnimationMin(10)
    , secsBeforeSharkAnimationMax(90)
    , secsBeforeSharkReset(10)
    , secsBeforeWaterSplash(35.0)
    , secsOfWaterSplash(1.0) {

    ptNode->PropertiesChangedEvent().Attach(*this);
}

ScreenplayController::~ScreenplayController() {
}

void ScreenplayController::ReloadProperties(){
    secsBeforeSharkAnimationMin = ptNode->GetPath("secsBeforeSharkAnimationMin", 10.0f);
    secsBeforeSharkAnimationMax = ptNode->GetPath("secsBeforeSharkAnimationMax", 90.0f);
    secsBeforeSharkReset = ptNode->GetPath("secsBeforeSharkReset", 10.0f);
    secsBeforeWaterSplash = ptNode->GetPath("secsBeforeWaterSplash", 35.0f);
    secsOfWaterSplash = ptNode->GetPath("secsOfWaterSplash", 1.0f);
}

void ScreenplayController::Handle(KeyboardEventArg arg) {

    if( arg.type == EVENT_PRESS ) {
        switch(arg.sym) {
        case KEY_SPACE: 
            {
                //logger.info << "state: " << state << logger.end;

                if( state == IDLE ){
                    state = FLOCK_INTERACTION;
                    logger.info << "[ScreenplayController] Woke up.." << logger.end;
                }
            }
            break;

        case KEY_u:
            {
                logger.info << "[ScreenplayController] Water-splash test ON." << logger.end;
                if( relayBox ){
                    relayBox->SetRelayState(PNEUMATICS, true);
                    relayBox->SetRelayState(WATER_NOZZEL_1, true);
                    relayBox->SetRelayState(WATER_NOZZEL_2, true);
                }
            }
            break;

        case KEY_i:
            {
                logger.info << "[ScreenplayController] Water-splash test OFF." << logger.end;
                if( relayBox ){
                    relayBox->SetRelayState(PNEUMATICS, false);
                    relayBox->SetRelayState(WATER_NOZZEL_1, false);
                    relayBox->SetRelayState(WATER_NOZZEL_2, false);
                }
            }
            break;

        default:
            break;
        }
    }
}

void ScreenplayController::Handle(LaserInputEventArg arg) {

    if( state == IDLE ){
        state = FLOCK_INTERACTION;
        logger.info << "[ScreenplayController] Woke up by laser input.." << logger.end;
    }
}

void ScreenplayController::Handle(PropertiesChangedEventArg arg) {
    // Load properties.
    ReloadProperties();
}


void ScreenplayController::Handle(Core::InitializeEventArg arg) {
    // Load properties.
    ReloadProperties();

    if( sharkAnimator ){
        Animation* animation = sharkAnimator->GetAnimation(0);
        if( animation ){
            AnimatedTransformation* animTrans = animation->GetAnimatedTransformation(0);
            if( animTrans ){
                sharkTrans = animTrans->GetAnimatedNode();
                if( !sharkTrans ){
                    logger.warning << "[ScreenplayController] Could not locate the sharks transformation node." << logger.end;
                }
            }
        }
    } else {
        logger.warning << "[ScreenplayController] Warning: Shark Animator not found!" << logger.end;
    }


    // Turn on projector
    if( PROJECTOR_ENABLED && projector ){
        projector->PowerOn();
        logger.info << "[ScreenplayController] Power on signal send to projector." << logger.end;
    }

}

void ScreenplayController::Handle(Core::ProcessEventArg arg) {

    // Start shark sound effect.
    ISound* sound = sounds["sound1"];
    if( sound && !sound->IsPlaying() ) {
        sound->Play();
        sound->SetGain(0.3);
        logger.info << "[ScreenplayController] Restarting sound1." << logger.end;
    }

    
    switch( state ) {
    case IDLE:
        // Shuffle screen saver stuff..
        break;

    case FLOCK_INTERACTION:
        // Start timer
        if( !timer.IsRunning() ){
            // Random the shark appearance time interval.
            Math::RandomGenerator rand;
            rand.SeedWithTime();
            secsBeforeSharkAnimation = rand.UniformFloat(secsBeforeSharkAnimationMin, secsBeforeSharkAnimationMax);
            logger.info << "[ScreenplayController] Seconds before shark appearance: " << secsBeforeSharkAnimation << logger.end;
            timer.Start();
        }
        // Check timer
        if( timer.GetElapsedTime().sec >= secsBeforeSharkAnimation ){
            timer.Stop();
            timer.Reset();
            // Start the shark animation.
            if( sharkAnimator ){
                sharkAnimator->Play();
                state = SHARK_ANIMATION_DID_START;
                logger.info << "[ScreenplayController] Shark animation started." << logger.end;
            }
        }
        break;

    case SHARK_ANIMATION_DID_START:
        {
            // Start shark sound effect.
            ISound* sound = sounds["sound4"];
            if( sound ) {
                sound->Play();
                sound->SetGain(1.0);
                logger.info << "[ScreenplayController] Shark sound playing." << logger.end;
            }
            state = SHARK_ANIMATION_IS_PLAYING;
        }
        break;

    case SHARK_ANIMATION_IS_PLAYING:
        {
            // Start timer
            if( !timer.IsRunning() ){
                timer.Start();
            }

            // Start water splash when shark opens mouth.
            if( timer.GetElapsedTime().sec >= secsBeforeWaterSplash ){
                timer.Stop();
                timer.Reset();
                logger.info << "[ScreenplayController] Time for water splash." << logger.end;
                state = WATER_SPLASH;                
            }
        }
        break;

    case WATER_SPLASH:
        {
            // Start timer
            if( !timer.IsRunning() ){
                timer.Start();
                
                // Turn off the light source.
                logger.info << "[ScreenplayController] Turn off light source." << logger.end;
                if( relayBox ) relayBox->SetRelayState(LIGHT_SOURCE, false);
                // Activate water splash.
                logger.info << "[ScreenplayController] Activate water splash." << logger.end;
                if( relayBox ) {
                    relayBox->SetRelayState(PNEUMATICS, true);
                    relayBox->SetRelayState(WATER_NOZZEL_1, true);
                    relayBox->SetRelayState(WATER_NOZZEL_2, true);
                }
            }
            // Check when to turn off water splash.
            if( timer.GetElapsedTime().sec >= secsOfWaterSplash ){
                timer.Stop();
                timer.Reset();
                // Activate water splash.
                logger.info << "[ScreenplayController] Deactivate water splash." << logger.end;
                if( relayBox ){
                    relayBox->SetRelayState(PNEUMATICS, false);
                    relayBox->SetRelayState(WATER_NOZZEL_1, false);
                    relayBox->SetRelayState(WATER_NOZZEL_2, false);
                }
                state = WAIT_FOR_SHARK_ANIMATION_TO_END;
            }
        }
        break;

    case WAIT_FOR_SHARK_ANIMATION_TO_END:
        {
            // Check if shark animation is done.
            if( !sharkAnimator->IsPlaying() ){
                logger.info << "[ScreenplayController] Shark animation did finish." << logger.end;
                state = SHARK_ANIMATION_DID_END;
            }
        }
        break;

    case SHARK_ANIMATION_DID_END:
        {
            // Start timer
            if( !timer.IsRunning() ){
                timer.Start();
            }
            // Reset shark after some time.
            if( timer.GetElapsedTime().sec >= secsBeforeSharkReset ){
                timer.Stop();
                timer.Reset();
                logger.info << "[ScreenplayController] Shark animation reset." << logger.end;
                state = RESET_SCENE; 
            }
        }
        break;


    case RESET_SCENE:
        {
            // Start timer since reset..
            if( !timer.IsRunning() ){
                timer.Start();
            }

            // Ignore laser input the first secs after reset due to water splash.    
            if( timer.GetElapsedTime().sec > 2 ){
                timer.Stop();
                timer.Reset();
                logger.info << "[ScreenplayController] Scene reset." << logger.end;
         
                // Reset shark animation.
                sharkAnimator->Reset();

                // Turn on light source.
                logger.info << "[ScreenplayController] Turn on light source." << logger.end;
                if( relayBox ) relayBox->SetRelayState(LIGHT_SOURCE, true);

                // Go back to idle state.
                state = IDLE;
            }
        }
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

void ScreenplayController::SetProjector(Projector* projector) {
    this->projector = projector;
}

void ScreenplayController::SetSounds(std::map<std::string,ISound*> sounds) {
    this->sounds = sounds;
}


} // NS dva

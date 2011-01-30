
#include "UserDefaults.h"
#include <Utils/SimpleSetup.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Animations/Animator.h>
#include <Sound/ISound.h>

using namespace OpenEngine::Scene;
using namespace OpenEngine::Sound;
using namespace OpenEngine::Animations;

namespace OpenEngine{
namespace Utils{

class CustomKeyHandler : public IListener<KeyboardEventArg> {
private:
    SimpleSetup& setup;
    RenderStateNode* rsn;
    std::map<std::string,ISound*> sounds;

public:
    CustomKeyHandler(SimpleSetup& setup) : setup(setup), rsn(NULL)  {
    }
    
    void SetRenderStateNode(RenderStateNode* rsn){
        this->rsn = rsn;
    }

    void Handle(KeyboardEventArg arg) {
        if( rsn == NULL ){
            SearchTool st;
            rsn = st.DescendantRenderStateNode(setup.GetScene());
            if (rsn) {
                logger.info << "[CustomKeyhandler] found RenderStateNode in scene." << logger.end;
            } else {
                logger.info << "[CustomKeyhandler] could not find RenderStateNode in scene, custom keys are disabled." << logger.end;
                return;
            }
        }

        if( arg.type == EVENT_PRESS ) {
            RenderStateNode::RenderStateOption opt = rsn->GetEnabled();
            switch(arg.sym) {
            case KEY_1: rsn->ToggleOption(RenderStateNode::LIGHTING); break;
            case KEY_2: rsn->ToggleOption(RenderStateNode::WIREFRAME); break;
            case KEY_3: rsn->ToggleOption(RenderStateNode::TEXTURE); break;
            case KEY_4: rsn->ToggleOption(RenderStateNode::SHADER); break;
            case KEY_5: rsn->ToggleOption(RenderStateNode::COLOR_MATERIAL); break;
            case KEY_6: rsn->ToggleOption(RenderStateNode::SOFT_NORMAL); break;
            case KEY_7: rsn->ToggleOption(RenderStateNode::HARD_NORMAL); break;
            case KEY_8: rsn->ToggleOption(RenderStateNode::BACKFACE); break;

            case KEY_e : setup.ShowFPS(); break;

            case KEY_SPACE: {
                Animator* anim = (Animator*)UserDefaults::GetInstance()->map["SharkAnimator"];
                if( anim ){
                    if( anim->IsPlaying() )
                        anim->Pause();
                    else
                        anim->Play();
                }
            }
                break;


            case KEY_f: 
                {
                    ISound* sound = sounds["sound1"];
                    if (sound != NULL)
                        sound->Play(); 
                    break;
                }
            case KEY_c: 
                {
                    ISound* sound = sounds["sound1"];
                    if (sound != NULL)
                        sound->Stop(); 
                    break;
                }
            case KEY_g: 
                {
                    ISound* sound = sounds["sound2"];
                    if (sound != NULL)
                        sound->Play(); 
                    break;
                }
            case KEY_v: 
                {
                    ISound* sound = sounds["sound2"];
                    if (sound != NULL)
                        sound->Stop(); 
                    break;
                }
            case KEY_h: 
                {
                    ISound* sound = sounds["sound3"];
                    if (sound != NULL)
                        sound->Play(); 
                    break;
                }
            case KEY_b: 
                {
                    ISound* sound = sounds["sound3"];
                    if (sound != NULL)
                        sound->Stop(); 
                    break;
                }
            case KEY_j: 
                {
                    ISound* sound = sounds["sound4"];
                    if (sound != NULL)
                        sound->Play(); 
                    break;
                }
            case KEY_n: 
                {
                    ISound* sound = sounds["sound4"];
                    if (sound != NULL)
                        sound->Stop(); 
                    break;
                }
            case KEY_k: 
                {
                    ISound* sound = sounds["sound5"];
                    if (sound != NULL)
                        sound->Play(); 
                    break;
                }
            case KEY_m: 
                {
                    ISound* sound = sounds["sound5"];
                    if (sound != NULL)
                        sound->Stop(); 
                    break;
                }


            default:break;
            
            } 
            // Print status if any change.
            if( rsn->GetEnabled() != opt ){
                logger.info << rsn->ToString() << logger.end;
            }
        }
    }

    void AddSound(ISound* sound, std::string str) {
        sounds[str] = sound;
    }
};

} // NS Utils
} // NS OpenEngine

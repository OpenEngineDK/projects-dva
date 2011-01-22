
#include "UserDefaults.h"
#include <Utils/SimpleSetup.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Animations/Animator.h>

using namespace OpenEngine::Scene;
using namespace OpenEngine::Animations;

namespace OpenEngine{
namespace Utils{

class CustomKeyHandler : public IListener<KeyboardEventArg> {
private:
    SimpleSetup& setup;
    RenderStateNode* rsn;

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


            case KEY_f : setup.ShowFPS(); break;

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



            default:break;
            
            } 
            // Print status if any change.
            if( rsn->GetEnabled() != opt ){
                logger.info << rsn->ToString() << logger.end;
            }
        }
    }
};

} // NS Utils
} // NS OpenEngine

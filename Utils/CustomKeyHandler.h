
#include "UserDefaults.h"
#include <Utils/SimpleSetup.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Animations/Animator.h>

using namespace OpenEngine::Scene;
using namespace OpenEngine::Animations;

namespace OpenEngine{
namespace Utils{

class SceneNodeFinder : public ISceneNodeVisitor {
private:
    string nodeName;
    ISceneNode* foundNode;

    void DefaultVisitNode(ISceneNode* node){
        if( node->GetNodeName() == nodeName ){
            foundNode = node;
        }else if( node->subNodes.size() > 0 ) {
            node->VisitSubNodes(*this);
        }
    }

public:
    ISceneNode* Find(string nodeWithName, ISceneNode* searchRoot){
        nodeName = nodeWithName;
        foundNode = NULL;

        nodeName = nodeWithName;
        //
        DefaultVisitNode(searchRoot);
        //
        return foundNode;
    }
};


class CustomKeyHandler : public IListener<KeyboardEventArg> {
private:
    SimpleSetup& setup;
    RenderStateNode* rsn;
    DotVisitor dv;
    ofstream* os;

    bool FindRenderStateNode(){
        ISceneNode* scene = setup.GetScene();
        if( scene != NULL && rsn == NULL ){
            for(unsigned int i=0; i < scene->GetNumberOfNodes(); i++){
                if( scene->GetNode(i)->GetNodeName() == "RenderStateNode" ){
                    rsn = (RenderStateNode*)scene->GetNode(i);
                    return true;
                }
            }
        }
        return false;
    }

public:
    CustomKeyHandler(SimpleSetup& setup) : setup(setup), rsn(NULL)  {
        os = new ofstream("graph.dot", ofstream::out);
    }
    
    void SetRenderStateNode(RenderStateNode* rsn){
        this->rsn = rsn;
    }

    void Handle(KeyboardEventArg arg) {
        if( rsn == NULL ){
            //if( FindRenderStateNode() ){
            SceneNodeFinder finder;
            if( (rsn = dynamic_cast<RenderStateNode*>(finder.Find("RenderStateNode", setup.GetScene()))) != NULL ){
                logger.info << "[CustomKeyhandler] found RenderStateNode in scene." << logger.end;
            }else{
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

//             case KEY_p:   
//                 // Write dot graph    
//                 dv.Write(*(setup.GetScene()), os);
//                 break;

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

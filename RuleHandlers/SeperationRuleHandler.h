// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_COHESION_RULE_HANDLER_H_
#define _OE_COHESION_RULE_HANDLER_H_

#include "IRuleHandler.h"
#include <Animations/CohersionRule.h>
#include <Scene/TransformationNode.h>
#include <Logging/Logger.h>
#include <vector>

namespace dva {

using OpenEngine::Animations::CohersionRule;
using OpenEngine::Scene::TransformationNode;


class FleeRuleHandler : public IRuleHandler {
private:
    CohersionRule* rule;

public:
    FleeRuleHandler(Flock* flock) 
        : IRuleHandler(flock)
        , rule(new CohersionRule()) {
        Load();
        logger.info << "[CohesionRuleHandler] created" << logger.end;
    }

    ~CohesionRuleHandler() {
        Unload();
        if( rule ){
            delete rule;
            rule = NULL;
        }
        logger.info << "[CohesionRuleHandler] deleted" << logger.end;
    }

    void Load(){
        if( !loaded ){
            flock->AddRule(rule);
            loaded = true;
            logger.info << "[CohesionRuleHandler] loaded" << logger.end;
        }
    }
    
    void Unload(){
        if( loaded ){
            flock->RemoveRule(rule);
            logger.info << "[CohesionRuleHandler] unloaded" << logger.end;
        }
    }
    
    void HandleInput(std::vector< Vector<2,int> > points) {
        // Need two points to set cohesion magnitude.
        if( points.size() >= 2 ) {

            Vector<2,int> p = points[0];
            Vector<3,float> pos = InputController::ScreenToSceneCoordinates(p[0], p[1]);
            pos[2] = trans->GetPosition()[2];
            trans->SetPosition(pos);
        }
    }
};

} // NS dva

#endif // _OE_COHESION_RULE_HANDLER_H_

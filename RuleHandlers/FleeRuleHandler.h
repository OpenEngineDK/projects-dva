// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_FLEE_RULE_HANDLER_H_
#define _OE_FLEE_RULE_HANDLER_H_

#include "IRuleHandler.h"
#include <Animations/FleeSphereRule.h>
#include <Scene/TransformationNode.h>
#include <Logging/Logger.h>
#include <vector>

namespace dva {

using OpenEngine::Animations::FleeSphereRule;
using OpenEngine::Scene::TransformationNode;


class FleeRuleHandler : public IRuleHandler {
private:
    TransformationNode* trans;

public:
    FleeRuleHandler(FleeSphereRule* r, Flock* flock) 
        : IRuleHandler(r, flock)
        , trans(r->GetTransformationToFleeFrom()) {
        logger.info << "[FleeRuleHandler] created" << logger.end;
    }

    ~FleeRuleHandler() {
        if( trans ){
            delete trans;
            trans = NULL;
        }
        logger.info << "[FleeRuleHandler] deleted" << logger.end;
    }

    void HandleInput(std::vector< Vector<2,int> >& points) {
        if( points.size() > 0 && trans != NULL) {
            Vector<2,int> p = points[0];
            Vector<3,float> pos = InputController::ScreenToSceneCoordinates(p[0], p[1]);
            pos[2] = trans->GetPosition()[2];
            trans->SetPosition(pos);
         } 
    }
};

} // NS dva

#endif // _OE_FLEE_RULE_HANDLER_H_

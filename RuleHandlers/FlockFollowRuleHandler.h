// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_FLOCK_FOLLOW_RULE_HANDLER_H_
#define _OE_FLOCK_FOLLOW_RULE_HANDLER_H_

#include "IRuleHandler.h"
#include <Animations/FollowRule.h>
#include <Logging/Logger.h>
#include <vector>

namespace dva {

using OpenEngine::Animations::FollowRule;

class FlockFollowRuleHandler : public IRuleHandler {
private:
    TransformationNode* trans;
    float pos;
    Vector<3,float> offset;
    Vector<2,float> circle;
    float speed;
    

public:
    FlockFollowRuleHandler(Flock* flock, 
                           TransformationNode* t,
                           Vector<2,float> c=(Vector<2,float>(10,100)),
                           float s=2)
        : IRuleHandler(new FollowRule(t), flock)
        , trans(t)
        , pos(0)
        , offset(t->GetPosition())
        , circle(c)
        , speed(s) {
        logger.info << "[FlockFollowRuleHandler] created" << logger.end;
    }

    ~FlockFollowRuleHandler() {
        if( trans ) {
            delete trans;
            trans = NULL;
        }
        logger.info << "[FlockFollowRuleHandler] deleted" << logger.end;
    }

    void HandleInput(std::vector< Vector<2,int> >& screenPoints) {}


    void Handle(Core::ProcessEventArg arg) {
        float delta = arg.approx/1000000.0;
        pos += delta;
        trans->SetPosition(Vector<3,float>(offset[0]+circle[0]*sin(pos*speed),
                                           offset[1],
                                           offset[2]+circle[1]*cos(pos*speed)));
    }
 
};

} // NS dva

#endif // _OE_FLOCK_FOLLOW_RULE_HANDLER_H_

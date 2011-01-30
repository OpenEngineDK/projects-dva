// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_SEPARATION_RULE_HANDLER_H_
#define _OE_SEPARATION_RULE_HANDLER_H_

#include "IRuleHandler.h"
#include <Animations/SeparationRule.h>
#include <Logging/Logger.h>
#include <vector>

namespace dva {

using OpenEngine::Animations::SeparationRule;

const float SEPARATION_FACTOR = 0.1f;

/*
 * NOTE: this rule handler removes the two points which are furthest from
 *       each other from the list of tracking points given.
 *
 */
class SeparationRuleHandler : public IRuleHandler {
public:
    SeparationRuleHandler(Flock* flock) 
        : IRuleHandler(new SeparationRule(), flock) {
        logger.info << "[SeparationRuleHandler] created" << logger.end;
    }

    ~SeparationRuleHandler() {}
    
    void HandleInput(std::vector< Vector<2,int> >& points) {
        // Need two points to set cohesion magnitude.
        if( points.size() >= 2 ) {

            // Find max distance between all points.
            float maxDist = 0.0;
            unsigned int pid0 = 0;
            unsigned int pid1 = 0;
            for( unsigned int i=0; i<points.size(); i++){
                for( unsigned int j=0; j<points.size(); j++){
                    if( i == j ) continue;
                    Vector<2,int> pos0 = points[i];
                    Vector<2,int> pos1 = points[j];
                    
                    float dist = (pos0 - pos1).GetLength();
                    if( dist > maxDist ) {
                        maxDist = dist;
                        pid0 = i;
                        pid1 = j;
                    }
                }
            }
            if( maxDist > 0.0 ){
                // Max dist found -> pid 0 and 1 must be valid.
                std::vector< Vector<2,int> > exclude;
                for( unsigned int i=0; i<points.size(); i++){
                    if( i != pid0 && i != pid1 ){
                        exclude.push_back(points[i]);
                    }
                }
                points = exclude;
            }
     
            maxDist *= SEPARATION_FACTOR;
            ((SeparationRule*)rule)->SetDistance(maxDist);
        }

    }
};

} // NS dva

#endif // _OE_SEPARATION_RULE_HANDLER_H_

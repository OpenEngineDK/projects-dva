// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _CIRCLE_MOVER_H_
#define _CIRCLE_MOVER_H_

#include <Core/IListener.h>
#include <Scene/TransformationNode.h>

namespace dva {

using OpenEngine::Core::IListener;
using OpenEngine::Scene::TransformationNode;

class CircleMover : public IListener<Core::ProcessEventArg> {
    TransformationNode* node;
    float pos;
    Vector<3,float> offset;
    Vector<2,float> circle;
    float speed;
public:
    CircleMover(TransformationNode *n, 
                Vector<2,float> cir=(Vector<2,float>(10,100)),
                float s=2)
        : node(n),pos(0),offset(n->GetPosition()), circle(cir),speed(s) {}
    void Handle(Core::ProcessEventArg arg) {
        float delta = arg.approx/1000000.0;
        pos += delta;
        node->SetPosition(Vector<3,float>(offset[0]+circle[0]*sin(pos*speed),
                                          offset[1],
                                          offset[2]+circle[1]*cos(pos*speed)));
    }
};


} // NS dva

#endif

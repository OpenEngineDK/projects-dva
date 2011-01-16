// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include "Predator.h"
#include <Logging/Logger.h>
#include <Math/Vector.h>
#include <Scene/TransformationNode.h>
#include <Devices/IMouse.h>
#include "setup.h"
#include <algorithm>
#include "Utils/UserDefaults.h"

using namespace std;
using namespace OpenEngine::Math;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Devices;

namespace dva {

Predator::Predator(TransformationNode* avatar) : avatar(avatar){
    avatar->SetPosition(Vector<3,float>(-300,14.0,0));
    copy = new TransformationNode();
}

Predator::~Predator() {
}

TransformationNode* Predator::GetTransformationNode() {
    return copy;
}

void Predator::Handle(InitializeEventArg arg) {
    IMouse* mouse = (IMouse*)UserDefaults::GetInstance()->map["Mouse"];
    if( mouse ) mouse->HideCursor();
}

void Predator::Handle(ProcessEventArg arg) {
    //    logger.info << "Predator pos: " << avatar->GetPosition() << logger.end;
    copy->SetPosition(Vector<3,float>(avatar->GetPosition()[1] * -1,
                                      avatar->GetPosition()[2], 
                                      avatar->GetPosition()[0] * -1));

    //    logger.info << "copy pos    : " << copy->GetPosition() << logger.end;
}

void Predator::Handle(DeinitializeEventArg arg) {
}

void Predator::Handle(MouseMovedEventArg arg) {
    Vector<3,float> pos = avatar->GetPosition();

    float xRange = 370.0f;
    float yRange = 160.0f;
    float yTop = 170.0;

    pos[2] = (xRange/2.0f) - ((arg.x / (float)SCREEN_WIDTH) * xRange);
    pos[1] = yTop - ((arg.y / (float)SCREEN_HEIGHT) * yRange);

    avatar->SetPosition(pos);
}


} // NS dva

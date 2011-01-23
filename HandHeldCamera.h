// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _HAND_HELD_CAMERA_H_
#define _HAND_HELD_CAMERA_H_

namespace dva {

#include <Core/IModule.h>
#include <Scene/TransformationNode.h>
#include <Math/Quaternion.h>
#include <Utils/Timer.h>

// class MorphableQuaternion 
// : public Quaternion<float>, IMorpher<Quaternion<float> > {

// public:
//     MorphableQuaternion() {
//     }
//     ~MorphableQuaternion(){}
// };

/**
 * Short description.
 *
 * @class HandHeldCamera HandHeldCamera.h ts/dva/HandHeldCamera.h
 */
class HandHeldCamera : public IModule {
private:
    Utils::Timer timer;                // Sequence timer.
    Camera* camera;
    Vector<3,float> orgCamPos;
    float progx;
    float progy;

public:
    HandHeldCamera(Camera* c) : camera(c), progx(0.0), progy(0.0) {
        orgCamPos = camera->GetPosition();
    }
    ~HandHeldCamera() {}

    // IModule handlers
    void Handle(Core::InitializeEventArg arg) {
        timer.Start();
    }

    void Handle(Core::ProcessEventArg arg) {
        unsigned int elapsed = timer.GetElapsedTimeAndReset().usec;
        float timeInMicros = 10000000.0f;

        progx += fmod(((float)elapsed / timeInMicros), 1.0);
        progy += fmod((((float)elapsed*2) / timeInMicros), 1.0);

        //logger.info << "elapsed: " << elapsed << ", " << progx << ", " << progy << logger.end;
        
        float radX = 2 * PI * progx;
        float radY = 2 * PI * progy;

        float radius = 2.0f;
        Vector<3,float> pos = orgCamPos;
        pos[0] += sin(radX) * radius * 4;
        pos[1] += sin(radY) * radius;

        //logger.info << "prog: " << prog << ", sin: " << sin(rad) << logger.end;
        //        float y = camera->GetPosition()[1];
        // float z = camera->GetPosition()[2];

        camera->SetPosition(pos);
    }

    void Handle(Core::DeinitializeEventArg arg) {}
        
};

} // NS dva

#endif

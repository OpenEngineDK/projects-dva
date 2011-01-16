// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _PREDATOR_H_
#define _PREDATOR_H_

#include <Core/IModule.h>
#include <Devices/IMouse.h>
#include <Scene/ISceneNode.h>

namespace OpenEngine {
    namespace Scene {
        class TransformationNode;    
        class ISceneNode;
    }
}

namespace dva {
    
    using namespace OpenEngine::Core;
    using namespace OpenEngine::Devices;
    using namespace OpenEngine::Scene;

/**
 * Short description.
 *
 * @class Predator Predator.h ts/dva/Predator.h
 */
class Predator : public IModule, 
                 public IListener<MouseMovedEventArg> {
private:
    TransformationNode* avatar;
    TransformationNode* copy;
    
public:
    Predator(TransformationNode* avatar);
    ~Predator();

    //    ISceneNode* GetSceneNode();
    TransformationNode* GetTransformationNode();

    void Handle(OpenEngine::Core::InitializeEventArg arg);
    void Handle(OpenEngine::Core::ProcessEventArg arg);
    void Handle(OpenEngine::Core::DeinitializeEventArg arg);
    void Handle(MouseMovedEventArg arg);

};

} // NS dva


#endif

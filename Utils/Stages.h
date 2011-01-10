// Cross fade between scene stages
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _DVA_STAGES_H_
#define _DVA_STAGES_H_

#include <Core/IModule.h>

#include <Display/IFrame.h>
#include <Display/ICanvas.h>
#include <Display/OpenGL/FadeCanvas.h>
#include <Renderers/TextureLoader.h>

namespace OpenEngine {
namespace Utils {

using Core::IModule;
using Display::IFrame;
using Display::ICanvas;
using Display::OpenGL::FadeCanvas;
using Renderers::TextureLoader;

class Stages: public IModule {
private:
    IFrame& frame;
    TextureLoader& tl;
    ICanvas *loadStage, *sceneStage;
    FadeCanvas* fader;
    float prevTime, progress, loadTime, sceneTime;
    
public:
    Stages(IFrame& frame, TextureLoader& tl, ICanvas* sceneStage);
    virtual ~Stages();
    
    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);
};

}
}

#endif //_DVA_STAGES_H_

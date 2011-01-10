// Cross fade between scene stages
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------


#include "Stages.h"

#include <Math/Vector.h>
#include <Display/OpenGL/TextureCopy.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>

namespace OpenEngine {
namespace Utils {

using namespace Core;
using namespace Resources;
using namespace Display::OpenGL;

Stages::Stages(IFrame& frame, TextureLoader& tl, ICanvas* sceneStage)
    : frame(frame)
    , tl(tl)
    , loadStage(NULL)
    , sceneStage(sceneStage)
    , fader(new FadeCanvas(new TextureCopy()))
    , prevTime(0.0)
    , progress(0.0)
    , loadTime(0.0)
    , sceneTime(2.0)
{
    BlendCanvas* bc = new BlendCanvas(new TextureCopy());
    ITexture2DPtr img = ResourceManager<ITextureResource>::Create("projects/dva/data/small.jpg");
    tl.Load(img);
    bc->AddTexture(img, 100, 100, Vector<4,float>(1.0, 1.0, 1.0, 1.0));
    bc->SetBackground(Vector<4,float>(1.0,1.0,1.0,1.0));
    loadStage = bc;


    fader->InitCanvas(loadStage);
    fader->InitCanvas(sceneStage);
    fader->InitCanvas(bc);
    
    frame.SetCanvas(fader);
}

Stages::~Stages() {
    delete loadStage;
    delete fader;
}

void Stages::Handle(Core::InitializeEventArg arg) {

}
    
void Stages::Handle(Core::DeinitializeEventArg arg) {
}

void Stages::Handle(Core::ProcessEventArg arg) {
    progress += arg.approx * 1e-06;
    if (prevTime <= loadTime && loadTime <= progress) {
        logger.info << "fadeIn" << logger.end;
        fader->FadeIn(loadStage, 1.0);
    }

    if (prevTime <= sceneTime && sceneTime <= progress) {
        logger.info << "fadeTo" << logger.end;
        fader->FadeTo(sceneStage, 1.0);
    }

    prevTime = progress;
}
    
}
}

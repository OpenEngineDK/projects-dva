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

namespace OpenEngine {
namespace Utils {

using namespace Display::OpenGL;
using namespace Core;

Stages::Stages(ICanvasBackend* backend)
    : fade(false)
     , bc(new BlendCanvas(backend))
     , source(NULL)
     , target(NULL)
{
}

Stages::~Stages() {
    delete bc;
}

void Stages::Handle(Display::InitializeEventArg arg) {
    list<ICanvas*>::iterator i = inits.begin();
    for (; i != inits.end(); ++i) {
        ((IListener<Display::InitializeEventArg>*)*i)->Handle(arg);
    }
    bc->Handle(arg);
}
    
void Stages::Handle(Display::DeinitializeEventArg arg) {
    bc->Handle(arg);
    list<ICanvas*>::iterator i = inits.begin();
    for (; i != inits.end(); ++i) {
        ((IListener<Display::DeinitializeEventArg>*)*i)->Handle(arg);
    }
}

void Stages::Handle(Display::ProcessEventArg arg) {
    ((IListener<Display::ProcessEventArg>*)target)->Handle(arg);
    if (!fade) return;
    progress += arg.approx * 1e-6;
    float scale = fmin(progress / duration, 1.0);
    bc->Clear();
    if (source) bc->AddTexture(source->GetTexture(), 0, 0, Vector<4,float>(1.0));

    bc->AddTexture(target->GetTexture(), 0, 0, Vector<4,float>(1.0, 1.0, 1.0, scale));
    bc->Handle(arg);
    if (progress > duration) {
        fade = false;
        source = NULL;
    }
}
    
void Stages::Handle(Display::ResizeEventArg arg) {
    bc->Handle(arg);
}

unsigned int Stages::GetWidth() const {
    return bc->GetWidth();
}

unsigned int Stages::GetHeight() const {
    return bc->GetHeight();
}
    
void Stages::SetWidth(const unsigned int width) {
    bc->SetWidth(width);
}

void Stages::SetHeight(const unsigned int height) {
    bc->SetHeight(height);
}
    
ITexture2DPtr Stages::GetTexture() {
    if (fade) return bc->GetTexture();
    else target->GetTexture();
}


void Stages::FadeIn(ICanvas* canvas, float duration) {
    progress = 0.0;
    this->duration = duration;
    fade = true;
    target = canvas;
    bc->AddTexture(target->GetTexture(), 0, 0, Vector<4,float>(1.0,1.0,1.0,1.0));
    bc->SetBackground(Vector<4,float>(0.0,0.0,0.0,1.0));  
}

void Stages::FadeTo(ICanvas* canvas, float duration) {
    progress = 0.0;
    this->duration = duration;
    fade = true;
    source = target;
    target = canvas;
    bc->Clear();
    bc->AddTexture(source->GetTexture(), 0, 0, Vector<4,float>(1.0,1.0,1.0,1.0));
}

void Stages::InitCanvas(ICanvas* canvas) {
    inits.push_back(canvas);
}

}
}

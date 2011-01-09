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

// #include <Core/IModule.h>

#include <Display/IFrame.h>
#include <Display/ICanvas.h>
#include <Resources/ITexture2D.h>
#include <Display/ICanvasBackend.h>
#include <Display/OpenGL/BlendCanvas.h>
#include <list>

namespace OpenEngine {
namespace Utils {

// using Core::IModule;
using Display::IFrame;
using Display::ICanvas;
using Display::ICanvasBackend;
using Display::OpenGL::BlendCanvas;
using Resources::ITexture2DPtr;

using std::list;

class Stages: public ICanvas {
private:
    // IFrame& frame;
    bool fade;
    float progress, duration;    
    BlendCanvas* bc;
    ICanvas *source, *target;
    list<ICanvas*> inits;
public:
    Stages(ICanvasBackend* backend);
    virtual ~Stages();
    
    void Handle(Display::InitializeEventArg arg);
    void Handle(Display::DeinitializeEventArg arg);
    void Handle(Display::ProcessEventArg arg);
    void Handle(Display::ResizeEventArg arg);
    // void Handle(Core::InitializeEventArg arg);
    // void Handle(Core::ProcessEventArg arg);
    // void Handle(Core::DeinitializeEventArg arg);
    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    void SetWidth(const unsigned int width);
    void SetHeight(const unsigned int height);
    ITexture2DPtr GetTexture();

    void FadeIn(ICanvas* canvas, float duration);
    void FadeTo(ICanvas* canvas, float duration);

    void InitCanvas(ICanvas* canvas);
};

}
}

#endif //_DVA_STAGES_H_

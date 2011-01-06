// Heightfield node.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OCEANFLOOR_NODE_H_
#define _OCEANFLOOR_NODE_H_

#include <Scene/HeightMapNode.h>
#include <Resources/IShaderResource.h>
#include <Resources/ResourceManager.h>
#include <Meta/OpenGL.h>

#define SHADER

namespace OpenEngine {
namespace Scene {
        
    class OceanFloorNode : public HeightMapNode {
    private:
        unsigned int elapsedTime;
        ITexture2DPtr sand;

    public:
        OceanFloorNode() : HeightMapNode() {}
        OceanFloorNode(FloatTexture2DPtr tex) : HeightMapNode(tex) {
            elapsedTime = 0;
        }
        ~OceanFloorNode() {}

        void Initialize(RenderingEventArg arg) {
#ifdef SHADER
            string path = DirectoryManager::FindFileInPath("shaders/oceanfloor/oceanfloor.glsl");
            landscapeShader = ResourceManager<IShaderResource>::Create(path);
#else
            string path = DirectoryManager::FindFileInPath("textures/sand01.jpg");
            sand = ResourceManager<ITexture2D>::Create(path);
            sand->SetMipmapping(true);
            arg.renderer.LoadTexture(sand.get());
#endif
        }

        void Process(Core::ProcessEventArg arg){
            elapsedTime += arg.approx;
        }

        void PreRender(Renderers::RenderingEventArg arg) {
#ifdef SHADER
            this->landscapeShader->SetUniform("time", (float) elapsedTime / 12000000);
#else
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, sand->GetID());
#endif
        }

        void PostRender(Display::Viewport view) {
#ifndef SHADER
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
#endif
            
        }
    };
}
}

#endif

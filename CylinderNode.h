// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _CYLINDER_NODE_H_
#define _CYLINDER_NODE_H_

#include <Scene/RenderNode.h>
#include <Scene/TransformationNode.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Math/Vector.h>
#include <Math/Math.h>

using OpenEngine::Math::PI;
using OpenEngine::Scene::RenderNode;
using OpenEngine::Scene::ISceneNodeVisitor;

namespace dva {

class CylinderNode : public RenderNode {
public:
    CylinderNode(Vector<3,float> axisPoint0, Vector<3,float> axisPoint1, float radius) :
        axisPoint0(axisPoint0), axisPoint1(axisPoint1), radius(radius) {
    }

    void Apply(Renderers::RenderingEventArg arg, ISceneNodeVisitor& v) {
        // Draw cylinder axis
        arg.renderer.DrawLine( Geometry::Line(axisPoint0, axisPoint1), Vector<3,float>(1.0,0.0,0.0));

        float halfRadius = radius / 2.0f;

        for(float r=0; r<2*PI; r+=PI/45){
            float cX0 = (cos(r) * halfRadius) + axisPoint0[0];
            float cY0 = (sin(r) * halfRadius) + axisPoint0[1];

            float cX1 = (cos(r) * halfRadius) + axisPoint1[0];
            float cY1 = (sin(r) * halfRadius) + axisPoint1[1];

            Vector<3,float> p0(cX0, cY0, axisPoint0[2]);
            Vector<3,float> p1(cX1, cY1, axisPoint1[2]);

            arg.renderer.DrawLine( Geometry::Line(p0,p1), Vector<3,float>(1.0,0.0,0.0));
        }
    }
private:
    Vector<3,float> axisPoint0;
    Vector<3,float> axisPoint1;
    float radius;
};



} // NS dva

#endif

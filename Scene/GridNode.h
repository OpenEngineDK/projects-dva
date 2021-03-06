
#ifndef _GRID_NODE_
#define _GRID_NODE_

#include <Math/Vector.h>
#include <Scene/RenderNode.h>
#include <Scene/ISceneNodeVisitor.h>

using namespace OpenEngine;
using namespace OpenEngine::Math;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Renderers;

class GridNode : public Scene::RenderNode {
 public:
    GridNode(float numberOfLinesPerAxis,
             float spaceBetweenLines, Vector<3,float> color);
    ~GridNode();

    //void Apply(RenderingEventArg arg);
    void Apply(Renderers::RenderingEventArg arg, ISceneNodeVisitor& v);


 private:
    float numberOfLinesPerAxis;
    float spaceBetweenLines;
    Vector<3,float> color;
};

#endif // _GRID_NODE_

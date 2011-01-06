
#include "GridNode.h"
#include <Logging/Logger.h>

GridNode::GridNode(float numberOfLinesPerAxis,
                   float spaceBetweenLines, Vector<3,float> color) {
    this->numberOfLinesPerAxis = numberOfLinesPerAxis;
    this->spaceBetweenLines = spaceBetweenLines;
    this->color = color;
}

GridNode::~GridNode() {
}

void GridNode::Apply(Renderers::RenderingEventArg arg, ISceneNodeVisitor& v) /* = 0; (prohibited by Clone) */ {
    // draw xz plane as grid
    for (float i= -numberOfLinesPerAxis; i<numberOfLinesPerAxis; i+=spaceBetweenLines) {
        if (i == 0.0) continue;
        arg.renderer.DrawLine( Geometry::Line(Vector<3,float>(-numberOfLinesPerAxis,0.0,i),  Vector<3,float>(numberOfLinesPerAxis,0.0,i) ), color);
        arg.renderer.DrawLine( Geometry::Line(Vector<3,float>(i, 0.0, -numberOfLinesPerAxis), Vector<3,float>(i, 0.0, numberOfLinesPerAxis) ), color);
    }

    // Draw coordinate axis. red = x, green = y, blue = z
    arg.renderer.DrawLine( Geometry::Line(Vector<3,float>(0.0,0.0,0.0),  Vector<3,float>(100,0.0,0.0) ), Vector<3,float>(1.0,0.0,0.0));
    arg.renderer.DrawLine( Geometry::Line(Vector<3,float>(0.0,0.0,0.0),  Vector<3,float>(0.0,100.0,0.0) ), Vector<3,float>(0.0,1.0,0.0));
    arg.renderer.DrawLine( Geometry::Line(Vector<3,float>(0.0,0.0,0.0),  Vector<3,float>(0.0,0.0,100.0) ), Vector<3,float>(0.0,0.0,1.0));
    

    // Hack to clear the color
    arg.renderer.DrawLine( Geometry::Line(Vector<3,float>(0, 0.0, 0), 
                                          Vector<3,float>(0, 0.0, 0) ), 
                           Vector<3,float>(1,1,1));
    
}

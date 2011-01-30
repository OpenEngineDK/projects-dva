// wrapper around the 2D wave post process effect to provide.
// -------------------------------------------------------------------
// Copyright (C) 2010 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _DVA_LASER_WAVE_H_
#define _DVA_LASER_WAVE_H_

#include <Scene/PointWaveNode.h>
#include "../Devices/LaserSensor.h"

namespace OpenEngine {
namespace Scene {

using Devices::LaserSensor;

class LaserWaveNode 
    : public PointWaveNode
    , public IListener<Devices::LaserInputEventArg>
{
private: 
    LaserSensor* l;
    unsigned int count;
public:
    LaserWaveNode(LaserSensor* l, unsigned int width, unsigned int height, unsigned int maxPoints)
        : PointWaveNode(width, height, maxPoints)
        , l(l)
        , count(0)
    {
        l->LaserInputEvent().Attach(*this);
    }

    virtual ~LaserWaveNode() {
        l->LaserInputEvent().Detach(*this);
    }

    void Handle(Devices::LaserInputEventArg arg) {
        if (++count > 4) {
            count = 0;
        
            std::vector< Vector<2,float> > ps =  l->GetState();
            std::vector< Vector<2,float> >::iterator itr = ps.begin();

            deque<pair<float,Vector<2,float> > >::iterator itr1;

            for (; itr != ps.end(); ++itr) {
                Vector<2,int> p = InputController::LaserPointToScreenCoordinates(*itr);
                
                Vector<2,float> point = Vector<2,float>(p[0]/float(dimensions[0]), (float(dimensions[1])-p[1])/float(dimensions[1]));
        
                // logger.info << "laserpoint: " << point << logger.end;
                itr1 = points.begin();
                bool add = true;
                for (; itr1 != points.end(); ++itr1) {
                    if ((point - (*itr1).second).GetLength() < 0.10) {
                        add = false;
                        break;
                    }
                }
                if (add) {
                    // logger.info << "add point: " << point << logger.end;
                    AddPoint(point);
                }
            }
        }
    }
};

}
}

#endif //_DVA_LASER_WAVE_H_

// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _CLUSTER_ANALYSER_H_
#define _CLUSTER_ANALYSER_H_

#include <vector>
#include <Math/Vector.h>

namespace OpenEngine {
namespace Devices {

    using namespace OpenEngine::Math;
using namespace std;
/**
 * Short description.
 *
 * @class ClusterAnalyser ClusterAnalyser.h s/dva/Devices/ClusterAnalyser.h
 */
class ClusterAnalyser {
private:
    float epsilon;
    unsigned int minClusterPoints;

    vector<unsigned int> GetNeighbours(vector< Vector<2,float> > points, 
                                       Vector<2,float> point,
                                       float epsilon);

    Vector<2,float> GetClusterCenter(std::vector< Vector<2,float> > cluster);

public:
    ClusterAnalyser(float epsilon, unsigned int minClusterPoints);
    ~ClusterAnalyser();

    vector< Vector<2,float> > AnalyseDataSet(vector< Vector<2,float> > points);
    
  
};

} // NS Devices
} // NS OpenEngine

#endif

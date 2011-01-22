// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include "ClusterAnalyser.h"
#include <iostream>

namespace OpenEngine {
namespace Devices {


ClusterAnalyser::ClusterAnalyser(float epsilon, unsigned int minClusterPoints) :
    epsilon(epsilon), minClusterPoints(minClusterPoints) {
}

ClusterAnalyser::~ClusterAnalyser() {
}


vector<unsigned int> ClusterAnalyser::GetNeighbours(vector< Vector<2,float> > points, 
                                                    Vector<2,float> point,
                                                    float epsilon) {
    // Check distance to all other points, if less than epsilon add as neighbour.
    vector<unsigned int> neighbours;
    
    for(unsigned int i=0; i<points.size(); i++){
        float dist = (points[i] - point).GetLength();
        //        std::cout << "WHAT: " << dist << std::endl;
        
        if( dist < epsilon )
            neighbours.push_back(i);
    }
    return neighbours;
}


Vector<2,float> ClusterAnalyser::GetClusterCenter(std::vector< Vector<2,float> > cluster){

    if( cluster.size() == 0 ) 
        return Vector<2,float>(0.0);

    Vector<2,float> center;

    for(unsigned int i=0; i<cluster.size(); i++){
        center += cluster[i];
    }
     // 
    center /= cluster.size();
    return center;
}

/* Algorithm used for cluster analysis

   DBSCAN(D, eps, MinPts)
    C = 0
    for each unvisited point P in dataset D
      mark P as visited
      N = getNeighbors (P, eps)
      if sizeof(N) < MinPts
        mark P as NOISE
      else
        C = next cluster
        expandCluster(P, N, C, eps, MinPts)
   
     expandCluster(P, N, C, eps, MinPts)
       add P to cluster C
       for each point P' in N 
         if P' is not visited
           mark P' as visited
           N' = getNeighbors(P', eps)
           if sizeof(N') >= MinPts
             N = N joined with N'
          if P' is not yet member of any cluster
            add P' to cluster C
*/
vector< Vector<2,float> > ClusterAnalyser::AnalyseDataSet(vector< Vector<2,float> > points) {
    // List of clusters
    vector< Vector<2,float> > clusterCenters;

    // Check if there are any points to analyse (returns empty set).
    if( points.size() == 0 ) return clusterCenters; 

    // List of visited points.
    bool visited[points.size()];
    memset(&visited, 0, points.size());
    //
    bool assignedToCluster[points.size()];
    memset(&assignedToCluster, 0, points.size());

    // Iterate through all unvisited points
    for(unsigned int i=0; i<points.size(); i++){
         // Only loop unvisited points
        if( visited[i] ) continue;

        // Mark as visited
        visited[i] = true;

        // Get point.
        Vector<2,float> point = points[i];

        // Get indexes to all neighbouring points.
        vector<unsigned int> neighbours = GetNeighbours(points, point, epsilon);

        if( neighbours.size() > minClusterPoints ){
            // Form new cluster.
            std::vector< Vector<2,float> > cluster;
            // Add current point to cluster
            cluster.push_back(point);
            assignedToCluster[i] = true;

            for(unsigned int n=0; n<neighbours.size(); n++){
                // Get index of the n'th neighbour
                unsigned int nId = neighbours[n];
                // Get point data for the n'th neighbour
                Vector<2,float> nPoint = points[nId];
                if(!visited[nId]){
                    visited[nId] = true;
                    vector<unsigned int> nNeighbours = GetNeighbours(points, nPoint, epsilon);

                    // Enough support => join neighbours and neighbours-neighbours.
                    if( nNeighbours.size() >= minClusterPoints ){
                        // TODO: change GetNeighbours return type to list, then perform sort and unique here...
                        neighbours.insert(neighbours.end(), nNeighbours.begin(), nNeighbours.end());
                    }
                }

                // If nId is not assigned to any cluster add it to this cluster.
                if( !assignedToCluster[nId] ){
                    cluster.push_back(nPoint);
                    assignedToCluster[nId] = true;
                }
            }

            // Calculate center of cluster
            Vector<2,float> center = GetClusterCenter(cluster);
            clusterCenters.push_back(center);
        }
    }
    return clusterCenters;
}


} // NS Devices
} // NS OpenEngine

// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _SETUP_H_
#define _SETUP_H_

#include <string>

typedef enum {
    NONE,
    MOUSE_SPHERE_FLEE,
    MOUSE_CYLINDER_FLEE,
    MOUSE_FLOCK_FOLLOW,
    LASER_SPHERE_FLEE,
    LASER_CYLINDER_FLEE,
    LASER_FOLLOW_FLOCK_AND_RESIZE
} CtrlMode;


namespace dva {
    const unsigned int SCREEN_WIDTH  = 1024;
    const unsigned int SCREEN_HEIGHT = 768; 

    const CtrlMode INPUT_CTRL_MODE = LASER_FOLLOW_FLOCK_AND_RESIZE;

    // Laser Sensor Setup
    const std::string LASER_SENSOR_IP     = "192.168.0.100";
    const int         LASER_SENSOR_PORT   = 2111; // 2111 or 2112
    const bool        LASER_DEBUG_ENABLED = true;

    // Cluster Analyser
    const float CLUSTER_EPSILON    = 0.10; // Max distance between points in cluster.
    const int   CLUSTER_MIN_POINTS = 5;    // Minimum number of points to form a cluster.

    

    
} // NS dva

#endif // _SETUP_H_

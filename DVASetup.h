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
    extern unsigned int SCREEN_WIDTH;
    extern unsigned int SCREEN_HEIGHT;

    const CtrlMode INPUT_CTRL_MODE = MOUSE_FLOCK_FOLLOW;
    //const CtrlMode INPUT_CTRL_MODE = LASER_FOLLOW_FLOCK_AND_RESIZE;


    // Laser Sensor Setup
    const bool        LASER_SENSOR_ENABLED = true;
    const std::string LASER_SENSOR_IP      = "192.168.0.100";
    const int         LASER_SENSOR_PORT    = 2111; // 2111 or 2112
    const bool        LASER_DEBUG_ENABLED  = false;

    // Relay Box Setup
    const bool        RELAY_BOX_ENABLED    = true;
    const std::string RELAY_BOX_IP         = "192.168.0.101";
    const int         RELAY_BOX_PORT       = 12302;

    // Cluster Analyser
    const float CLUSTER_EPSILON    = 0.10; // Max distance between points in cluster.
    const int   CLUSTER_MIN_POINTS = 5;    // Minimum number of points to form a cluster.


    // 
    const unsigned int NUM_SEAWEED_ANIMATORS = 3;

    
} // NS dva

#endif // _SETUP_H_

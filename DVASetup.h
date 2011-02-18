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

    // Projector Setup (TCP/IP -> RS232 through Relay Box)
    const bool        PROJECTOR_ENABLED    = true;  // Send wake up message to projector.
    const std::string PROJECTOR_IP         = "192.168.0.101";
    const int         PROJECTOR_PORT       = 10001;
    

    // Cluster Analyser
    const float CLUSTER_EPSILON    = 0.08; // Max distance between points in cluster.
    const int   CLUSTER_MIN_POINTS = 5;    // Minimum number of points to form a cluster.

    // Sound System
    const bool SOUND_ENABLED = true;

    // 
    const unsigned int NUM_SEAWEED_ANIMATORS = 3;

    const bool CHECK_LICENSE = true;
    
} // NS dva

#endif // _SETUP_H_

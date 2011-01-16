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

namespace dva {
    const unsigned int SCREEN_WIDTH  = 1024;
    const unsigned int SCREEN_HEIGHT = 768; 

    const std::string LASER_SENSOR_IP   = "192.168.0.100";
    const int         LASER_SENSOR_PORT = 2111; // 2111 or 2112

} // NS dva

#endif // _SETUP_H_

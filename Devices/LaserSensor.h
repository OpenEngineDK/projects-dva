
//
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _LASER_SENSOR_H_
#define _LASER_SENSOR_H_

#include <Core/IModule.h>
#include <string>


namespace OpenEngine {
namespace Devices {

using namespace OpenEngine::Core;

class SICKDeviceDriver;

/**
 * Short description.
 *
 * @class LaserSensor LaserSensor.h s/dva/Devices/LaserSensor.h
 */
class LaserSensor : public IModule {
public:
    enum SensorStatus {
        NOT_CONNECTED,   // Idle and not connected yet.
        CONNECTING,      // Connection in progress.
        CONNECTED,       // Connected successfully and receiving data.
        CONNECTION_ERR,  // Could not connect.
        DATA_ERR,        // Connected but cannot parse input data.
    };

    LaserSensor();
    ~LaserSensor();

    bool Connect(std::string ip, unsigned short port);    
    SensorStatus GetSensorStatus();

    // IModule handlers
    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);
   
private:
    SICKDeviceDriver* device;
    SensorStatus status;

};

} // NS Devices
} // NS OpenEngine

#endif

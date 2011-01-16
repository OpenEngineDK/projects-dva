// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include "LaserSensor.h"
#include "SICKDeviceDriver.h"
#include <Math/Vector.h>
#include <Logging/Logger.h>


namespace OpenEngine {
namespace Devices {

using namespace OpenEngine::Math;

LaserSensor::LaserSensor() : status(NOT_CONNECTED) {
    device = new SICKDeviceDriver(0, 180, 1.0, Vector<2,float>(100,100));
}

LaserSensor::~LaserSensor() {
    if( device )
        delete device;
}


bool LaserSensor::Connect(string ip, unsigned short port) {
    if( status == NOT_CONNECTED || status == CONNECTION_ERR ){
        // Try to connect to device
        status = CONNECTING;
        logger.info << "[LaserSensor] Connecting to sensor..." << logger.end;
        // Connect the device driver.
        status = device->Connect(ip, port) ? CONNECTED : CONNECTION_ERR;
        // Print status
        if( status == CONNECTED ) 
            logger.info << "[LaserSensor] Connected successfully." << logger.end;
        else
            logger.info << "[LaserSensor] Error connecting to sensor..." << logger.end;
    }
    return (status == CONNECTED);
}

void LaserSensor::Handle(Core::InitializeEventArg arg) {
    logger.warning << "LASER SENSOR INIT" << logger.end;
    // If connected to device, process readings.
    if( status == CONNECTED ){
        logger.info << "[LaserSensor] Starting SICK device driver" << logger.end;
        device->Start();
    }
}


void LaserSensor::Handle(Core::ProcessEventArg arg) {
    // Get data from device driver.

    // Parse sensor readings.

    // Cluster analyse sensor readings.
    
}

void LaserSensor::Handle(Core::DeinitializeEventArg arg) {
    // Close connection to device
    device->Close();
    status = NOT_CONNECTED;
}


} // NS Devices
} // NS OpenEngine

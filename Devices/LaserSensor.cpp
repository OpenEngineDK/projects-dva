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

LaserSensor::LaserSensor(string ip, unsigned short port) {
    device = new SICKDeviceDriver(ip, port, 0, 180, 1.0, Vector<2,float>(10,10));
}

LaserSensor::~LaserSensor() {
    if( device )
        delete device;
}


void LaserSensor::Connect() {
    if( device->GetStatus() == NOT_CONNECTED ) {
        // Try to connect to device.
        logger.info << "[LaserSensor] Connecting to sensor..." << logger.end;
        device->Start();
    }
}

void LaserSensor::Handle(Core::InitializeEventArg arg) {
    logger.info << "[LaserSensor] Starting SICK device driver" << logger.end;
    Connect();
}


void LaserSensor::Handle(Core::ProcessEventArg arg) {
    // Check device status
    if( device->GetStatus() == NOT_CONNECTED ){
        // Connection probably lost, reconnect..
        logger.error << "[LaserSensor] Error: connection lost, reconnecting..." << logger.end;
        Connect();
    }
  
    // Get data from device driver.
    std::list< Math::Vector<2,float> > readings = device->GetReadings();
    UpdateCalibrationCanvas(readings);
    
    // Debug print points
    std::list< Math::Vector<2,float> >::iterator itr;
    for(itr=readings.begin(); itr!=readings.end(); itr++){
        logger.info << "Laser: " << *itr << logger.end;
    }

    // Cluster analyse sensor readings.
    
}

void LaserSensor::Handle(Core::DeinitializeEventArg arg) {
    // Close connection to device
    device->Close();
}

void LaserSensor::SetLaserDebug(LaserDebugPtr debug){
    this->laserDebug = debug;
}
    
void LaserSensor::UpdateCalibrationCanvas(std::list< Math::Vector<2,float> > readings) {
    if( laserDebug ){
        
        int width = laserDebug->GetWidth();
        int height = laserDebug->GetHeight();
        
        unsigned char* canvasPtr = laserDebug->GetData();

        // Calculate all canvas points.
        std::list< Math::Vector<2,float> >::iterator itr;
        for(itr=readings.begin(); itr!=readings.end(); itr++){
            Vector<2,float> reading = *itr;
            int x = abs((int)(reading[0] * (width-1)));
            int y = abs((int)(reading[1] * (height-1)));

            canvasPtr[(y*width + x)*4+0] = 255.0;
            canvasPtr[(y*width + x)*4+1] = 0.0;
            canvasPtr[(y*width + x)*4+2] = 0.0;
            canvasPtr[(y*width + x)*4+3] = 1.0;
        }
        //
        laserDebug->UpdateTexture();
    }
}


} // NS Devices
} // NS OpenEngine

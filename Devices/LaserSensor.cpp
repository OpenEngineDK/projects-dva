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
    device = new SICKDeviceDriver(ip, port, -PI/4.0, PI+(PI/4.0), PI/360.0, Vector<2,float>(800,800));
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


vector< Vector<2, float> > LaserSensor::GetState() {
    // Return analysed data from device driver.
    return device->GetClusters();
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
    std::vector< Math::Vector<2,float> > readings = device->GetReadings();
    // Get analysed data from device driver.
    std::vector< Math::Vector<2,float> > clusters = device->GetClusters();

    // If the laser has found any clusters, notify listeners.
    if( clusters.size() > 0 ){
        laserInputEvent.Notify(LaserInputEventArg());
    }

    // Just for debugging purpose.
    UpdateCalibrationCanvas(readings, clusters);
}

void LaserSensor::Handle(Core::DeinitializeEventArg arg) {
    // Close connection to device
    device->Close();
}

void LaserSensor::SetLaserDebug(LaserDebugPtr debug){
    this->laserDebug = debug;
}
    

Core::IEvent<LaserInputEventArg>& LaserSensor::LaserInputEvent() {
    return this->laserInputEvent;

}

void LaserSensor::UpdateCalibrationCanvas(std::vector< Math::Vector<2,float> > readings,
                                          std::vector< Math::Vector<2,float> > clusters) {
    if( laserDebug ){

        int width = laserDebug->GetWidth();
        int height = laserDebug->GetHeight();
        
        // Clear canvas
        unsigned char* canvasPtr = laserDebug->GetData();
        std::memset(canvasPtr,0,height*width*4);

        // Calculate all canvas points.
        std::vector< Math::Vector<2,float> >::iterator itr;
        for(itr=readings.begin(); itr!=readings.end(); itr++){
            Vector<2,float> reading = *itr;
            int x = ((reading[0] + 1) / 2.0) * (width - 1);
            //int y = ((reading[1] + 1) / 2.0) * (height - 1);
            int y = reading[1] * (height - 1);

            SetPixel(x,y,Vector<4,unsigned char>(0,255,0,255));
        }

        // Calculate all canvas points.
        for(unsigned int i=0; i<clusters.size(); i++){
            Vector<2,float> cc = clusters[i];
            int xPos = ((cc[0] + 1) / 2.0) * (width - 1);
            int yPos = cc[1] * (height - 1);
            
            for(float r=0; r<2*PI; r+=PI/180){
                int cX = (int)(cos(r) * 10) + xPos;
                int cY = (int)(sin(r) * 10) + yPos;
                SetPixel(cX,cY,Vector<4,unsigned char>(255,0,0,255));
            }
            SetPixel(100,100,Vector<4,unsigned char>(255,0,0,255));
        }

        //std::cout << "NumClusters: " << clusters.size() << endl;
        //
        laserDebug->UpdateTexture();
    }
}


void LaserSensor::SetPixel(unsigned int x, unsigned int y, 
                           Vector<4,unsigned char> color) {

    int width = laserDebug->GetWidth();
    unsigned char* canvasPtr = laserDebug->GetData();

    canvasPtr[(y*width + x)*4+0] = color[0];
    canvasPtr[(y*width + x)*4+1] = color[1];
    canvasPtr[(y*width + x)*4+2] = color[2];
    canvasPtr[(y*width + x)*4+3] = color[3];
}


} // NS Devices
} // NS OpenEngine

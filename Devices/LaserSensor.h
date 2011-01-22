
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
#include <Resources/Texture2D.h>
#include "../LaserDebug.h"
#include <string>
#include <vector>


namespace OpenEngine {
namespace Devices {

using namespace OpenEngine::Core;
using namespace OpenEngine::Resources;
using namespace dva;

class SICKDeviceDriver;


/**
 * Short description.
 *
 * @class LaserSensor LaserSensor.h s/dva/Devices/LaserSensor.h
 */
class LaserSensor : public IModule {
public:
    LaserSensor(std::string ip, unsigned short port);
    ~LaserSensor();

    void Connect();    
    std::vector< Vector<2,float> > GetState();


    // IModule handlers
    void Handle(Core::InitializeEventArg arg);
    void Handle(Core::ProcessEventArg arg);
    void Handle(Core::DeinitializeEventArg arg);

    void SetLaserDebug(LaserDebugPtr debug);
   
private:
    SICKDeviceDriver* device;
    LaserDebugPtr laserDebug;

    void UpdateCalibrationCanvas(std::vector< Math::Vector<2,float> > readings,
                                 std::vector< Math::Vector<2,float> > clusters);

    void SetPixel(unsigned int x, unsigned int y, 
                  Vector<4,unsigned char> color); 
};

} // NS Devices
} // NS OpenEngine

#endif

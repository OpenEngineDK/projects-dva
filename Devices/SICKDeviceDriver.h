// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _SICK_DEVICE_DRIVER_H_
#define _SICK_DEVICE_DRIVER_H_

#include <Core/Thread.h>
#include <Network/TCPSocket.h>
#include <Math/Vector.h>
#include <list>

namespace OpenEngine {
namespace Devices {

using namespace OpenEngine::Network;
using namespace OpenEngine::Core;

const string SCAN_DATA = "sRN LMDscandata";
const unsigned int STX = 0x02;
const unsigned int ETX = 0x03;


/**
 * Short description.
 *
 * @class SICKDeviceDriver SICKDeviceDriver.h s/dva/Devices/SICKDeviceDriver.h
 */
class SICKDeviceDriver : public Thread {
private:
    TCPSocket* socket;

    int startAngle, endAngle;
    float resolution;
    Math::Vector<2,float> bounds;

    string reqMsg; // Request measurements from SICK LMS100 sensor.
    string term;   // Termination string used in sensor reply.

    // SICK specific data parser
    std::list< Math::Vector<2,float> > ParseData(string data);

public:
    SICKDeviceDriver(int startAngle, int endAngle, float resolution, Math::Vector<2,float> rectBounds);
    ~SICKDeviceDriver();

    bool Connect(string ip, int port);
    void Close();

    std::list< Math::Vector<2,float> > GetReadings();

    // Thread loop
    void Run();
    
};

} // NS Devices
} // NS OpenEngine

#endif

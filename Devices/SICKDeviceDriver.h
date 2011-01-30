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
#include <Core/Mutex.h>
#include <Network/TCPSocket.h>
#include <Network/TCPTypes.h>
#include <Math/Vector.h>
#include "ClusterAnalyser.h"
#include <list>

namespace OpenEngine {
namespace Devices {

using namespace OpenEngine::Network;
using namespace OpenEngine::Core;

const string SCAN_DATA = "sRN LMDscandata";
const unsigned int STX = 0x02;
const unsigned int ETX = 0x03;
const unsigned int IDX_OFFSET = 26;


/**
 * Device driver for the SICK LMS-100 Laser Sensor.
 *
 * @class SICKDeviceDriver SICKDeviceDriver.h s/dva/Devices/SICKDeviceDriver.h
 */
class SICKDeviceDriver : public Thread {
private:
    NetStat status;
    TCPSocket* socket;
    std::string deviceIp;
    unsigned short devicePort;

    float startAngle, endAngle;
    float resolution;
    Math::Vector<2,float> bounds;
    Math::Vector<2,float> boundsOffset;
 
    Mutex mutex;
    std::vector< Math::Vector<2,float> > curReadings;
    std::vector< Math::Vector<2,float> > curClusters;

    ClusterAnalyser* clusterAnalyser;

    string reqMsg; // Request measurements from SICK LMS100 sensor.
    string term;   // Termination string used in sensor reply.

    // SICK specific data parser
    std::vector< Math::Vector<2,float> > ParseData(string data);

    // Check bounding condition on readings.
    bool InsideBounds(Math::Vector<2,float> p);

public:
    SICKDeviceDriver(string ip, unsigned short port, 
                     float startAngle, float endAngle, 
                     float resolution, Math::Vector<2,float> rectBounds);
    ~SICKDeviceDriver();

    bool Connect(string ip, int port);
    void Close();
    NetStat GetStatus();

    std::vector< Math::Vector<2,float> > GetReadings();
    std::vector< Math::Vector<2,float> > GetClusters();

    // Thread loop
    void Run();
};

} // NS Devices
} // NS OpenEngine

#endif

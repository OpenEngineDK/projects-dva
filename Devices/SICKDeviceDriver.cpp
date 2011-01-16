// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include "SICKDeviceDriver.h"
#include <Math/Vector.h>
#include <Logging/Logger.h>
#include <string>
#include <iostream>
#include <vector>
#include <iostream>
#include <istream>
#include <ostream>
#include <iterator>
#include <sstream>
#include <algorithm>

namespace OpenEngine {
namespace Devices {

using namespace OpenEngine::Math;

SICKDeviceDriver::SICKDeviceDriver(int startAngle, int endAngle, float resolution, Vector<2,float> rectBounds) :
    socket(NULL), startAngle(startAngle), endAngle(endAngle), resolution(resolution), bounds(rectBounds) {

    // Create request message.
    stringstream r;
    r  << (char)STX << SCAN_DATA << (char)ETX;
    reqMsg = r.str();

    // Create termination signature.
    stringstream t;
    t << (char)ETX;
    term = t.str();
}

SICKDeviceDriver::~SICKDeviceDriver() {
}


std::list< Vector<2,float> > SICKDeviceDriver::ParseData(string data) {
    
    list< Vector<2,float> > points;

    // Split string into n separate measurements.
    stringstream strstr(data);
    // Use stream iterators to copy the stream to the vector as whitespace separated strings
    istream_iterator<string> it(strstr);
    istream_iterator<string> end;
    vector<string> results(it, end);

    logger.info << "Laser sensor num readings: " << results.size() << logger.end;

//     vector<string>::iterator itr;
//     for(itr=results.begin(); itr!=results.end(); itr++){
//         logger.info << *itr << logger.end;
//     }

        // Calculate max number of measurements.

        // Calculate intersection points.

        // return result.
    return points;
}

bool SICKDeviceDriver::Connect(string ip, int port) {
    if( socket ){
        delete socket;
    }
    socket = new TCPSocket(port);
    return socket->Connect(ip);
}

void SICKDeviceDriver::Close() {
    socket->Close();
}


std::list< Vector<2,float> > SICKDeviceDriver::GetReadings() {
}

// Thread loop
void SICKDeviceDriver::Run(){
    // Loop while connection to device is open.
    while( socket->IsOpen() ) {

        // Request measurements from laser sensor.
        socket->SendLine(reqMsg);

        // Read measurements from laser sensor (ETX terminated).
        string data = socket->ReadLine(term);

        // Parse sensor readings.
        if( data.length() > 0 ){
            ParseData(data);
        }
        // Perform cluster analysis.
        
        
    }
    logger.error << "SOCKET CLOSED" << logger.end;
}


} // NS Devices
} // NS OpenEngine

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

SICKDeviceDriver::SICKDeviceDriver(string ip, unsigned short port, 
                                   float startAngle, float endAngle, 
                                   float resolution, Vector<2,float> rectBounds) :
    status(NOT_CONNECTED), 
    socket(NULL), 
    deviceIp(ip), 
    devicePort(port), 
    startAngle(startAngle), 
    endAngle(endAngle), 
    resolution(resolution), 
    bounds(rectBounds) {

    // epsilon, min cluster points
    clusterAnalyser = new ClusterAnalyser(0.10, 10);

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
    delete clusterAnalyser;
}


std::vector< Vector<2,float> > SICKDeviceDriver::ParseData(string data) {
    // Split string into n separate measurements.
    stringstream strstr(data);
    // Use stream iterators to copy the stream to the vector as whitespace separated strings
    istream_iterator<string> it(strstr);
    istream_iterator<string> end;
    vector<string> results(it, end);

    // List for final points.
    vector< Vector<2,float> > points;

    //    logger.info << "Laser sensor num readings: " << results.size() << logger.end;
    if( results.size() > 0 ){

        float curAngle = startAngle;

        // 
        for(unsigned int i=IDX_OFFSET; i<results.size(); i++ ) {
            int dist;
            stringstream ss;
            ss << std::hex << results[i];
            ss >> dist;
            
            float cosAngle = cos(curAngle);
            float sinAngle = sin(curAngle);
            float x = cosAngle * dist;
            float y = sinAngle * dist;

            //            logger.info << "[" << i << "]" << results[i] << ", dist: " << dist << ", (x,y): " << x << ", " << y << logger.end;

            Vector<2,float> point(x,y); 

            // Check bounding condition.
            if( InsideBounds(point) ){
                // Calculate relative position according to bounding box.
                Vector<2,float> relativePos;
                relativePos[0] = point[0] / bounds[0];
                relativePos[1] = point[1] / bounds[1];
                points.push_back(relativePos);
            }

            // Update angle
            curAngle = curAngle + resolution;
        }
    }
    // return result.
    return points;
}

bool SICKDeviceDriver::InsideBounds(Vector<2,float> p) {
    return p[1] > 0 && // Ignore negative y
           (-bounds[0] < p[0]) && (p[0] < bounds[0]) &&
           (-bounds[1] < p[1]) && (p[1] < bounds[1]);
}


bool SICKDeviceDriver::Connect(string ip, int port) {
    if( status == NOT_CONNECTED ){
        // Change status to connecting..
        status = CONNECTING;
        
        if( socket ){
            socket->Close();
            delete socket;
        }
        
        socket = new TCPSocket(port);
        if( socket->Connect(ip) ){
            status = CONNECTED;
            return true;
        }
    }
    status = NOT_CONNECTED;
    return false;
}

void SICKDeviceDriver::Close() {
    socket->Close();
    status = NOT_CONNECTED;
}

NetStat SICKDeviceDriver::GetStatus() {
    return status;
}


std::vector< Vector<2,float> > SICKDeviceDriver::GetReadings() {
    mutex.Lock();
    std::vector< Vector<2,float> > res = curReadings;
    mutex.Unlock();
    return res;
}

std::vector< Math::Vector<2,float> > SICKDeviceDriver::GetClusters() {
    mutex.Lock();
    std::vector< Vector<2,float> > res = curClusters;
    mutex.Unlock();
    return res;
}


// Thread loop
void SICKDeviceDriver::Run(){

    // Start by connecting
    if( !Connect(deviceIp, devicePort) ) {
        logger.error << "[SICKDeviceDriver] Error: Could not connect to device ip: " << deviceIp << ":" << devicePort << logger.end;
        return;
    }
    logger.info << "[SICKDeviceDriver] Connected successfully." << logger.end;

    
    // Loop while connection to device is open.
    while( socket->IsOpen() ) {
        string data;

        try {
            // Request measurements from laser sensor.
            socket->SendLine(reqMsg);
            
            // Read measurements from laser sensor (ETX terminated).
            data = socket->ReadLine(term);
        } catch (...) {
            logger.error << "[SICKDeviceDriver] socket error." << logger.end;
        }


        // Parse sensor readings.
        if( data.length() > 0 ){
            mutex.Lock();
            curReadings = ParseData(data);
            mutex.Unlock();
        
            
            // Perform cluster analysis.            
            if( curReadings.size() > 0 ){
                vector< Vector<2,float> > clusterCenters;
                clusterCenters = clusterAnalyser->AnalyseDataSet(curReadings);

                mutex.Lock();
                curClusters = clusterCenters;
                mutex.Unlock();
            }
        }
        
        
        
    }
    // We lost the connection for some reason.
    status = NOT_CONNECTED;
    logger.info << "[SICKDeviceDriver] socket closed." << logger.end;
}


} // NS Devices
} // NS OpenEngine

// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include "Projector.h"
#include <Network/TCPSocket.h>
#include <Logging/Logger.h>
#include <utility>

namespace dva {

using namespace std;

Projector::Projector() 
    : status(NOT_CONNECTED)
    , socket(NULL)
    , deviceIp("")
    , devicePort(0)
    , done(false) {
}

Projector::Projector(std::string ip, unsigned short port) 
    : status(NOT_CONNECTED)
    , socket(NULL)
    , deviceIp(ip)
    , devicePort(port)
    , done(false) {
}

Projector::~Projector() {
}

void Projector::SendCmd(string cmd) {
    if( GetStatus() == CONNECTED ) {
        mutex.Lock();
        cmdQueue.push(cmd);
        mutex.Unlock();
    } else {
        logger.warning << "[Projector] Warning: could not send command due to broken connection.." << logger.end;
    }
}

bool Projector::Connect(std::string ip, unsigned short port) {
    this->deviceIp = ip;
    this->devicePort = port;

    if( status == NOT_CONNECTED ){
        // Change status to connecting..
        status = CONNECTING;
        logger.info << "[Projector] Attempting to connect to " << ip << ":" << port << logger.end;
        
        if( socket ){
            socket->Close();
            delete socket;
        }
        
        socket = new TCPSocket(port);
        if( socket->Connect(ip) ){
            status = CONNECTED;
            logger.info << "[Projector] Connected successfully." << logger.end;
            return true;
        }else {
            logger.error << "[Projector] Error: Could not connect to: " << deviceIp << ":" << devicePort << logger.end;
        }
    }
    status = NOT_CONNECTED;
    return false;
}

void Projector::Close() {
    done = true;
    socket->Close();
    status = NOT_CONNECTED;
}

NetStat Projector::GetStatus() {
    return status;
}


void Projector::Run() {

    while( !done ) {
        // Start by connecting
        if( GetStatus() == NOT_CONNECTED ){
            if( !Connect(deviceIp, devicePort) ) {
                logger.error << "[Projector] waiting 20 sec before reconnecting...." << logger.end;
                Thread::Sleep(20000000);
            }
        }

        // If socket is open and the queue is not empty - work...
        while( GetStatus() == CONNECTED && !cmdQueue.empty() ) {
            mutex.Lock();
            string cmd = cmdQueue.front();
            cmdQueue.pop();
            mutex.Unlock();
            
            try {
                // Send command to the relay box. 
                socket->SendLine(cmd);
                
                // If the response is not an error the command was interpreted correctly.
                string res = socket->ReadLine("\r");
                if(  res.find("?") != string::npos ){
                    logger.error << "[Projector] command: '" << cmd << "' is unknown."  << logger.end;
                }

            } catch (...) {
                logger.error << "[Projector] socket error." << logger.end;
            }
        }
        // Avoid CPU hammering by sleeping 10ms.
        Thread::Sleep(10000);
    }
}

void Projector::PowerOn() {
    SendCmd("PWR1\r");
}

void Projector::PowerOff() {
    SendCmd("PWR0\r");
}

} // NS dva


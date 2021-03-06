// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#include "RelayBox.h"
#include <Network/TCPSocket.h>
#include <Logging/Logger.h>
#include <utility>

namespace dva {

using namespace std;

RelayBox::RelayBox() 
    : status(NOT_CONNECTED)
    , socket(NULL)
    , deviceIp("")
    , devicePort(0)
    , done(false) {
}

RelayBox::RelayBox(std::string ip, unsigned short port) 
    : status(NOT_CONNECTED)
    , socket(NULL)
    , deviceIp(ip)
    , devicePort(port)
    , done(false) {
}

RelayBox::~RelayBox() {
}

void RelayBox::SendCmd(string cmd) {
    if( GetStatus() == CONNECTED ) {
        mutex.Lock();
        cmdQueue.push(cmd);
        mutex.Unlock();
    } else {
        logger.warning << "[RelayBox] Warning: could not send command due to broken connection.." << logger.end;
    }
}

bool RelayBox::Connect(std::string ip, unsigned short port) {
    this->deviceIp = ip;
    this->devicePort = port;

    if( status == NOT_CONNECTED ){
        // Change status to connecting..
        status = CONNECTING;
        logger.info << "[RelayBox] Attempting to connect to " << ip << ":" << port << logger.end;
        
        if( socket ){
            socket->Close();
            delete socket;
        }
        
        socket = new TCPSocket(port);
        if( socket->Connect(ip) ){
            status = CONNECTED;
            logger.info << "[RelayBox] Connected successfully." << logger.end;
            return true;
        }else {
            logger.error << "[RelayBox] Error: Could not connect to: " << deviceIp << ":" << devicePort << logger.end;
        }
    }
    status = NOT_CONNECTED;
    return false;
}

void RelayBox::Close() {
    done = true;
    socket->Close();
    status = NOT_CONNECTED;
}

NetStat RelayBox::GetStatus() {
    return status;
}


void RelayBox::Run() {

    while( !done ) {
        // Start by connecting
        if( GetStatus() == NOT_CONNECTED ){
            if( !Connect(deviceIp, devicePort) ) {
                logger.error << "[RelayBox] waiting 10 sec before reconnecting...." << logger.end;
                Thread::Sleep(10000000);
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
                if(  res.find("cmderr") != string::npos ){
                    logger.error << "[RelayBox] command: '" << cmd << "' is unknown."  << logger.end;
                }

            } catch (...) {
                logger.error << "[RelayBox] socket error." << logger.end;
            }
        }
        // Avoid CPU hammering by sleeping 10ms.
        Thread::Sleep(10000);
    }
}

void RelayBox::SetRelayState(unsigned int relayNo, bool state) {
    stringstream cmd;
    cmd << "setio," << relayNo << "," << (int)state << "\r";
    SendCmd(cmd.str());
}


void RelayBox::ToggleRelay(unsigned int relayNo) {
    stringstream cmd;
    cmd << "setio," << relayNo << "," << (int)999 << "\r";
    SendCmd(cmd.str());
}

void RelayBox::DeactivateAll() {
    for( unsigned int i=1; i<=NUM_RELAYS; i++){
        SetRelayState(i, false);
    }
}

void RelayBox::ActivateAll() {
    for( unsigned int i=1; i<=NUM_RELAYS; i++){
        SetRelayState(i, true);
    }
}


} // NS dva


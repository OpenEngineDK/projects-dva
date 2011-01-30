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

void RelayBox::SendCmd(string req, string res) {
    //   logger.info << "SendCmd: " << req << ", " << res << logger.end;
    if( GetStatus() == CONNECTED ) {
        mutex.Lock();
        cmdQueue.push( make_pair(req, res) );
        mutex.Unlock();
    } else {
        logger.warning << "[RelayBox] Warning: could not send command since the connection is lost.." << logger.end;
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
                Thread::Sleep(10000);
            }
        }

        // If socket is open and the queue is not empty - work...
        if( GetStatus() == CONNECTED && !cmdQueue.empty() ) {
            mutex.Lock();
            pair<string,string> cmd = cmdQueue.front();
            cmdQueue.pop();
            mutex.Unlock();
            
            string req = cmd.first;
            string exp = cmd.second;
            
            try {
                // 
                socket->SendLine(req);
                
                // If we got the expected response remove command from queue.
                string res = socket->ReadLine("\r"); 
                   
//                 if( exp != res ){
//                     logger.error << "[RelayBox] error: command '" << req << "', returned '" <<  res << "' but we expcted '" << exp << "'" << logger.end;
//                 }
                
            } catch (...) {
                logger.error << "[RelayBox] socket error." << logger.end;
            }
        }
    }
}

void RelayBox::SetRelayState(unsigned int relayNo, bool state) {
    stringstream cmd;
    cmd << "setio," << relayNo << "," << (int)state << "\r";

    stringstream expect;
    expect << "statechange," << relayNo << "," << (int)state << "\r";

    SendCmd(cmd.str(), expect.str());
}


void RelayBox::ToggleRelay(unsigned int relayNo) {

}

void RelayBox::DeactivateAll() {
}

void RelayBox::ActivateAll() {
}


} // NS dva


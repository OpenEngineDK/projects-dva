// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_PROJECTOR_H_
#define _OE_PROJECTOR_H_

#include <Core/Thread.h>
#include <Core/Mutex.h>
#include <Network/TCPTypes.h>
#include <queue>
#include <string>

namespace OpenEngine {
    namespace Network {
        class TCPSocket;
    }
}

namespace dva {

using OpenEngine::Core::Thread;
using OpenEngine::Core::Mutex;
using OpenEngine::Network::TCPSocket;

/**
 * 
 * 
 *
 * @class Projector Projector.h ts/dva/Projector.h
 */
class Projector : public Thread {
private:
    NetStat status;
    TCPSocket* socket;
    std::string deviceIp;
    unsigned short devicePort;
    bool done;

    Mutex mutex;
    std::queue<std::string> cmdQueue;

    void SendCmd(std::string cmd);

    void Run();

public:
    Projector();
    Projector(std::string ip, unsigned short port); 
    ~Projector();

    bool Connect(std::string ip, unsigned short port);
    void Close();
    NetStat GetStatus();

    void PowerOn();
    void PowerOff();
};

} // NS dva


#endif // _OE_RELAY_BOX_H_

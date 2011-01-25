// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_RELAY_BOX_H_
#define _OE_RELAY_BOX_H_

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
 * This class represents the relay box controlling the
 * water splash, the air and lighting.
 *
 * @class RelayBox RelayBox.h ts/dva/RelayBox.h
 */
class RelayBox : public Thread {
private:
    NetStat status;
    TCPSocket* socket;
    std::string deviceIp;
    unsigned short devicePort;
    bool done;

    Mutex mutex;
    std::queue< std::pair<std::string,std::string> > cmdQueue;

    void SendCmd(std::string, std::string);

    void Run();

public:
    RelayBox();
    RelayBox(std::string ip, unsigned short port);
    ~RelayBox();

    bool Connect(std::string ip, unsigned short port);
    void Close();
    NetStat GetStatus();

    void SetRelayState(unsigned int relayNo, bool state);
    void ToggleRelay(unsigned int relayNo);

    void DeactivateAll();
    void ActivateAll();
};

} // NS dva


#endif // _OE_RELAY_BOX_H_

// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#include <Devices/IKeyboard.h>
#include <Core/IListener.h>
#include <Utils/SimpleSetup.h>
#include <Display/Camera.h>

namespace dva {

    using namespace OpenEngine::Devices;

class CameraSwitcher : public OpenEngine::Core::IListener<KeyboardEventArg> {
    OpenEngine::Utils::SimpleSetup* setup;
    vector<OpenEngine::Display::Camera*> cams;
    unsigned int idx;
    
public:
    CameraSwitcher(OpenEngine::Utils::SimpleSetup* setup)
        : setup(setup), idx(0) {
        cams.push_back(setup->GetCamera());
    }

    void AddCamera(OpenEngine::Display::Camera* c) {
        cams.push_back(c);
    }

    void Handle(KeyboardEventArg arg) {
        if (arg.type != EVENT_RELEASE) return;
        if (arg.sym == KEY_p && cams.size() > (idx+1)) {
            ++idx;
            setup->SetCamera(*cams[idx]);
        } else if (arg.sym == KEY_o && cams.size() > (idx-1)) {
            --idx;
            setup->SetCamera(*cams[idx]);
        }
    }
};


} // NS dva

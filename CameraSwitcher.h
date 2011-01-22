// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


namespace dva {

class CameraSwitcher : public IListener<KeyboardEventArg> {
    SimpleSetup* setup;
    vector<Camera*> cams;
    unsigned int idx;
    
public:
    CameraSwitcher(SimpleSetup* setup)
        : setup(setup), idx(0) {
        cams.push_back(setup->GetCamera());
    }

    void AddCamera(Camera* c) {
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

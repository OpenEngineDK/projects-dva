// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------


#ifndef _OE_I_RULE_HANDLER_H_
#define _OE_I_RULE_HANDLER_H_

#include <vector>

namespace OpenEngine {
    namespace Animations {
        class Flock;
    }
}

namespace dva {

using OpenEngine::Animations::Flock;

class IRuleHandler {
protected:
    IRule* rule;
    Flock* flock;
    bool loaded;

    void Load(){
        if( !loaded ){
            flock->AddRule(rule);
            loaded = true;
            logger.info << "[" << this << "] loaded" << logger.end;
        }
    }
    
    void Unload(){
        if( loaded ){
            flock->RemoveRule(rule);
            logger.info << "[" << this << "] unloaded" << logger.end;
        }
    }

public:
    IRuleHandler(IRule* rule, Flock* flock) 
        : rule(rule)
        , flock(flock)
        , loaded(false) {
        Load();
    }
    
    virtual ~IRuleHandler() {
        Unload();
        if( rule ){
            delete rule;
            rule = NULL;
        }
    }

    IRule* GetRule() {
        return rule;
    }

    virtual void Handle(Core::ProcessEventArg arg) {}
    virtual void HandleInput(std::vector< Vector<2,int> >& screenPoints) = 0;
};

} // NS dva

#endif // _OE_I_RULE_HANDLER_H_

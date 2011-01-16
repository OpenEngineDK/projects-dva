// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _USER_DEFAULTS_H_
#define _USER_DEFAULTS_H_

#include <map>

namespace OpenEngine {
namespace Utils {

class UserDefaults;
static UserDefaults* instance = NULL;

class UserDefaults {
public:
    std::map<std::string, void*> map;

    static UserDefaults* GetInstance(){
        if( instance == NULL )
            instance = new UserDefaults();
        return instance;
    }
};



} // NS Utils
} // NS OpenEngine


#endif

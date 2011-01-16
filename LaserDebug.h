// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _LASER_DEBUG_H_
#define _LASER_DEBUG_H_

#include <Resources/Texture2D.h>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>


namespace dva {

    using namespace OpenEngine;

class LaserDebug;
typedef boost::shared_ptr<LaserDebug> LaserDebugPtr;
class LaserDebug : public virtual Resources::Texture2D<unsigned char> {
private:
    boost::weak_ptr<LaserDebug> weak_this;
    LaserDebug(unsigned int height, unsigned int width)
        : Resources::Texture2D<unsigned char>(height,width,3) {}
public:
    static LaserDebugPtr Create(unsigned int width, unsigned int height) {
        LaserDebugPtr ptr(new LaserDebug(width, height));
        ptr->weak_this = ptr;
        return ptr;
    }
    virtual ~LaserDebug() {}

    void UpdateTexture() {
        ChangedEvent().Notify(Resources::Texture2DChangedEventArg(Resources::ITexture2DPtr(weak_this)));
    }
};


} // NS dva

#endif

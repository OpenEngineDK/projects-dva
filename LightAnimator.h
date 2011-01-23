#ifndef __LIGHT_ANIMATOR__
#define __LIGHT_ANIMATOR__

#include <Geometry/Light.h>
#include <Core/EngineEvents.h>
#include <Core/IListener.h>
#include <Math/Math.h>
#include <Logging/Logger.h>

class LightAnimator : public OpenEngine::Core::IListener<OpenEngine::Core::ProcessEventArg> {
 protected:
    OpenEngine::Geometry::Light* light;
    float t;
 public:
 LightAnimator(OpenEngine::Geometry::Light* light) : light(light) {
        t = 0.0f;
    }
    void Handle(OpenEngine::Core::ProcessEventArg arg) {
        t += (arg.approx * 1e-6);
        float value = ( (sin( (t/40.0f) * 2*OpenEngine::Math::PI) + 1.0f)/2.0f) * 0.1;
        light->ambient = Vector<4,float>(0.4,0.45+value,0.4,1.0);
    }
};

#endif // __LIGHT_ANIMATOR__

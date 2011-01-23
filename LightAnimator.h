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
    bool disco;
 public:
 LightAnimator(OpenEngine::Geometry::Light* light) : light(light) {
        t = 0.0f;
        disco = true;
    }
    void Handle(OpenEngine::Core::ProcessEventArg arg) {
        t += (arg.approx * 1e-6);
        if (!disco) {
            float timeFactor = 40.0f;
            float value = (sin( (t/timeFactor) * 2*OpenEngine::Math::PI) + 1.0f)/2.0f;
            light->ambient = Vector<4,float>(0.4,0.45+(value*0.1),0.4,1.0);
        } else {
            float value1 = (sin( (t/3.0f) * 2*OpenEngine::Math::PI) + 1.0f)/2.0f;
            float value2 = (sin( (t/5.0f) * 2*OpenEngine::Math::PI) + 1.0f)/2.0f;
            float value3 = (sin( (t/7.0f) * 2*OpenEngine::Math::PI) + 1.0f)/2.0f;
            light->ambient = Vector<4,float>(0.7+value1*0.3,0.5+value2*0.5,0.3+value3*0.7,1.0);
        }
    }
};

#endif // __LIGHT_ANIMATOR__

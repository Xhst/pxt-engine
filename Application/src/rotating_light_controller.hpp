#include "pxtengine.h"

using namespace PXTEngine;

class RotatingLightController : public Script {
public:
    void onCreate() override;
    void onUpdate(float deltaTime) override;

private:
    float m_baseAngle = 0.0f;
    float m_angle = 0.0f;
};
#ifndef _POINT_LIGHT_
#define _POINT_LIGHT_

struct PointLight {
    vec4 position;  // .xyz = world position, .w = unused
    vec4 color;     // .xyz = RGB color, .w = intensity
};

#endif
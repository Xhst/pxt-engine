#ifndef __POINT_LIGHT__
#define __POINT_LIGHT__

struct PointLight {
    vec4 position;  // .xyz = world position, .w = unused
    vec4 color;     // .xyz = RGB color, .w = intensity
};

#endif
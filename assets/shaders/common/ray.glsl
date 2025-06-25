#ifndef _RAY_
#define _RAY_

// Minimum intersection distance
#define RAY_T_MIN 0.001

// Maximum intersection distance
#define RAY_T_MAX 10000.0

struct Ray {
	vec3 origin;
	vec3 direction;
};

#endif
#ifndef _BRDF_CONFIG_
#define _BRDF_CONFIG_

/**
 * @brief Defines to select specific BRDF models and components.
 *
 * These defines are used throughout the shader code to choose which
 * BRDF implementation functions are compiled and used, often based on
 * rendering quality settings or material properties.
 */

 // Diffuse BRDFs
#define DIFFUSE_LAMBERT             0
#define DIFFUSE_BURLEY              1

// Specular BRDF
// Normal distribution functions
#define SPECULAR_D_GGX              0

// Anisotropic NDFs
#define SPECULAR_D_GGX_ANISOTROPIC  0

// Cloth NDFs
#define SPECULAR_D_CHARLIE          0

// Visibility functions
#define SPECULAR_V_SMITH_GGX        0
#define SPECULAR_V_SMITH_GGX_FAST   1
#define SPECULAR_V_GGX_ANISOTROPIC  2
#define SPECULAR_V_KELEMEN          3
#define SPECULAR_V_NEUBELT          4

// Fresnel functions
#define SPECULAR_F_SCHLICK          0

#define BRDF_DIFFUSE                DIFFUSE_LAMBERT

// QUALITY and QUALITY_HIGH are defined in another file
#if QUALITY < QUALITY_HIGH
	#define BRDF_SPECULAR_D         SPECULAR_D_GGX
	#define BRDF_SPECULAR_V         SPECULAR_V_SMITH_GGX_FAST
	#define BRDF_SPECULAR_F         SPECULAR_F_SCHLICK
#else
	#define BRDF_SPECULAR_D         SPECULAR_D_GGX
	#define BRDF_SPECULAR_V         SPECULAR_V_SMITH_GGX
	#define BRDF_SPECULAR_F         SPECULAR_F_SCHLICK
#endif


#endif 
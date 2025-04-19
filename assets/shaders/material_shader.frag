#version 450

layout(constant_id = 0) const int MAX_LIGHTS = 10;

#define EPSILON 0.65
#define SHADOW_OPACITY 0.5

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

struct PointLight {
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  mat4 inverseViewMatrix;
  vec4 ambientLightColor;
  PointLight pointLights[MAX_LIGHTS];
  int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D textures[3];

layout(set = 2, binding = 0) uniform samplerCube shadowCubeMap;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
  vec4 color;
  float specularIntensity;
  float shininess;
  int textureIndex;
  int normalMapIndex;
} push;

void main() {
  
  vec3 surfaceNormal = normalize(fragNormalWorld);

  // normal map value is in [0, 1] range
  vec3 normalMapValue = 2.0 * texture(textures[push.normalMapIndex], fragUV).rgb - 1.0;
  if (push.normalMapIndex != 0) {
    surfaceNormal = normalize(fragTBN * normalMapValue);
  }

  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specularLight = vec3(0.0);
 
  vec3 cameraPosWorld = ubo.inverseViewMatrix[3].xyz;
  vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld); // it's the direction from fragment to camera

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 vectorToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(vectorToLight, vectorToLight);
    vec3 directionToLight = normalize(vectorToLight);

    float cosAngleIncidence = max(dot(surfaceNormal, directionToLight), 0);
    vec3 lightColor = light.color.xyz * light.color.w * attenuation; // color * intensity * attenuation
    
    diffuseLight += lightColor * cosAngleIncidence;

    // specualar light - using Blinn-Phong model
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = dot(surfaceNormal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, push.shininess); // Higher values -> sharper highlight

    specularLight += lightColor * blinnTerm * push.specularIntensity; 
  }

  vec3 imageColor = texture(textures[push.textureIndex], fragUV).rgb;

  /* we need to add control coefficients to regulate both terms
     for now we use fragColor for both which is ideal for metallic objects
  */
  outColor = vec4((diffuseLight * fragColor + specularLight * fragColor) * imageColor, 1.0);

  // Shadow
  vec3 lightVec = fragPosWorld - ubo.pointLights[0].position.xyz;
  float sampledDist = texture(shadowCubeMap, lightVec).r;
  float dist = length(lightVec);
  
  // Check if fragment is in shadow
  float shadow = (dist <= sampledDist + EPSILON) ? 1.0 : SHADOW_OPACITY;

  outColor.rgb *= shadow;
}
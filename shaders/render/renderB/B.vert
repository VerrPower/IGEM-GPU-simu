#version 460 core

#include "DeviceParams.comp"

layout(location = 0) in vec3 aPos;
out float w;

uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * cauris_models[gl_InstanceID] * vec4(aPos, 1.0);
    w = cauris[gl_InstanceID].health;
}




#version 460 core

#include "DeviceParams.comp"

layout(location = 0) in vec3 aPos;
out float w;
flat out int state;

uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * ecoli_models[gl_InstanceID] * vec4(aPos, 1.0);
    Capsule P = capsules[gl_InstanceID];
    w = float(P.cooldown) / 120.0f ;
    state = P.motion_state;
}




// Fragment Shader
#version 460 core
#extension GL_EXT_shader_integer_mix : require

in float w;
out vec4 fragColor;

const vec4 orange = vec4(1.0f, 0.637f, 0.113f, 0.0f);
const vec4 red = vec4(1.0f, 0.176f, 0.058f, 0.0f);
const vec4 black = vec4(0.1f, 0.1f, 0.1f, 0.0f);

void main() {
    float bit = float(int(w <= 0.0f));
    fragColor = mix(mix(red, orange, w), black, bit);
}




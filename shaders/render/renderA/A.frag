// Fragment Shader
#version 460 core

flat in int state;
in float w;
out vec4 fragColor;

const vec4 color_table[] = {
    vec4(0.9, 0.9, 0.9, 1.0), // white
    vec4(0.53, 0.81, 0.92, 1.0), // sky blue - neutral 0
    vec4(1.0, 0.1, 0.1, 1.0), // red - run 3
    vec4(0.0, 1.0, 0.0, 1.0),  // green - rotate 6
    vec4(0,0,0,1)
};

float f(float x){
    if(x<0.001) return 0.0f;
    return exp(x-1);
}

int get_color(int st){
    switch(st){
        case 0: return 0;
        case 1: return 4;
        case 3: return 2;
        case 6: return 3;
        default: return 1;
    }
}

void main() {
    //fragColor = color_table[get_color(state)];
    fragColor = color_table[1];
}




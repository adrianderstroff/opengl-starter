#version 330 core

layout(location = 0) out vec3 outColor;

in vec3 oColor;

void main() {
    outColor = oColor;
} 
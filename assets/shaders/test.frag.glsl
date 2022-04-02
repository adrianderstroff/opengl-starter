
#version 330 core

layout(location = 0) out vec4 outColor;

in vec2 texCoord;
uniform sampler2D tex;

void main() {
    outColor = texture(tex, texCoord);
} 
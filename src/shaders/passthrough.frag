#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSamplers[2];

void main() {
    //outColor = texture(texSamplers[0], fragTexCoord);
    // blend for testing :D
    outColor = 0.5 * texture(texSamplers[0], fragTexCoord) + 0.5 * texture(texSamplers[1], fragTexCoord);

}

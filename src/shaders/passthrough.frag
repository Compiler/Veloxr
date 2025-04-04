#version 450

layout(location = 0) in vec4 fragTexCoord;
layout(location = 1) in flat int texUnit;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSamplers[2];

void main() {
    outColor = texture(texSamplers[texUnit], fragTexCoord.xy);
    // blend for testing :D
    //outColor = 0.5 * texture(texSamplers[0], fragTexCoord.xy) + 0.5 * texture(texSamplers[1], fragTexCoord.xy);

}

#version 450

layout(location = 0) in vec4 fragTexCoord;
layout(location = 1) in flat int texUnit;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord.xy);
    return;
}

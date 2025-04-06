#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inTexCoord;
layout(location = 2) in int inTextureUnit;

layout(location = 0) out vec4 fragTexCoord;
layout(location = 1) out flat int texUnit;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition.xy, 0.0, 1.0);
    //gl_Position = vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
    texUnit = inTextureUnit;
}

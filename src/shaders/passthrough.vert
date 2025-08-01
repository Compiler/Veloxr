#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inTexCoord;
layout(location = 2) in int inTextureUnit;
layout(location = 3) in int inRenderID;

layout(location = 0) out vec4 fragTexCoord;
layout(location = 1) out flat int texUnit;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 roi;
    uint hiddenMask;
    float nSplitVal;
} ubo;

out gl_PerVertex {
    vec4  gl_Position;
    float gl_ClipDistance[4];
};

void main() {

    vec4 world = vec4(inPosition.x, inPosition.y, 0.0, 1.0);

    if(any(notEqual(ubo.roi, vec4(0.0)))) {
        gl_ClipDistance[0] =  world.x - ubo.roi.x;
        gl_ClipDistance[1] =  ubo.roi.z - world.x;
        gl_ClipDistance[2] =  world.y - ubo.roi.y;
        gl_ClipDistance[3] =  ubo.roi.w - world.y;
    } else {
        gl_ClipDistance[0] = gl_ClipDistance[1] = gl_ClipDistance[2] = gl_ClipDistance[3] = 1.0;
    }

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition.xy, 0.0, 1.0);

    int renderIdInt = int(round(inRenderID));
    bool isHidden = (ubo.hiddenMask & (1u << renderIdInt)) != 0u;
    // hack
    if(isHidden) {
        gl_Position = vec4(0);
        gl_ClipDistance[0] = -1;
    }

    fragTexCoord = inTexCoord;
    texUnit = inTextureUnit;
}


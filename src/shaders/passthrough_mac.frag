#version 450

layout(location = 0) in vec4 fragTexCoord;
layout(location = 1) in flat int texUnit;

layout(location = 0) out vec4 outColor;

// Use 16 samplers for macOS compatibility (M3 Pro hardware limit)
layout(binding = 1) uniform sampler2D texSamplers[16];

void main() {
    outColor = texture(texSamplers[texUnit], fragTexCoord.xy);
    
    // Debug code for edge detection
    float edgeThreshold = 0.005;

    bool nearLeft   = (fragTexCoord.x <= edgeThreshold);
    bool nearRight  = (fragTexCoord.x >= 1.0 - edgeThreshold);
    bool nearBottom = (fragTexCoord.y <= edgeThreshold);
    bool nearTop    = (fragTexCoord.y >= 1.0 - edgeThreshold);

    if (nearLeft || nearRight || nearBottom || nearTop) {
        outColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
} 
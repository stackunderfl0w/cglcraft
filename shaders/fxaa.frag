#version 330 core

in vec2 vTexCoord;

uniform sampler2D screenTexture;
uniform vec2 inverseScreenSize;

out vec4 FragColor;

// Calculate the luminance of a color
float Luma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 texCoord = vTexCoord;

    // Offsets for neighboring texels
    vec2 offsets[4] = vec2[](
    vec2(-inverseScreenSize.x,  0.0), // Left
    vec2( inverseScreenSize.x,  0.0), // Right
    vec2( 0.0, -inverseScreenSize.y), // Down
    vec2( 0.0,  inverseScreenSize.y)  // Up
    );

    // Sample neighboring pixels
    vec3 colorM = texture(screenTexture, texCoord).rgb;                   // Middle
    vec3 colorL = texture(screenTexture, texCoord + offsets[0]).rgb;      // Left
    vec3 colorR = texture(screenTexture, texCoord + offsets[1]).rgb;      // Right
    vec3 colorD = texture(screenTexture, texCoord + offsets[2]).rgb;      // Down
    vec3 colorU = texture(screenTexture, texCoord + offsets[3]).rgb;      // Up

    // Calculate luminance
    float lumaM = Luma(colorM);
    float lumaL = Luma(colorL);
    float lumaR = Luma(colorR);
    float lumaD = Luma(colorD);
    float lumaU = Luma(colorU);

    // Find the minimum and maximum luminance
    float lumaMin = min(lumaM, min(min(lumaL, lumaR), min(lumaU, lumaD)));
    float lumaMax = max(lumaM, max(max(lumaL, lumaR), max(lumaU, lumaD)));

    // Luminance range
    float lumaRange = lumaMax - lumaMin;

    // Threshold to decide if anti-aliasing is needed
    if (lumaRange < 0.1)
    {
        // No significant contrast, output original color
        FragColor = vec4(colorM, 1.0);
    }
    else
    {
        // Edge detected, apply FXAA
        float gradientH = abs(lumaL - lumaR);
        float gradientV = abs(lumaU - lumaD);

        bool isHorizontal = (gradientH >= gradientV);

        vec2 edgeOffset = isHorizontal ? vec2(0.0, inverseScreenSize.y) : vec2(inverseScreenSize.x, 0.0);

        // Sample along the edge
        vec3 colorA = 0.5 * (
        texture(screenTexture, texCoord + edgeOffset).rgb +
        texture(screenTexture, texCoord - edgeOffset).rgb
        );

        // Blend factor based on luminance difference
        float lumaA = Luma(colorA);
        float blend = clamp(abs(lumaA - lumaM) / lumaRange, 0.0, 1.0);

        // Output the blended color
        FragColor = vec4(mix(colorM, colorA, blend), 1.0);
    }
}

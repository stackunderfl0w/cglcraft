#version 330 core

//worldspace
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightDir;       // Dir light dir (normalized, world space)
uniform vec3 viewPos;        // Camera position world space
uniform vec3 lightColor;
uniform sampler2D texture1;

void main()
{
    // Normalize the interpolated normal
    vec3 norm = normalize(Normal);

    // Ambient lighting
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 lightDirNorm = normalize(-lightDir); // Invert light direction to point *TOWARDS* the surface
    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.1;
    vec3 viewDir = normalize(viewPos - FragPos); // Direction to the camera
    vec3 reflectDir = reflect(-lightDirNorm, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 lighting = ambient + diffuse + specular;

    vec4 texColor = texture(texture1, TexCoord);

    vec3 result = lighting * texColor.rgb;

    FragColor = vec4(result, texColor.a);
}

#version 460 core
layout (location = 0) in vec3 attribute_Position;
layout (location = 1) in vec2 attribute_TexCoords;
layout (location = 2) in vec3 attribute_Normal;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 uP_m = mat4(1.0);
uniform mat4 uM_m = mat4(1.0);
uniform mat4 uV_m = mat4(1.0);

void main()
{
    mat4 modelMatrix = uM_m;
    mat4 viewMatrix = uV_m;
    mat4 projectionMatrix = uP_m;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(attribute_Position, 1.0);
    TexCoord = attribute_TexCoords;
    FragPos = vec3(modelMatrix * vec4(attribute_Position, 1.0));
    Normal = mat3(transpose(inverse(modelMatrix))) * (attribute_Normal);
}
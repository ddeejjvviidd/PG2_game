#version 460 core
in vec3 aPos;
in vec3 aNorm;
in vec2 aTex;

uniform mat4 uP_m = mat4(1.0f);
uniform mat4 uM_m = mat4(1.0f);
uniform mat4 uV_m = mat4(1.0f);

out VS_OUT {
    vec2 texcoord;
} vs_out;

void main() {
    // Outputs the positions/coordinates of all vertices
    gl_Position = uP_m * uV_m * uM_m * vec4(aPos, 1.0f);

    vs_out.texcoord = aTex;
}

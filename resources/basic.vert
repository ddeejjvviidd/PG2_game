// #version 460 core
// in vec3 attribute_Position;
// uniform mat4 model; // Add this
// void main() {
//     gl_Position = model * vec4(attribute_Position, 1.0); // Apply model transformation
// }

#version 460 core
in vec3 attribute_Position;

uniform mat4 uP_m = mat4(1.0);
uniform mat4 uM_m = mat4(1.0);
uniform mat4 uV_m = mat4(1.0);

void main()
{
    // Outputs the positions/coordinates of all vertices
    gl_Position = uP_m * uV_m * uM_m * vec4(attribute_Position, 1.0f);
}

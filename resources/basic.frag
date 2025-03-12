// #version 460 core

// in vec3 color; // input from vertex stage of graphics pipeline, automatically interpolated
// out vec4 FragColor; // output color of current fragment: MUST be written

// void main()
// {
//     FragColor = vec4(color, 1.0f); // copy RGB color, add Alpha=1.0 (not transparent)
// }

#version 460 core
uniform vec4 uniform_Color;
out vec4 FragColor;
void main() {
    FragColor = uniform_Color;
}
// #version 460 core

// // input vertex attributes

// in vec3 aPos;   // position: MUST exist
// in vec3 aColor; // any additional attributes are optional, any data type, etc.

// out vec3 color; // optional output attribute

// void main()
// {
//     // Outputs the positions/coordinates of all vertices, MUST WRITE
//     gl_Position = vec4(aPos, 1.0f);
    
//     color = aColor; // copy color to output
// }

#version 460 core
in vec3 attribute_Position;
void main() {
    gl_Position = vec4(attribute_Position, 1.0);
}
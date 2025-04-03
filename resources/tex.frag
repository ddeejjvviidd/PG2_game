#version 460 core

// (interpolated) input from previous pipeline stage
in VS_OUT {
    vec2 texcoord;
} fs_in;

// uniform variables
uniform sampler2D tex0; // texture unit from C++
uniform vec4 u_diffuse_color = vec4(1.0f);

// mandatory: final output color
out vec4 FragColor; 

void main() {
    // modulate texture with material color, including transparency
    FragColor = u_diffuse_color * texture(tex0, fs_in.texcoord);
}

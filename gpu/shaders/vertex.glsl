#version 330 core
in ivec2 vertex_position;
in ivec3 vertex_color;

out vec3 color;

// drawing offset
uniform ivec2 uniform_offset; 

void main() 
{
    // apply offset
    ivec2 position = vertex_position + uniform_offset;

    // Convert VRAM coords into OpenGL coords (e.g. 0;1023, 0;511 -> -1;1, -1;1)
    float xpos = (float(position.x) / 512) - 1.0;
    // VRAM puts 0 to top, OpenGL to bottom, so flip
    float ypos = 1.0 - (float(position.y) / 256);

    gl_Position.xyzw = vec4(xpos, ypos, 0.0, 1.0);

    // Convert from 0;255 to 0;1
    color = vec3(float(vertex_color.r) / 255,
                 float(vertex_color.g) / 255,
                 float(vertex_color.b) / 255);
}

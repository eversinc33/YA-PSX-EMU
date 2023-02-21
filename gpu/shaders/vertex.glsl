#version 330

in ivec2 vertex_position;
in ivec3 vertex_color;

out vec3 color;

void main() 
{
    // Convert VRAM coords into OpenGL coords (e.g. 0;1023, 0;511 -> -1;1, -1;1)
    float xpos = (float(vertex_position.x) / 512.0) - 1.0;
    // VRAM puts 0 to top, OpenGL to bottom, so flip
    float ypos = 1.0 - (float(vertex_position.y) / 256.0);

    gl_Position = vec4(xpos, ypos, 0.0, 1.0);

    // Convert from 0;255 to 0;1
    color = vec3(float(vertex_color.r) / 255.0,
                 float(vertex_color.g) / 255.0,
                 float(vertex_color.b) / 255.0);
}

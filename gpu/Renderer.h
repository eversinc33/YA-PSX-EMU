#ifndef RENDERER_H
#define RENDERER_H

#pragma once

#define GL_SILENCE_DEPRECATION
#include <SDL2/SDL.h> 

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "../util/logging.h"
#include "../util/filesystem.h"
#include <cstring>

#ifndef GL_MAP_WRITE_BIT
#define GL_MAP_WRITE_BIT 0x0002
#endif
#ifndef GL_MAP_PERSISTENT_BIT
#define GL_MAP_PERSISTENT_BIT 0x0040
#endif
#ifndef GL_MAP_COHERENT_BIT
#define GL_MAP_COHERENT_BIT 0x0080
#endif

struct Position {
    GLshort x;
    GLshort y;
    Position(GLshort x, GLshort y)
    {
        this-> x = x;
        this-> y = y;
    };
};

struct Color {
    GLubyte r;
    GLubyte g;
    GLubyte b;
    Color(GLubyte r, GLubyte g, GLubyte b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
    };
};

// Parse a position from a gp0 param
inline Position pos_from_gp0(const uint32_t& value)
{
    GLshort x = (int16_t)value;
    GLshort y = (int16_t)(value >> 16);

    return Position(x, y);
}

inline Color color_from_gp0(const uint32_t& value)
{
    GLubyte r = (uint8_t)value;
    GLubyte g = (uint8_t)(value >> 8);
    GLubyte b = (uint8_t)(value >> 16);

    return Color(r, g, b);
}

const uint32_t VERTEX_BUFFER_LEN = 64 * 1024; // max n of vertices that can be stored in a buffer

template <class T>
class Buffer {
public:
    GLuint object = 0; // buffer object
    T* map; // mapped buffer memory

    Buffer() {
    };
    
    void init_buffer() {
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        GLsizeiptr element_size = (GLsizeiptr)(sizeof(T));
        GLsizeiptr buffer_size  = (GLsizeiptr)(element_size * VERTEX_BUFFER_LEN);
        GLbitfield access = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
#ifdef __APPLE__
        // TODO FIXME this call is wrong
        glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, access);
        DEBUG("MACOSX:Unsupported:glBufferStorage_not_implemented!s")
        throw std::exception();
#else 
        glBufferStorage(GL_ARRAY_BUFFER, buffer_size, NULL, access);
#endif
        this->map = (T*)glMapBufferRange(GL_ARRAY_BUFFER, 0, buffer_size, access);
        memset(this->map, 0, buffer_size);
        if (this->map == NULL)
        {
            DEBUG("ERROR:gl_buffer_range_mapped_is_null");
            throw std::exception();
        }
        DEBUG("MAP:" << this->map)
    }

    ~Buffer() {
        glBindBuffer(GL_ARRAY_BUFFER, this->object);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glDeleteBuffers(1, &this->object);
    }

    void set(const uint32_t& index, const T& value)
    {
        if (index >= VERTEX_BUFFER_LEN)
        {
            DEBUG("BUFFER_OVERLOAD");
            throw std::exception();
        }
        
        this->map[index] = value; // TODO CHECK does this work like this?
    }
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void push_triangle(Position positions[3], Color colors[3]);
    void push_quad(Position positions[4], Color colors[4]);
    void display();
    void set_drawing_offset(const int16_t& x, const int16_t& y);
private:
    SDL_Window* window;
    SDL_Surface* screen_surface;
    SDL_GLContext gl_context;

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program; // openGL program object
    GLuint vertex_array_object; // openGL vertex array object
    Buffer<Position> positions; // buffer with positions
    Buffer<Color> colors; // buffer with colors
    uint32_t nvertices = 0; // current n of vertices in the buffers
    GLint uniform_offset; // offset for drawing vertices
    
    bool init_sdl();
    void check_for_errors();
    void draw();
};

#endif

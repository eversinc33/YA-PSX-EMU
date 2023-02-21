#ifndef RENDERER_H
#define RENDERER_H

#pragma once

#define GL_SILENCE_DEPRECATION
#include <SDL2/SDL.h> 
#include <OpenGL/gl3.h>
#include "../util/logging.h"
#include "../util/filesystem.h"

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
    
    void create() {
        glGenBuffers(1, &object);
        glBindBuffer(GL_ARRAY_BUFFER, object);
        GLsizeiptr element_size = (GLsizeiptr)(sizeof(T));
        GLsizeiptr buffer_size  = (GLsizeiptr)(element_size * VERTEX_BUFFER_LEN);
        GLbitfield access = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
#ifdef __APPLE__
        glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, access);
#else 
        glBufferStorage(GL_ARRAY_BUFFER, buffer_size, NULL, access);
#endif
        auto memory = glMapBufferRange(GL_ARRAY_BUFFER, 0, buffer_size, access);
        // TODO: reset buffer to zero memory
        this->map = (T*)memory;
    }

    ~Buffer() {
        glBindBuffer(GL_ARRAY_BUFFER, this->object);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glDeleteBuffers(1, &this->object);
    }

    void set(const uint32_t &index, const T& value)
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

    void poll();
    bool init_sdl();

    void push_triangle(Position positions[3], Color colors[3]);
    void display();
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
    
    bool should_quit = false; 

    void draw();
};

#endif

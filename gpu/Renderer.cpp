#include "Renderer.h"
#include "../util/logging.h"
#include "../util/filesystem.h"
#include "Constants.h"
#include <exception>
#include <vector>

GLuint compile_shader(const std::string& data, const GLenum& shader_type)
{
    GLuint shader = glCreateShader(shader_type);

    // compile
    const char *c_str = data.c_str();
    glShaderSource(shader, 1, &c_str, NULL);
    glCompileShader(shader);

    // check for errors
    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        DEBUG("ERROR:GL_Shader_compilation_failed:" << status);
        // get reason
        GLint max_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);
        std::vector<GLchar> error_log(max_length);
        glGetShaderInfoLog(shader, max_length, &max_length, &error_log[0]);
        for (const auto& c : error_log) 
        {
            std::cout << c;
        }
        throw std::exception();
    }

    DEBUG("Compiled_shader");

    return shader;
}

GLuint link_program(const std::pair<GLuint, GLuint> &shaders)
{
    auto program = glCreateProgram();

    glAttachShader(program, shaders.first);
    glAttachShader(program, shaders.second);

    glLinkProgram(program);

    auto status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        DEBUG("ERROR:GL_Program_linking_failed");
        throw std::exception();
    }

    return program;
}

// get index for attribute in program
GLuint find_program_attrib(const GLuint& program, const std::string& attr)
{
    auto cstr = attr.c_str();
    auto index = glGetAttribLocation(program, cstr);

    if (index < 0) 
    {
        DEBUG("ERROR:GL_attrib_'" << attr << "'_not_found_in_program");
        throw std::exception();
    }

    return (GLuint)index;
}

// get index for uniform in program
GLuint find_program_uniform(const GLuint& program, const std::string& attr)
{
    auto cstr = attr.c_str();
    auto index = glGetUniformLocation(program, cstr);

    if (index < 0) 
    {
        DEBUG("ERROR:GL_uniform_'" << attr << "'_not_found_in_program");
        throw std::exception();
    }

    return (GLuint)index;
}

Renderer::Renderer()
{
    if (!this->init_sdl()) 
    {
        throw std::exception();
    }

    // TODO: fix hardcoded paths
    auto vertex_shader_path = "../gpu/shaders/vertex.glsl";
    auto fragment_shader_path = "../gpu/shaders/fragment.glsl";
    if (!file_exists(vertex_shader_path))
    {
        DEBUG("Shader not found. Expected path:" << vertex_shader_path);
        throw std::exception();
    }
    if (!file_exists(fragment_shader_path))
    {
        DEBUG("Shader not found. Expected path:" << vertex_shader_path);
        throw std::exception();
    }

    auto vs_src = read_file_to_string("../gpu/shaders/vertex.glsl");
    auto fg_src = read_file_to_string("../gpu/shaders/fragment.glsl");
    this->vertex_shader   = compile_shader(vs_src, GL_VERTEX_SHADER);
    this->fragment_shader = compile_shader(fg_src, GL_FRAGMENT_SHADER);

    this->program = link_program({this->vertex_shader, this->fragment_shader});
    glUseProgram(this->program);

    // Vertex attribute object
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Set up Positions buffer
    this->positions = Buffer<Position>();
    this->positions.init_buffer();
    auto index = find_program_attrib(program, "vertex_position");
    glEnableVertexAttribArray(index);
    // 2 GLShort attributes, not normalized
    glVertexAttribIPointer(index, 2, GL_SHORT, 0, nullptr);

    // set up offset handling
    this->uniform_offset = find_program_uniform(program, "uniform_offset");
    glUniform2i(this->uniform_offset, 0, 0); // initially set to 0,0

    // Set up Color buffer
    this->colors = Buffer<Color>();
    this->colors.init_buffer();
    index = find_program_attrib(program, "vertex_color");
    glEnableVertexAttribArray(index);
    // 3 GLubyte attributes, not normalized
    glVertexAttribIPointer(index, 3, GL_UNSIGNED_BYTE, 0, nullptr);

    // Clear screen
    glClearColor(0.0, 0.0, 0.0, 1.0);    
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(this->window);
    
    // verify all went well
    this->check_for_errors();
}

bool Renderer::init_sdl()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    // TODO: set DEBUG_FLAG only when run as debug
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

#ifdef __APPLE__
    this->window = SDL_CreateWindow("PSX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH_PX, SCREEN_HEIGHT_PX, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
#else
    this->window = SDL_CreateWindow("PSX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH_PX, SCREEN_HEIGHT_PX, SDL_WINDOW_SHOWN);
#endif
    if (this->window == NULL)
    {
        DEBUG("Window could not be created! SDL_Error: " << SDL_GetError());
        return false;
    }

    this->gl_context     = SDL_GL_CreateContext(this->window);
    this->screen_surface = SDL_GetWindowSurface(this->window);

    return true;
}

void Renderer::check_for_errors()
{
    const GLsizei buffer_len  = 4096;
    GLchar buffer[buffer_len] = { 0 }; // buffer for message log
    // bool fatal                = false; // TODO: throw exception on fatal severity

    GLsizei msg_size = 0;
    GLenum severity  = 0;
    GLenum source    = 0;
    GLenum mtype     = 0;
    GLuint id        = 0;

    while (true) 
    {
        if (glGetDebugMessageLog(1, buffer_len, &source, &mtype, &id, &severity, &msg_size, buffer) == 0)
        {
            break;
        }
    }

    std::string message = std::string(&buffer[0], &buffer[msg_size]);
    DEBUG("OpenGL_Error:" << severity << "," << mtype << ":" << message);
}

void Renderer::display()
{
    this->draw();
    SDL_GL_SwapWindow(this->window);
    this->check_for_errors();
}

void Renderer::draw() 
{
    // make sure all data is lfushed to buffer
    // glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)this->nvertices);

    // wait for GPU to complete
    auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    while (true) {
        auto r = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 10000000);
        if (r == GL_ALREADY_SIGNALED || r == GL_CONDITION_SATISFIED) 
        {
            // drawing finished
            break;
        }
    }

    // reset buffers
    this->nvertices = 0;
}

void Renderer::push_triangle(Position positions[3], Color colors[3]) 
{
    // make sure we have enough room to queue the vertices
    if (this->nvertices + 3 > VERTEX_BUFFER_LEN)
    {
        DEBUG("VERTEX_BUFFERS_FULL!_FORCING_DRAW!");
        this->draw();
    }
    
    for (const auto& i : { 0,1,2 }) 
    {
        // push
        this->positions.set(this->nvertices, positions[i]);
        this->colors.set(this->nvertices, colors[i]);
        // DEBUG("Triangle-vertex:" << positions[i].x << "," << positions[i].y << " -> " << (float(positions[i].x)/512)-1.0f << "," << float(positions[i].y / 256.0));
        this->nvertices++;
    }
}

void Renderer::push_quad(Position positions[4], Color colors[4]) 
{
    // make sure we have enough room to queue the vertices
    // 2 triangles = 1 quad, so 6 vertices
    if (this->nvertices + 6 > VERTEX_BUFFER_LEN)
    {
        DEBUG("VERTEX_BUFFERS_FULL!_FORCING_DRAW!");
        this->draw();
    }
    
    // push first triangle
    for (const auto& i : { 0,1,2 }) 
    {
        this->positions.set(this->nvertices, positions[i]);
        this->colors.set(this->nvertices, colors[i]);
        // DEBUG("Triangle-vertex-for-quad:" << positions[i].x << "," << positions[i].y << " -> " << (float(positions[i].x)/512)-1.0f << "," << float(positions[i].y / 256.0));
        this->nvertices++;
    }
    // push second triangle
    for (const auto& i : { 1,2,3 }) 
    {
        this->positions.set(this->nvertices, positions[i]);
        this->colors.set(this->nvertices, colors[i]);
        // DEBUG("Triangle-vertex-for-quad:" << positions[i].x << "," << positions[i].y << " -> " << (float(positions[i].x)/512)-1.0f << "," << float(positions[i].y / 256.0));
        this->nvertices++;
    }
}

Renderer::~Renderer()
{
    SDL_DestroyWindow(this->window);
    SDL_Quit();
    glDeleteVertexArrays(1, &this->vertex_array_object);
    glDeleteShader(this->vertex_shader);
    glDeleteShader(this->fragment_shader);
    glDeleteProgram(this->program);
}

void Renderer::set_drawing_offset(const int16_t& x, const int16_t& y)
{
    // draw before applying offset
    this->draw();

    glUniform2i(this->uniform_offset, GLint(x), GLint(y));
}


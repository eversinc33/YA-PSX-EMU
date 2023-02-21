#include "Renderer.h"
#include "../util/logging.h"
#include "../util/filesystem.h"
#include "Constants.h"
#include <exception>

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

Renderer::Renderer()
{
    if (!this->init_sdl()) 
    {
        throw std::exception();
    }

    // Read and compile shaders TODO relative paths
    auto vs_src = read_file_to_string("/Users/sven/Documents/Code/Repos/YA-PSX-EMU/gpu/shaders/vertex.glsl");
    auto fg_src = read_file_to_string("/Users/sven/Documents/Code/Repos/YA-PSX-EMU/gpu/shaders/fragment.glsl");
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
    this->positions.create();
    auto index = find_program_attrib(program, "vertex_position");
    glEnableVertexAttribArray(index);
    // 2 GLShort attributes, not normalized
    glVertexAttribIPointer(index, 2, GL_SHORT, 0, nullptr);

    // Set up Color buffer
    this->colors = Buffer<Color>();
    this->colors.create();
    index = find_program_attrib(program, "vertex_color");
    glEnableVertexAttribArray(index);
    // 3 GLubyte attributes, not normalized
    glVertexAttribIPointer(index, 3, GL_UNSIGNED_BYTE, 0, nullptr);

    // Clear screen
    glClearColor(0.0, 0.0, 0.0, 1.0);    
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(this->window);
}

bool Renderer::init_sdl()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    this->window = SDL_CreateWindow("PSX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH_PX, SCREEN_HEIGHT_PX, SDL_WINDOW_SHOWN);
    if (this->window == NULL)
    {
        DEBUG("Window could not be created! SDL_Error: " << SDL_GetError());
        return false;
    }

    this->gl_context     = SDL_GL_CreateContext(this->window);
    this->screen_surface = SDL_GetWindowSurface(this->window);

    return true;
}

void Renderer::display()
{
    this->draw();
    SDL_GL_SwapWindow(this->window);
}

#ifdef DEPRECATED
void Renderer::poll()
{
    SDL_Event e; 
    SDL_PollEvent(&e);
    switch (e.type)
    {
        case SDL_WINDOWEVENT:
            switch (e.window.event) 
            {
                case SDL_WINDOWEVENT_CLOSE:  
                    DEBUG("Window closed")
                    this->should_quit = true;
                    break;
                default: break;
            }
            break; 
        case SDL_QUIT:
            this->should_quit = true;
            break;              
        default: break;
    }

    if (this->should_quit)
    {
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }
}
#endif

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
    
    for (const auto& i : { 0,1,2,3 }) 
    {
        // push
        this->positions.set(this->nvertices, positions[i]);
        this->colors.set(this->nvertices, colors[i]);
        this->nvertices++;
    }
}

Renderer::~Renderer()
{
    glDeleteVertexArrays(1, &this->vertex_array_object);
    glDeleteShader(this->vertex_shader);
    glDeleteShader(this->fragment_shader);
    glDeleteProgram(this->program);
}

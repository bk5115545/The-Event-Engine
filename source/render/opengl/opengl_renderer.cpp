#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/opengl/opengl_renderer.h"

OpenGLRenderer::OpenGLRenderer() {
    window = nullptr;
    context = nullptr;
    camera = nullptr;
    width = 800;
    height = 600;
}

OpenGLRenderer::OpenGLRenderer(int screen_width, int screen_height) {
    window = nullptr;
    context = nullptr;
    width = screen_width;
    height = screen_height;
}

OpenGLRenderer::~OpenGLRenderer() {
    glDeleteVertexArrays(1, &vertex_array_id);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
}

bool OpenGLRenderer::initialize() {
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                              SDL_WINDOW_OPENGL);

    if (window == nullptr) {
        std::cerr << "Error with SDL_CreateWindow in opengl renderer." << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    context = SDL_GL_CreateContext(window);

    if (context == nullptr) {
        std::cerr << "SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
        return 1;
    }

    glewExperimental = GL_TRUE;
    glewInit();

    // Set to 1 to enable V-Sync, 0 to disale V-Sync, and -1 if you want to allow tearing for slow rendering
    SDL_GL_SetSwapInterval(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0, 0, 0, 0);

    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    return true;
}

void OpenGLRenderer::addModel(GlDrawable *model) { models.push_back(model); }

void OpenGLRenderer::preDraw() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void OpenGLRenderer::draw() {
    for (auto iter = models.begin(); iter != models.end(); ++iter) {
        (*iter)->draw(camera->get_vp_matrix());
    }
}

void OpenGLRenderer::postDraw() { SDL_GL_SwapWindow(window); }

int OpenGLRenderer::get_width() { return width; }

int OpenGLRenderer::get_height() { return height; }

void OpenGLRenderer::set_camera(Camera *cam) { camera = (GlCamera *)cam; }

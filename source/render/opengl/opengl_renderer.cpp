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
#include "util/game_utils.h"

OpenGLRenderer::OpenGLRenderer() {
    window_ = nullptr;
    context_ = nullptr;
    camera_ = nullptr;
    width_ = 800;
    height_ = 600;
}

OpenGLRenderer::OpenGLRenderer(uint32 screen_width, uint32 screen_height) {
    window_ = nullptr;
    context_ = nullptr;
    width_ = screen_width;
    height_ = screen_height;
}

OpenGLRenderer::~OpenGLRenderer() {
    glDeleteVertexArrays(1, &vertex_array_id_);
    SDL_GL_DeleteContext(context_);
    SDL_DestroyWindow(window_);
}

bool OpenGLRenderer::Initialize() {
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    window_ = SDL_CreateWindow("Game Engine",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               width_,
                               height_,
                               SDL_WINDOW_OPENGL
    );

    if (window_ == nullptr) {
        LogSDLError(std::cerr, "SDL_CreateWindow");
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    context_ = SDL_GL_CreateContext(window_);

    if(context_ == nullptr) {
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

    glGenVertexArrays(1, &vertex_array_id_);
    glBindVertexArray(vertex_array_id_);

    return true;
}

void OpenGLRenderer::AddModel(GlDrawable* model) {
    models_.push_back(model);
}

void OpenGLRenderer::PreDraw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::Draw() {
    for(auto iter = models_.begin(); iter != models_.end(); ++iter) {
        (*iter)->Draw(camera_->vp_matrix());
    }
}

void OpenGLRenderer::PostDraw() {
    SDL_GL_SwapWindow(window_);
}

uint32 OpenGLRenderer::width() {
    return width_;
}
uint32 OpenGLRenderer::height() {
    return height_;
}

void OpenGLRenderer::set_camera(GlCamera* camera) {
    camera_ = camera;
}

#pragma once

#include <vector>
#include <memory>

// Opengl related includes
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util/definitions.h"
#include "render/renderer.h"
#include "render/opengl/gl_camera.h"
#include "render/opengl/gl_drawable.h"

class GlDrawable;

class OpenGLRenderer : public Renderer {
protected:
    SDL_Window* window_;
    SDL_GLContext context_;
    GlCamera* camera_;
    GLuint mvp_uniform_;
    GLuint vertex_array_id_;

    std::vector<GlDrawable*> models_;

    uint32 width_;
    uint32 height_;

public:
    OpenGLRenderer();
    OpenGLRenderer(uint32 screen_width, uint32 screen_height);

    ~OpenGLRenderer();

    bool Initialize();
    void AddModel(GlDrawable*);

    void PreDraw();
    void Draw();
    void PostDraw();

    uint32 width();
    uint32 height();

    void set_camera(GlCamera*);
};

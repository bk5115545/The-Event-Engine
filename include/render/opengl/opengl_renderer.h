#pragma once

#include <vector>
#include <memory>

// Opengl related includes
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define GLM_FORCE_RADIANS

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render/Renderer.h"
#include "render/opengl/gl_camera.h"
#include "render/opengl/gl_drawable.h"

class GlDrawable;

class OpenGLRenderer : public Renderer {
  protected:
    SDL_Window* window;
    SDL_GLContext context;
    GLCamera* camera;
    GLuint mvp_uniform;
    GLuint vertex_array_id;

    std::vector<GlDrawable*> models;

    int width;
    int height;

  public:
    OpenGLRenderer();
    OpenGLRenderer(int screen_width, int screen_height);

    ~OpenGLRenderer();

    bool initialize();
    void addModel(GlDrawable* model);

    void preDraw();
    void draw();
    void postDraw();

    int get_width();
    int get_height();

    void set_camera(Camera* cam);
};

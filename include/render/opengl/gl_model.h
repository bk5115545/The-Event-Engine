#pragma once

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
#include <glm/gtc/type_ptr.hpp>

#include "util/Definitions.h"
#include "render/Renderer.h"

class GLModel {
  protected:
    GLuint vertex_buffer_object;
    GLuint color_buffer_object;

    GLuint mvp_id;
    // GLfloat* vertex_buffer_data_;
    // GLfloat* color_buffer_data_;

    GLuint shader_program;

    GLuint loadShaders(std::string, std::string);

  public:
    GLModel();
    ~GLModel();

    void initialize();
    void draw(glm::mat4);
    void bufferData();
};

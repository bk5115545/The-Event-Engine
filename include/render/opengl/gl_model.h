#pragma once

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

#include "util/definitions.h"
#include "render/renderer.h"

class GlModel {
protected:
    GLuint vertex_buffer_object_;
    GLuint color_buffer_object_;

    GLuint mvp_id_;
    // GLfloat* vertex_buffer_data_;
    // GLfloat* color_buffer_data_;

    GLuint shader_program_;

    GLuint LoadShaders(std::string, std::string);

public:
    GlModel();
    ~GlModel();

    void Initialize();
    void Draw(glm::mat4);
    void BufferData();
};

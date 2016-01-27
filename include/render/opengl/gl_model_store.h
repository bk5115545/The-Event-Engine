#pragma once

#include <map>
#include <string>
#include <stack>

// #include <SDL.h>
// #include <GL/glew.h>
// #include <SDL_opengl.h>
// #include <GL/gl.h>
// #include <GL/glu.h>

#include "render/opengl/gl_model.h"

class GLModelStore {
  protected:
    std::map<std::string, std::shared_ptr<GLModel>> model_store;

  public:
    GLModelStore();
    ~GLModelStore();

    bool loadAssets();
    std::shared_ptr<GLModel> search(std::string);
};

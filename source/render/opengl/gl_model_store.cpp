#include <string>
#include <utility>

#include "render/opengl/gl_model_store.h"

GLModelStore::GLModelStore() {}

GLModelStore::~GLModelStore() {}

bool GLModelStore::loadAssets() {
    // Hard coded cube
    std::shared_ptr<GLModel> cube = std::shared_ptr<GLModel>(new GLModel());
    cube->initialize();
    model_store.insert(std::pair<std::string, std::shared_ptr<GLModel>>("cube", cube));
    return true;
}

std::shared_ptr<GLModel> GLModelStore::search(std::string model_name) { return model_store.at(model_name); }

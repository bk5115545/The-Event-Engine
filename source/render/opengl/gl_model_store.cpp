#include <string>
#include <utility>

#include "render/opengl/gl_model_store.h"

GlModelStore::GlModelStore() {}

GlModelStore::~GlModelStore() {}

bool GlModelStore::loadAssets() {
    // Hard coded cube
    std::shared_ptr<GlModel> cube = std::shared_ptr<GlModel>(new GlModel());
    cube->initialize();
    model_store.insert(std::pair<std::string, std::shared_ptr<GlModel>>("cube", cube));
    return true;
}

std::shared_ptr<GlModel> GlModelStore::search(std::string model_name) { return model_store.at(model_name); }

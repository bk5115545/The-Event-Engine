#include <string>
#include <utility>

#include "render/opengl/gl_model_store.h"

GlModelStore::GlModelStore() {
}

GlModelStore::~GlModelStore() {

}

bool GlModelStore::LoadAssets() {
    // Hard coded cube
    GlModel* cube = new GlModel();
    cube->Initialize();
    model_store_.insert(std::pair<std::string, GlModel*>("cube", cube));
    return true;
}

GlModel* GlModelStore::Search(std::string model_name) {
    return model_store_.at(model_name);
}

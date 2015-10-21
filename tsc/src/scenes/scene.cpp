#include "../core/global_basic.hpp"
#include "scene.hpp"

using namespace TSC;

cScene::~cScene()
{
    //
}

/**
 * Return the name of this scene. This is only used for debugging the
 * scene stack. Please return a name that identifies your cScene
 * subclass uniquely.
 */
std::string cScene::Name() const
{
    return "(Generic Scene)";
}

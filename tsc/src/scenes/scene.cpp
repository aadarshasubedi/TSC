#include "../core/global_basic.hpp"
#include "scene.hpp"

using namespace TSC;

cScene::cScene()
    : m_finished(false)
{
    //
}

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

/**
 * Indicate the scene has finished. The scene manager
 * will pop and delete (free) this scene instance if
 * this flag is set.
 *
 * Note a pecularity regarding freeing of the scene. Since a scene
 * upon finishing may push new (subsequent) scenes onto the scene
 * stack, the scene manager doesn’t pop-and-free the finished scene
 * right at the end of the mainloop iteration in which Finish() was
 * called. Instead, the finished scene will remain in the scene stack
 * (buried below the new scenes it pushed, if any), but when it is
 * found to be at the top again (with the finish flag set), it will be
 * popped and deleted as described. There will *not* be a further call
 * to the scene’s Update() or Draw() method, the scene manager checks
 * the finish flag before it executes those methods, and if it is
 * found, the scene is popped and freed as described, and the mainloop
 * is started from new (then finding the next scene on the stack).
 */
void cScene::Finish()
{
    m_finished = true;
}

/**
 * Check whether the scene has finished (cf. Finish()).
 */
bool cScene::Has_Finished() const
{
    return m_finished;
}

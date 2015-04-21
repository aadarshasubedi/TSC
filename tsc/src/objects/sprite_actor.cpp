#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/property_helper.hpp"
#include "../core/bintree.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_settings.hpp"
#include "../video/img_manager.hpp"
#include "../level/level.hpp"
#include "actor.hpp"
#include "sprite_actor.hpp"

using namespace TSC;
namespace fs = boost::filesystem;

cSpriteActor::cSpriteActor(fs::path relative_texture_path)
    : cActor()
{
    m_rel_texture_path = relative_texture_path;
}

cSpriteActor::~cSpriteActor()
{
    //
}

void cSpriteActor::Added_To_Level(cLevel* p_level, const unsigned long& uid)
{
    cActor::Added_To_Level(p_level, uid);

    const cImage_Manager::ConfiguredTexture& txtinfo = p_level->Get_ImageManager()->Get_Texture(m_rel_texture_path);
    m_sprite.setTexture(*txtinfo.m_texture);
    txtinfo.m_settings->Apply(*this);
}

void cSpriteActor::Draw(sf::RenderWindow& stage) const
{
    stage.draw(m_sprite, getTransform()); // <3 SFML. This allows us to apply the same transformations that affect the collision rectangle to also affect the sprite.

    cActor::Draw(stage);
}

/**
 * Set width and height of this actor, in pixels. Also resets the
 * collision rectangle to equal exactly that new dimensions, so if
 * you want a different want, set one with Set_Collision_Rect()
 * afterwards.
 *
 * This is *not* the method for scaling sprites up/down! Use the
 * inherited setScale() method for that, which will affect the
 * collision rectangle as well. This method is only intended for
 * specifying the pre-transform measures when the actor sprite is
 * loaded from its file originally.
 *
 * TODO: For a later version, we might consider specifying the collision
 * rect ontop of the original coordinates rather than on the rectangle
 * resulting after the `width` and `height` settings keys are applied.
 * Then we could just scale the collision rectangle completely together
 * with the main sprite rectangle, and we could get rid of this method
 * completely. I.e. if an image is 256x256 as a PNG file, specify the
 * collision rect on that coordinates, e.g. 10, 10, 140, 140. Afterwards,
 * scale it down together with the main image rectangle (or even abolish
 * the `width` and `height` keys and replace them width a simple `scale`
 * factor settings). Currently, you have to specify the collision rect
 * on the result of what the `width` and `height` keys cause, which can
 * result in quite some headache with trial&error.
 *
 * \param width New width, in pixels.
 * \param height New height, in pixels.
 */
void cSpriteActor::Set_Dimensions(int width, int height)
{
    sf::FloatRect origrect = m_sprite.getLocalBounds();
    float newwidth = width / origrect.width;
    float newheight = height / origrect.height;

    /* Scale the visible sprite accordingly. Note how we do NOT
     * call setScale() directly on this instance, but only on
     * the m_sprite. This is because otherwise we would affect
     * the transform of the collision rectangle with it, which
     * is undesired, because the collision rectangle’s width and
     * height should be settable independantly. See the large TODO
     * comment in this method’s docs. */
    m_sprite.setScale(newwidth, newheight);
    Set_Collision_Rect(sf::FloatRect(0, 0, width, height));
}

bool cSpriteActor::Handle_Collision(cCollision* p_collision)
{
    cActor::Handle_Collision(p_collision);
    std::cout << "COLLISION ON CSPRITEACTOR!" << std::endl;
    return true;
}

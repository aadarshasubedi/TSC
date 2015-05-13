#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/property_helper.hpp"
#include "../core/bintree.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_set.hpp"
#include "../video/img_settings.hpp"
#include "../video/img_manager.hpp"
#include "../level/level.hpp"
#include "actor.hpp"
#include "../core/collision.hpp"
#include "sprite_actor.hpp"
#include "static_actor.hpp"
#include "../core/tsc_app.hpp"

using namespace TSC;
namespace fs = boost::filesystem;

/**
 * Default constructor. The sprite gets attached the pixmaps/game/dummy_1.png
 * image as its texture.
 */
cStaticActor::cStaticActor()
    : cSpriteActor()
{
    Set_Texture(utf8_to_path("game/dummy_1.png"));
}

/**
 * Convenience constructor. Directly specify the path to the image
 * you want to use as the texture for this sprite.
 */
cStaticActor::cStaticActor(boost::filesystem::path relative_texture_path)
    : cSpriteActor()
{
    Set_Texture(relative_texture_path);
}

cStaticActor::~cStaticActor()
{
    //
}

/**
 * Set this sprite’s texture to a different one.
 *
 * This method ignores imagesets and thus should only be
 * used by sprites that never change their image.
 * Use instances of cMovingActor instead, and on those,
 * use the Set_Image_Num() methods inherited from ImageSet.
 */
void cStaticActor::Set_Texture(fs::path relative_texture_path)
{
    m_rel_texture_path = relative_texture_path;

    const struct ConfiguredTexture& txtinfo = gp_app->Get_ImageManager().Get_Texture(m_rel_texture_path);
    m_sprite.setTexture(*txtinfo.m_texture);
    txtinfo.m_settings->Apply(*this);
}

void cStaticActor::Draw(sf::RenderWindow& stage) const
{
    cSpriteActor::Draw(stage);
}

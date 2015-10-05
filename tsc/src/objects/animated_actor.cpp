#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../video/img_manager.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_settings.hpp"
#include "../video/img_set.hpp"
#include "../scripting/scriptable_object.hpp"
#include "actor.hpp"
#include "sprite_actor.hpp"
#include "animated_actor.hpp"

using namespace TSC;
namespace fs = boost::filesystem;

/**
 * Construct new animated actor. The given texture is used as the initial
 * image.
 */
cAnimatedActor::cAnimatedActor()
    : cSpriteActor(), cImageSet()
{
}

cAnimatedActor::cAnimatedActor(XmlAttributes& attributes, cLevel& level, const std::string type_name)
    : cSpriteActor(attributes, level, type_name)
{
    //
}

cAnimatedActor::~cAnimatedActor()
{
    //
}

/**
 * Callback function for cImage_Set::Set_Image_Num().
 */
void cAnimatedActor::Set_Image_Set_Image(const struct ConfiguredTexture* p_new_image, bool new_startimage /* = false */)
{
    // TODO: Handle `new_startimage'
    m_sprite.setTexture(*(p_new_image->m_texture));
    p_new_image->m_settings->Apply(*this);
}

/**
 * Override of cImage_Set::Get_Identity().
 */
std::string cAnimatedActor::Get_Identity(void)
{
    std::stringstream ss;
    ss << "sprite type " << "(TODO: m_type)" << ", name " << m_name;
    return ss.str();
}

void cAnimatedActor::Draw(sf::RenderWindow& stage) const
{
    if (m_named_ranges.empty()) {
        std::cerr << "Warning: Objekt '" << m_name << "' is an animated sprite with an empty imageset list!" << std::endl;
    }

    cSpriteActor::Draw(stage);
}

void cAnimatedActor::Update()
{
    cSpriteActor::Update();
    cImageSet::Update_Animation();
}

/***************************************************************************
 * level_loader.cpp - loading level XML
 *
 * Copyright © 2013 - 2014 The TSC Contributors
 ***************************************************************************/
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../core/global_basic.hpp"
#include "../core/global_game.hpp"
#include "../core/bintree.hpp"
#include "../core/property_helper.hpp"
#include "../core/errors.hpp"
#include "../core/xml_attributes.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_set.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../objects/actor.hpp"
#include "../objects/sprite_actor.hpp"
#include "../objects/static_actor.hpp"
#include "../objects/animated_actor.hpp"
#include "../objects/enemystopper.hpp"
#include "../objects/box.hpp"
#include "../objects/bonusbox.hpp"
#include "../objects/spinbox.hpp"
#include "../objects/text_box.hpp"
#include "../core/filesystem/resource_manager.hpp"
#include "../core/tsc_app.hpp"
#include "../video/img_manager.hpp"
#include "level_player.hpp"
#include "level.hpp"
#include "level_loader.hpp"

namespace fs = boost::filesystem;
using namespace TSC;

using namespace std;

cLevelLoader::cLevelLoader()
    : xmlpp::SaxParser()
{
    mp_level    = NULL;
}

cLevelLoader::~cLevelLoader()
{
    // Do not delete the cLevel instance — it is used by the
    // caller and deleted by him.
    mp_level = NULL;
}

cLevel* cLevelLoader::Get_Level()
{
    return mp_level;
}

/***************************************
 * SAX parser callbacks
 ***************************************/

void cLevelLoader::parse_file(boost::filesystem::path filename)
{
    m_levelfile = filename;
    xmlpp::SaxParser::parse_file(path_to_utf8(filename));
}

void cLevelLoader::on_start_document()
{
    if (mp_level)
        throw("Restarted XML parser after already starting it."); // FIXME: proper exception

    mp_level = new cLevel();
    m_in_script_tag = false;
}

void cLevelLoader::on_end_document()
{
    mp_level->m_level_filename = m_levelfile;

    // engine version entry not set
    if (mp_level->m_engine_version < 0)
        mp_level->m_engine_version = 0;
}

void cLevelLoader::on_start_element(const Glib::ustring& name, const xmlpp::SaxParser::AttributeList& properties)
{
    if (name == "property" || name == "Property") {
        std::string key;
        std::string value;

        /* Collect all the <property> elements for the surrounding
         * mayor element (like <settings> or <sprite>). When the
         * surrounding element is closed, the results are handled
         * in on_end_element(). */
        for (xmlpp::SaxParser::AttributeList::const_iterator iter = properties.begin(); iter != properties.end(); iter++) {
            xmlpp::SaxParser::Attribute attr = *iter;

            if (attr.name == "name")
                key = attr.value;
            else if (attr.name == "value")
                value = attr.value;
        }

        m_current_properties[key] = value;
    }
    else if (name == "script") {
        // Indicate a script tag has opened, so we can retrieve
        // its and only its text.
        m_in_script_tag = true;
    }
}

void cLevelLoader::on_end_element(const Glib::ustring& name)
{
    // <property> tags are parsed cumulatively in on_start_element()
    // so all have been collected when the surrounding element
    // terminates here.
    if (name == "property" || name == "Property")
        return;

    // Now for the real, cumbersome parsing process
    if (name == "information")
        Parse_Tag_Information();
    else if (name == "settings")
        Parse_Tag_Settings();
    // OLD else if (name == "background")
    // OLD     Parse_Tag_Background();
    else if (name == "player")
        Parse_Tag_Player();
    else if (cLevel::Is_Level_Object_Element(std::string(name))) // CEGUI doesn’t like Glib::ustring
        Parse_Level_Object_Tag(name);
    else if (name == "level") {
        /* Ignore the root <level> tag */
    }
    else if (name == "script")
        m_in_script_tag = false; // Indicate the <script> tag has ended
    else
        cerr << "Warning: Unknown XML tag '" << name << "'on level parsing." << endl;

    // Everything handled, so we can now safely clear the
    // collected <property> element values for the next
    // tag.
    m_current_properties.clear();
}

void cLevelLoader::on_characters(const Glib::ustring& text)
{
    /* If we’re currently in the <script> tag, read its
     * text (may be called multiple times for each token,
     * so append rather then set directly). */
    if (m_in_script_tag)
        mp_level->m_script.append(text);
}

/***************************************
 * Parsers for mayor XML tags
 ***************************************/

void cLevelLoader::Parse_Tag_Information()
{
    // Support V1.7 and lower which used float
    float engine_version_float = string_to_float(m_current_properties["engine_version"]);

    // if float engine version
    if (engine_version_float < 3)
        engine_version_float *= 10; // change to new format

    mp_level->m_engine_version = static_cast<int>(engine_version_float);
    mp_level->m_last_saved     = string_to_int64(m_current_properties["save_time"]);
}

void cLevelLoader::Parse_Tag_Settings()
{
    // If V1.9 and lower: Move Y coordinate bottom to 0
    if (mp_level->m_engine_version < 35 && m_current_properties.count("cam_limit_h") > 0) {
        float y = string_to_float(m_current_properties["cam_limit_h"]);
        m_current_properties["cam_limit_h"] = float_to_string(y - 600.0f);
    }

    mp_level->Set_Author(m_current_properties["lvl_author"]);
    mp_level->Set_Version(m_current_properties["lvl_version"]);
    mp_level->Set_Difficulty(string_to_int(m_current_properties["lvl_difficulty"]));
    mp_level->Set_Description(xml_string_to_string(m_current_properties["lvl_description"]));
    mp_level->Set_Music(utf8_to_path(m_current_properties["lvl_music"]));
    mp_level->Set_Land_Type(Get_Level_Land_Type_Id(m_current_properties["lvl_land_type"]));

    mp_level->m_fixed_camera_hor_vel = string_to_float(m_current_properties["cam_fixed_hor_vel"]);

    mp_level->m_camera_limits = sf::FloatRect(string_to_float(m_current_properties["cam_limit_x"]),
                                              string_to_float(m_current_properties["cam_limit_y"]),
                                              string_to_float(m_current_properties["cam_limit_w"]),
                                              string_to_float(m_current_properties["cam_limit_h"]));

    mp_level->m_unload_after_exit = static_cast<bool>(string_to_int(m_current_properties["unload_after_exit"]));
}

// OLD void cLevelLoader::Parse_Tag_Background()
// OLD {
// OLD     BackgroundType bg_type = static_cast<BackgroundType>(string_to_int(m_current_properties["type"]));
// OLD 
// OLD     // Use gradient background
// OLD     if (bg_type == BG_GR_HOR || bg_type == BG_GR_VER)
// OLD         mp_level->m_background_manager->Get_Pointer(0)->Load_From_Attributes(m_current_properties);
// OLD     else // default background
// OLD         mp_level->m_background_manager->Add(new cBackground(m_current_properties, mp_level->m_sprite_manager));
// OLD }

void cLevelLoader::Parse_Tag_Player()
{
    // Defaults
    mp_level->m_player_start_pos_x = cLevel_Player::m_default_pos_x;
    mp_level->m_player_start_pos_y = cLevel_Player::m_default_pos_y;

    if (m_current_properties.count("posx"))
        mp_level->m_player_start_pos_x = string_to_float(m_current_properties["posx"]);
    if (m_current_properties.count("posy"))
        mp_level->m_player_start_pos_y = string_to_float(m_current_properties["posy"]);

    // If V1.9 and lower: Move Y coordinate bottom to 0
    if (mp_level->m_engine_version < 35)
        mp_level->m_player_start_pos_y -= 600.0f;

    // Adjust with Y offsets other versions had
    if (mp_level->m_engine_version <= 10)
        mp_level->m_player_start_pos_y += 58.0f;
    else if (mp_level->m_engine_version <= 20)
        mp_level->m_player_start_pos_y -= 48.0f;

    // direction, set to default if invalid
    mp_level->m_player_start_direction = Get_Direction_Id(m_current_properties["direction"]);
    if (mp_level->m_player_start_direction != DIR_LEFT && mp_level->m_player_start_direction != DIR_RIGHT)
        mp_level->m_player_start_direction = DIR_RIGHT;

    /* TODO: Maybe move this directly in on_start_document()?
     * Currently, if the <player> tag is not the first tag,
     * the player may not be the first actor added, which is
     * bad. Then only do the configuration here. */
    cLevel_Player* p_player = new cLevel_Player();
    p_player->Set_Pos(mp_level->m_player_start_pos_x,
                      mp_level->m_player_start_pos_y,
                      true);
    // p_player->SetDir(mp_level->m_player_start_direction) NOT YET RE-IMPLEMENTED

    mp_level->Add_Player(p_player);
}

void cLevelLoader::Parse_Level_Object_Tag(const std::string& name)
{
    // create sprite
    std::vector<cActor*> actors = Create_Level_Objects_From_XML_Tag(name, m_current_properties, *mp_level, mp_level->m_engine_version);

    // valid
    if (actors.size() > 0) {
        for (std::vector<cActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++) {
            /* If a static UID has been set, assign it to the sprite. -1
             * (cSprite’s default) means "no UID set" and instructs
             * cSprite_Manager to assign a new, free one to the
             * object. Note that in the unlikely case of intermixed static
             * and dynamic UIDs (i.e.  some sprites have UIDs assigned,
             * others not — this can only be achieved by editing the level
             * XML by hand, the editor does not allow this) it could
             * happen that cSprite_Manager’s UID generator generates a UID
             * (and assignes it to the object) that is later set as a
             * static UID for another sprite — causing this sprite to have
             * the same UID as the one with the generated UID (this will
             * print a warning to the console if TSC is compiled in debug
             * mode). We cannot know this in advance, but as said you have
             * to edit the XML by hand and therefore we can ignore this
             * case safely. */
            if (iter == actors.begin() && m_current_properties.count("uid")) {
                mp_level->Add_Actor(*iter, string_to_int(m_current_properties["uid"])); // The 98% case is that we get only one sprite back, the other 2% are backward compatibility
            }
            else {
                mp_level->Add_Actor(*iter);
            }
        }
    }
}

/***************************************
 * Create_Level_Objects_From_XML_Tag()
 ***************************************/

std::vector<cActor*> cLevelLoader::Create_Level_Objects_From_XML_Tag(const std::string& name, XmlAttributes& attributes, cLevel& level, int engine_version)
{
    if (name == "sprite")
        return Create_Sprites_From_XML_Tag(name, attributes, level, engine_version);
    else if (name == "enemystopper")
        return Create_Enemy_Stoppers_From_XML_Tag(name, attributes, level, engine_version);
    // OLD else if (name == "levelexit")
    // OLD     return Create_Level_Exits_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "level_entry")
    // OLD     return Create_Level_Entries_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    else if (name == "box")
        return Create_Boxes_From_XML_Tag(name, attributes, level, engine_version);
    // OLD else if (name == "item" || name == "powerup") // powerup is pre V.0.99.5
    // OLD     return Create_Items_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "moving_platform")
    // OLD     return Create_Moving_Platforms_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "falling_platform") // falling platform is pre V.1.5
    // OLD     return Create_Falling_Platforms_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "enemy")
    // OLD     return Create_Enemies_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "sound")
    // OLD     return Create_Sounds_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "particle_emitter")
    // OLD     return Create_Particle_Emitters_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "path")
    // OLD     return Create_Paths_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "global_effect") // global_effect is V.1.9 and lower
    // OLD     return Create_Global_Effects_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "ball")
    // OLD     return Create_Balls_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "lava")
    // OLD     return Create_Lavas_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    // OLD else if (name == "crate")
    // OLD     return Create_Crates_From_XML_Tag(name, attributes, engine_version, p_sprite_manager);
    else
        cerr << "Warning: Unknown level object element '" << name << "'. Is cLevelLoader::Create_Level_Objects_From_XML_Tag() in sync with cLevel::Is_Level_Object_Element()?" << endl;

    // keep above list sync with cLevel::Is_Level_Object_Element()

    // This is not a level object tag, return empty list
    return std::vector<cActor*>();
}

std::vector<cActor*> cLevelLoader::Create_Sprites_From_XML_Tag(const std::string& name, XmlAttributes& attributes, cLevel& level, int engine_version)
{
    std::vector<cActor*> result;

    // V1.4 and lower: change some image paths
    if (engine_version < 25) {
        attributes.relocate_image("game/box/stone8.png", "blocks/metal/stone_2_violet.png");
        attributes.relocate_image("ground/jungle_1/tree_type_1.png", "ground/jungle_1/tree/1.png");
        attributes.relocate_image("ground/jungle_1/tree_type_1_front.png", "ground/jungle_1/tree/1_front.png");
        attributes.relocate_image("ground/jungle_1/tree_type_2.png", "ground/jungle_1/tree/2.png");
        attributes.relocate_image("ground/yoshi_1/extra_1_blue.png", "ground/jungle_2/hedge/1_blue.png");
        attributes.relocate_image("ground/yoshi_1/extra_1_green.png", "ground/jungle_2/hedge/1_green.png");
        attributes.relocate_image("ground/yoshi_1/extra_1_red.png", "ground/jungle_2/hedge/1_red.png");
        attributes.relocate_image("ground/yoshi_1/extra_1_yellow.png", "ground/jungle_2/hedge/1_yellow.png");
        attributes.relocate_image("ground/yoshi_1/rope_1_leftright.png", "ground/jungle_2/rope_1_hor.png");
    }
    // V1.5 and lower: change pipe connection image paths
    if (engine_version < 28) {
        attributes.relocate_image("blocks/pipe/connection_left_down.png", "blocks/pipe/connection/plastic_1/orange/right_up.png");
        attributes.relocate_image("blocks/pipe/connection_left_up.png", "blocks/pipe/connection/plastic_1/orange/right_down.png");
        attributes.relocate_image("blocks/pipe/connection_right_down.png", "blocks/pipe/connection/plastic_1/orange/left_up.png");
        attributes.relocate_image("blocks/pipe/connection_right_up.png", "blocks/pipe/connection/plastic_1/orange/left_down.png");
        attributes.relocate_image("blocks/pipe/metal_connector.png", "blocks/pipe/connection/metal_1/grey/middle.png");
    }
    // V1.7 and lower: change yoshi_1 hill_up to jungle_1 slider image paths
    if (engine_version < 31) {
        attributes.relocate_image("ground/yoshi_1/hill_up_1.png", "ground/jungle_1/slider/2_green_left.png");
        attributes.relocate_image("ground/yoshi_1/hill_up_2.png", "ground/jungle_1/slider/2_blue_left.png");
        attributes.relocate_image("ground/yoshi_1/hill_up_3.png", "ground/jungle_1/slider/2_brown_left.png");
    }
    // V.1.7 and lower : change slider grey_1 to green_1 brown slider image paths
    if (engine_version < 32) {
        attributes.relocate_image("slider/grey_1/slider_left.png", "ground/green_1/slider/1/brown/left.png");
        attributes.relocate_image("slider/grey_1/slider_middle.png", "ground/green_1/slider/1/brown/middle.png");
        attributes.relocate_image("slider/grey_1/slider_right.png", "ground/green_1/slider/1/brown/right.png");
    }
    // V1.7.x and lower: change green_1 ground to green_3 ground image paths
    if (engine_version < 34) {
        // normal
        attributes.relocate_image("ground/green_1/ground/left_up.png", "ground/green_3/ground/top/left.png");
        attributes.relocate_image("ground/green_1/ground/left_down.png", "ground/green_3/ground/bottom/left.png");
        attributes.relocate_image("ground/green_1/ground/right_up.png", "ground/green_3/ground/top/right.png");
        attributes.relocate_image("ground/green_1/ground/right_down.png", "ground/green_3/ground/bottom/right.png");
        attributes.relocate_image("ground/green_1/ground/up.png", "ground/green_3/ground/top/1.png");
        attributes.relocate_image("ground/green_1/ground/down.png", "ground/green_3/ground/bottom/1.png");
        attributes.relocate_image("ground/green_1/ground/right.png", "ground/green_3/ground/middle/right.png");
        attributes.relocate_image("ground/green_1/ground/left.png", "ground/green_3/ground/middle/left.png");
        attributes.relocate_image("ground/green_1/ground/middle.png", "ground/green_3/ground/middle/1.png");

        // hill (not available)
        //attributes.relocate_image( "ground/green_1/ground/hill_left_up.png", "ground/green_3/ground/" );
        //attributes.relocate_image( "ground/green_1/ground/hill_right_up.png", "ground/green_3/ground/" );
        //attributes.relocate_image( "ground/green_1/ground/hill_right.png", "ground/green_3/ground/" );
        //attributes.relocate_image( "ground/green_1/ground/hill_left.png", "ground/green_3/ground/" );
    }
    // V1.9 and lower: move y coordinate bottom to 0
    if (engine_version < 35) {
        if (attributes.count("posy") > 0)
            attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
    }
    // V1.9.x and lower: change fire-1 animation to particles
    if (engine_version < 37) {
        attributes.relocate_image("animation/fire_1/1.png", "animation/particles/fire_1.png");
        attributes.relocate_image("animation/fire_1/2.png", "animation/particles/fire_2.png");
        attributes.relocate_image("animation/fire_1/3.png", "animation/particles/fire_3.png");
        attributes.relocate_image("animation/fire_1/4.png", "animation/particles/fire_4.png");
    }
    // V2.0.0-beta6 and lower: green pipes have been removed, change to grey pipes.
    if (engine_version < 41) {
        attributes.relocate_image("pipes/green/ver.png", "pipes/grey/ver.png");
        attributes.relocate_image("pipes/green/hor.png", "pipes/grey/hor.png");
        attributes.relocate_image("pipes/green/up.png", "pipes/grey/up.png");
        attributes.relocate_image("pipes/green/left.png", "pipes/grey/left.png");
        attributes.relocate_image("pipes/green/right.png", "pipes/grey/right.png");
        attributes.relocate_image("pipes/green/down.png", "pipes/grey/down.png");
    }
    // V2.0.0-beta6 and lower: ver.png was renamed to ver_1.png. Note this
    // builds upon the previous line for green pipes. hor.png was renamed to hor_1.png.
    if (engine_version < 42) {
        attributes.relocate_image("pipes/blue/ver.png", "pipes/blue/ver_1.png");
        attributes.relocate_image("pipes/blue/hor.png", "pipes/blue/hor_1.png");
        attributes.relocate_image("pipes/grey/ver.png", "pipes/grey/ver_1.png");
        attributes.relocate_image("pipes/grey/hor.png", "pipes/grey/hor_1.png");
        attributes.relocate_image("pipes/orange/ver.png", "pipes/orange/ver_1.png");
        attributes.relocate_image("pipes/orange/hor.png", "pipes/orange/hor_1.png");
        attributes.relocate_image("pipes/yellow/ver.png", "pipes/yellow/ver_1.png");
        attributes.relocate_image("pipes/yellow/hor.png", "pipes/yellow/hor_1.png");
        // green was removed (see above)
    }
    // V2.0.0-beta6 and lower: up.png was renamed to up_1.png (same for left, down, right).
    // Note this builds upon the previous line for green pipes.
    if (engine_version < 43) {
        attributes.relocate_image("pipes/blue/up.png",    "pipes/blue/up_1.png");
        attributes.relocate_image("pipes/blue/left.png",  "pipes/blue/left_1.png");
        attributes.relocate_image("pipes/blue/right.png", "pipes/blue/right_1.png");
        attributes.relocate_image("pipes/blue/down.png",  "pipes/blue/down_1.png");
        attributes.relocate_image("pipes/grey/up.png",    "pipes/grey/up_1.png");
        attributes.relocate_image("pipes/grey/left.png",  "pipes/grey/left_1.png");
        attributes.relocate_image("pipes/grey/right.png", "pipes/grey/right_1.png");
        attributes.relocate_image("pipes/grey/down.png",  "pipes/grey/down_1.png");
        attributes.relocate_image("pipes/orange/up.png",    "pipes/orange/up_1.png");
        attributes.relocate_image("pipes/orange/left.png",  "pipes/orange/left_1.png");
        attributes.relocate_image("pipes/orange/right.png", "pipes/orange/right_1.png");
        attributes.relocate_image("pipes/orange/down.png",  "pipes/orange/down_1.png");
        attributes.relocate_image("pipes/red/up.png",    "pipes/red/up_1.png");
        attributes.relocate_image("pipes/red/left.png",  "pipes/red/left_1.png");
        attributes.relocate_image("pipes/red/right.png", "pipes/red/right_1.png");
        attributes.relocate_image("pipes/red/down.png",  "pipes/red/down_1.png");
        attributes.relocate_image("pipes/yellow/up.png",    "pipes/yellow/up_1.png");
        attributes.relocate_image("pipes/yellow/left.png",  "pipes/yellow/left_1.png");
        attributes.relocate_image("pipes/yellow/right.png", "pipes/yellow/right_1.png");
        attributes.relocate_image("pipes/yellow/down.png",  "pipes/yellow/down_1.png");
    }
    // V2.0.0-beta6 and lower: green small pipes were replaced with grey small pipes.
    // Thus any image with a green/small directory must be replaced with a grey/small directory.
    if (engine_version < 44) {
        attributes.relocate_image("pipes/green/small/up.png",    "pipes/grey/small/up.png");
        attributes.relocate_image("pipes/green/small/left.png",  "pipes/grey/small/left.png");
        attributes.relocate_image("pipes/green/small/right.png", "pipes/grey/small/right.png");
        attributes.relocate_image("pipes/green/small/down.png", "pipes/grey/small/down.png");
        attributes.relocate_image("pipes/green/small/ver.png",   "pipes/grey/small/ver.png");
        attributes.relocate_image("pipes/green/small/hor.png",   "pipes/grey/small/hor.png");
    }
    // V2.0.0-beta7 and lower: for statue background objects, fireplant.png was renamed to fireberry.png,
    // and mushroom.png was renamed to berry.png
    if (engine_version < 45) {
        attributes.relocate_image("ground/castle_1/statue/objects/fireplant.png", "ground/castle_1/statue/objects/fireberry.png");
        attributes.relocate_image("ground/castle_1/statue/objects/mushroom.png",  "ground/castle_1/statue/objects/berry.png");
    }
    // V2.0.0-beta7 and lower: the star was replaced with a lemon
    if (engine_version < 46) {
        attributes.relocate_image("game/items/star.png", "game/items/lemon_1.png");
    }

    // always: fix sprite with undefined massive-type
    if (attributes.count("type") > 0 && attributes["type"] == "undefined") {
        cerr << "Warning: Fixing type 'undefined' by forcing it to 'passive'" << endl;
        attributes["type"] = "passive"; // So it doesn’t hinder gameplay
    }

    cStaticActor* p_sprite = new cStaticActor(attributes, level);

    /* Obsolete, cStaticActor() constructor takes care of a dummy image
    // If image not available display its filename
    if (!p_sprite->m_start_image) {
        std::string text = attributes["image"];
        if (text.empty())
            text = "Invalid image here";

        cGL_Surface* p_text_image = pFont->Render_Text(pFont->m_font_small, text);
        p_text_image->m_path = utf8_to_path(text);
        p_sprite->Set_Image(p_text_image, true, true);
        p_sprite->Set_Massive_Type(MASS_PASSIVE); // It shouldn't hinder gameplay
        p_sprite->Set_Active(false); // Only display it in the editor
        } */

    cResource_Manager& resource_manager = gp_app->Get_ResourceManager();

    // If V1.2 and lower: change pipe position
    if (engine_version < 22) {
            if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/green/up.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/green/ver.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/green/down.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/blue/up.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/blue/ver.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/blue/down.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/yellow/up.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/yellow/ver.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/yellow/down.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/grey/up.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/grey/ver.png")) == 0 ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/grey/down.png")) == 0) {
                p_sprite->move(-6, 0);
                p_sprite->m_start_pos.x = p_sprite->getPosition().x;
            }
            else if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/green/right.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/green/hor.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/green/left.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/blue/right.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/blue/hor.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/blue/left.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/yellow/right.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/yellow/hor.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/yellow/left.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/grey/right.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/grey/hor.png")) == 0 ||
                     p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("pipes/grey/left.png")) == 0) {
                p_sprite->move(0, -6);
                p_sprite->m_start_pos.y = p_sprite->getPosition().y;
            }
        } // engine_version < 22

        // If V1.2.x and lower: change some hill positions
        if (engine_version < 23) {
            if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("hills/green_1/head.png")) ||
                    p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("hills/light_blue_1/head.png")) == 0) {
                p_sprite->move(0, -6);
                p_sprite->m_start_pos.y = p_sprite->getPosition().y;
            }
        } // engine_version < 23

        // If V1.7 and lower: change yoshi_1 hill_up to jungle_1 slider image paths
        if (engine_version < 31) {
            // This is one of the XML tags that explodes into multiple sprites.
            // image filename is already changed but we need to add the middle and right tiles
            if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/jungle_1/slider/2_green_left.png")) == 0 ||
                 p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/jungle_1/slider/2_blue_left.png")) == 0 ||
                 p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/jungle_1/slider/2_brown_left.png")) == 0) {
                std::string color;
                // green
                if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/jungle_1/slider/2_green_left.png")) == 0)
                    color = "green";
                // blue
                else if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/jungle_1/slider/2_blue_left.png")) == 0)
                    color = "blue";
                // brown
                else
                    color = "brown";

                // OLD cStaticActor* p_copy = p_sprite;
                // OLD
                // OLD // add middle tiles
                // OLD for (unsigned int i = 0; i < 4; i++) {
                // OLD     p_copy = p_copy->Copy();
                // OLD     p_copy->Set_Texture(utf8_to_path("ground/jungle_1/slider/2_" + color + "_middle.png"));
                // OLD     p_copy->Set_Pos_X(p_copy->m_start_pos.x + 22, 1);
                // OLD     result.push_back(p_copy);
                // OLD }
                // OLD 
                // OLD // add end tile
                // OLD p_copy = p_copy->Copy();
                // OLD p_copy->Set_Texture(utf8_to_path("ground/jungle_1/slider/2_" + color + "_right"));
                // OLD p_copy->Set_Pos_X(p_copy->m_start_pos.x + 22, 1);
                // OLD result.push_back(p_copy);
            }
        } // engine_version < 31

        // If V1.7 and lower: change slider grey_1 to green_1 brown slider image paths
        if (engine_version < 32) {
            // image filename is already changed but we need to add an additional middle tile for left and right
            if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/green_1/slider/1/brown/left.png")) == 0 ||
                p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/green_1/slider/1/brown/right.png")) == 0) {
                // add middle tile
                // OLD cStaticActor* p_copy = p_sprite->Copy();
                // OLD p_copy->Set_Texture(utf8_to_path("ground/green_1/slider/1/brown/middle.png"));
                // OLD 
                // OLD // if from left tile it must be moved
                // OLD if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/green_1/slider/1/brown/left.png")) == 0)
                // OLD     p_copy->Set_Pos_X(p_copy->m_start_pos.x + 18, 1);
                // OLD 
                // OLD result.push_back(p_copy);
            }
            // move right tile
            if (p_sprite->Get_Texture()->m_path.compare(resource_manager.Get_Game_Pixmap("ground/green_1/slider/1/brown/right.png")) == 0) {
                p_sprite->move(18, 0);
                p_sprite->m_start_pos.x = p_sprite->getPosition().x;
            }
        } // engine_version < 32

    result.push_back(p_sprite);
    return result;
}

std::vector<cActor*> cLevelLoader::Create_Enemy_Stoppers_From_XML_Tag(const std::string& name, XmlAttributes& attributes, cLevel& level, int engine_version)
{
    std::vector<cActor*> result;

    // If V1.9 and lower: Move Y coordinate bottom to 0
    if (engine_version < 35 && attributes.count("posy") > 0)
        attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);

    result.push_back(new cEnemyStopper(attributes, level));
    return result;
}

// OLD std::vector<cSprite*> cLevelLoader::Create_Level_Exits_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // If V1.9 and lower: Move Y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.count("posy") > 0)
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     // If V1.9.x and lower: change "motion" to "camera_motion"
// OLD     if (engine_version < 36 && attributes.count("motion") > 0) {
// OLD         attributes["camera_motion"] = int_to_string(string_to_int(attributes["motion"]) + 1);
// OLD         attributes.erase("motion");
// OLD     }
// OLD 
// OLD     result.push_back(new cLevel_Exit(attributes, p_sprite_manager));
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Level_Entries_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // If V1.9 and lower: Move Y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.count("posy") > 0)
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     result.push_back(new cLevel_Entry(attributes, p_sprite_manager));
// OLD     return result;
// OLD }
// OLD 
std::vector<cActor*> cLevelLoader::Create_Boxes_From_XML_Tag(const std::string& name, XmlAttributes& attributes, cLevel& level, int engine_version)
{
    std::vector<cActor*> result;

    // If V1.9 and lower: Move Y coordinate bottom to 0
    if (engine_version < 35 && attributes.exists("posy"))
        attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);

    if (attributes["type"] == "bonus")
        result.push_back(new cBonusBox(attributes, level));
    else if (attributes["type"] == "gold") { // `gold' is somewhere pre V0.99.5
        // Update old values
        attributes["type"]      = "bonus";
        attributes["animation"] = "Default";
        attributes["item"]      = int_to_string(TYPE_GOLDPIECE);

        // Renamed old values
        if (attributes.exists("color")) {
            attributes["gold_color"] = attributes["color"];
            attributes.erase("color");
        }

        result.push_back(new cBonusBox(attributes, level));
    }
    else if (attributes["type"] == "spin")
        result.push_back(new cSpinBox(attributes, level));
    else if (attributes["type"] == "text")
        result.push_back(new cText_Box(attributes, level));
    else if (attributes["type"] == "empty") { // pre V0.99.4
        // Update old values
        attributes["type"] = "bonus";
        attributes["item"] = "0";

        result.push_back(new cBonusBox(attributes, level));
    }
    else if (attributes["type"] == "invisible") { // pre V0.99.4
        // Update old values
        attributes["type"] = "bonus";
        attributes["item"] = "0";
        attributes["invisible"] = "1";

        result.push_back(new cBonusBox(attributes, level));
    }
    else // if attributes["type"] == X
        cerr << "Warning: Unknown level box type: " << attributes["type"] << endl;

    return result;
}

// OLD std::vector<cSprite*> cLevelLoader::Create_Items_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // if V.1.9 and lower : move y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.exists("posy"))
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     std::string type = attributes["type"];
// OLD     if (type == "goldpiece")
// OLD         result.push_back(new cGoldpiece(attributes, p_sprite_manager));
// OLD     else if (type == "mushroom")
// OLD         result.push_back(new cMushroom(attributes, p_sprite_manager));
// OLD     else if (type == "fireplant")
// OLD         result.push_back(new cFirePlant(attributes, p_sprite_manager));
// OLD     else if (type == "jstar")
// OLD         result.push_back(new cjStar(attributes, p_sprite_manager));
// OLD     else if (type == "moon")
// OLD         result.push_back(new cMoon(attributes, p_sprite_manager));
// OLD     else // type == "X"
// OLD         cerr << "Warning: Unknown level item type '" << type << "'" << endl;
// OLD 
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Moving_Platforms_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // if V.1.7 and lower : change slider grey_1 to green_1 brown slider image paths
// OLD     if (engine_version < 32) {
// OLD         attributes.relocate_image("slider/grey_1/slider_left.png", "ground/green_1/slider/1/brown/left.png", "image_top_left");
// OLD         attributes.relocate_image("slider/grey_1/slider_middle.png", "ground/green_1/slider/1/brown/middle.png", "image_top_middle");
// OLD         attributes.relocate_image("slider/grey_1/slider_right.png", "ground/green_1/slider/1/brown/right.png", "image_top_right");
// OLD     }
// OLD     // if V.1.9 and lower : move y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.exists("posy"))
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     cMoving_Platform* p_moving_platform = new cMoving_Platform(attributes, p_sprite_manager);
// OLD 
// OLD     // if V.1.7 and lower : change new slider middle count because start and end image is now half the width
// OLD     if (engine_version < 32) {
// OLD         if ( !(p_moving_platform->m_left_filename.empty() && p_moving_platform->m_middle_filename.empty() && p_moving_platform->m_right_filename.empty())) {
// OLD             if (p_moving_platform->m_left_filename.compare(pResource_Manager->Get_Game_Pixmap("ground/green_1/slider/1/brown/left.png")) == 0)
// OLD                 p_moving_platform->Set_Middle_Count(p_moving_platform->m_middle_count + 1);
// OLD             else if (p_moving_platform->m_left_filename.compare(pResource_Manager->Get_Game_Pixmap("ground/green_1/slider/1/brown/right.png")) == 0)
// OLD                 p_moving_platform->Set_Middle_Count(p_moving_platform->m_middle_count + 1);
// OLD         }
// OLD     }
// OLD 
// OLD     result.push_back(p_moving_platform);
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Falling_Platforms_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // it's not moving
// OLD     attributes["speed"] = "0";
// OLD 
// OLD     // renamed time_fall to touch_time and change to the new value
// OLD     if (attributes.exists("time_fall")) {
// OLD         attributes["touch_time"] = float_to_string(string_to_float(attributes["time_fall"]) * speedfactor_fps);
// OLD         attributes.erase("time_fall");
// OLD     }
// OLD     else
// OLD         attributes["touch_time"] = "48";
// OLD 
// OLD     // enable falling
// OLD     attributes["shake_time"] = "12";
// OLD 
// OLD     // if V.1.7 and lower : change slider grey_1 to green_1 brown slider image paths
// OLD     if (engine_version < 32) {
// OLD         attributes.relocate_image("slider/grey_1/slider_left.png", "ground/green_1/slider/1/brown/left.png", "image_top_left");
// OLD         attributes.relocate_image("slider/grey_1/slider_middle.png", "ground/green_1/slider/1/brown/middle.png", "image_top_middle");
// OLD         attributes.relocate_image("slider/grey_1/slider_right.png", "ground/green_1/slider/1/brown/right.png", "image_top_right");
// OLD     }
// OLD 
// OLD     // if V.1.9 and lower : move y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.exists("posy"))
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     cMoving_Platform* p_moving_platform = new cMoving_Platform(attributes, p_sprite_manager);
// OLD 
// OLD     // if V.1.7 and lower : change new slider middle count because start and end image is now half the width
// OLD     if (engine_version < 32) {
// OLD         if (p_moving_platform->m_left_filename.compare(pResource_Manager->Get_Game_Pixmap("ground/green_1/slider/1/brown/left.png")) == 0)
// OLD             p_moving_platform->Set_Middle_Count(p_moving_platform->m_middle_count + 1);
// OLD         else if (p_moving_platform->m_left_filename.compare(pResource_Manager->Get_Game_Pixmap("ground/green_1/slider/1/brown/right.png")) == 0)
// OLD             p_moving_platform->Set_Middle_Count(p_moving_platform->m_middle_count + 1);
// OLD     }
// OLD 
// OLD     result.push_back(p_moving_platform);
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Paths_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // if V.1.9 and lower : move y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.exists("posy"))
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     result.push_back(new cPath(attributes, p_sprite_manager));
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Enemies_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD     std::string type = attributes["type"];
// OLD 
// OLD     // if V.1.5 and lower
// OLD     if (engine_version < 26) {
// OLD         // change gumba to furball
// OLD         if (type == "gumba") {
// OLD             // change type
// OLD             type = "furball";
// OLD             attributes["type"] = "furball";
// OLD             // fix color : red was used in pre 1.0 but later became blue
// OLD             if (attributes.exists("color") && attributes["color"] == "red")
// OLD                 attributes["color"] = "blue";
// OLD         }
// OLD 
// OLD         // change rex to krush
// OLD         else if (type == "rex") {
// OLD             // change type
// OLD             type = "krush";
// OLD             attributes["type"] = "krush";
// OLD         }
// OLD     } // engine_version < 26
// OLD 
// OLD     // if V.1.7 and lower
// OLD     if (engine_version < 29) {
// OLD         if (type == "jpiranha") {
// OLD             // change type
// OLD             type = "flyon";
// OLD             attributes["type"] = "flyon";
// OLD 
// OLD             // change image dir
// OLD             if (attributes.exists("image_dir")) {
// OLD                 std::string img_dir = attributes["image_dir"];
// OLD                 std::string::size_type pos = img_dir.find("jpiranha");
// OLD 
// OLD                 // change if found
// OLD                 if (pos != std::string::npos) {
// OLD                     img_dir.replace(pos, 8, "flyon");
// OLD                     attributes["image_dir"] = img_dir;
// OLD                 }
// OLD             }
// OLD         }
// OLD     } // engine_version < 29
// OLD 
// OLD     // if V.1.9 and lower : move y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.exists("posy"))
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     // Now for the real enemy loading after all the backward compatibility stuff
// OLD     if (type == "eato")
// OLD         result.push_back(new cEato(attributes, p_sprite_manager));
// OLD     else if (type == "furball")
// OLD         result.push_back(new cFurball(attributes, p_sprite_manager));
// OLD     else if (type == "turtle" || type == "army" )
// OLD         result.push_back(new cArmy(attributes, p_sprite_manager));
// OLD     else if (type == "shell")
// OLD         result.push_back(new cShell(attributes, p_sprite_manager));
// OLD     else if (type == "turtleboss") {
// OLD         // if V.1.5 and lower : max_downgrade_time changed to shell_time
// OLD         if (engine_version < 27 && attributes.exists("max_downgrade_time")) {
// OLD             attributes["shell_time"] = attributes["max_downgrade_time"];
// OLD             attributes.erase("max_downgrade_time");
// OLD         }
// OLD         result.push_back(new cTurtleBoss(attributes, p_sprite_manager));
// OLD     }
// OLD     else if (type == "flyon")
// OLD         result.push_back(new cFlyon(attributes, p_sprite_manager));
// OLD     else if (type == "thromp") {
// OLD         cThromp* p_thromp = new cThromp(attributes, p_sprite_manager);
// OLD 
// OLD         // if V.1.4 and lower : fix thromp distance was smaller
// OLD         if (engine_version < 25)
// OLD             p_thromp->Set_Max_Distance(p_thromp->m_max_distance + 36);
// OLD 
// OLD         result.push_back(p_thromp);
// OLD     }
// OLD     else if (type == "rokko")
// OLD         result.push_back(new cRokko(attributes, p_sprite_manager));
// OLD     else if (type == "krush")
// OLD         result.push_back(new cKrush(attributes, p_sprite_manager));
// OLD     else if (type == "gee")
// OLD         result.push_back(new cGee(attributes, p_sprite_manager));
// OLD     else if (type == "spika")
// OLD         result.push_back(new cSpika(attributes, p_sprite_manager));
// OLD     else if (type == "static")
// OLD         result.push_back(new cStaticEnemy(attributes, p_sprite_manager));
// OLD     else if (type == "spikeball")
// OLD         result.push_back(new cSpikeball(attributes, p_sprite_manager));
// OLD     else if (type == "pip")
// OLD         result.push_back(new cPip(attributes, p_sprite_manager));
// OLD     else if (type == "beetle_barrage")
// OLD         result.push_back(new cBeetleBarrage(attributes, p_sprite_manager));
// OLD     else if (type == "beetle")
// OLD         result.push_back(new cBeetle(attributes, p_sprite_manager));
// OLD     else if (type == "larry")
// OLD         result.push_back(new cLarry(attributes, p_sprite_manager));
// OLD     else // type == "X"
// OLD         cerr << "Warning: Unknown level enemy type: " << type << endl;
// OLD 
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Sounds_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // if V.1.9 and lower : move y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.exists("posy"))
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     result.push_back(new cRandom_Sound(attributes, p_sprite_manager));
// OLD 
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Particle_Emitters_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // Note : If you relocate images don't forget the global effect
// OLD 
// OLD     //Fix old emitter definitions
// OLD     if (attributes.exists("pos_x"))
// OLD         attributes["posx"] =  attributes["pos_x"];
// OLD     if (attributes.exists("pos_y"))
// OLD         attributes["posy"] =  attributes["pos_y"];
// OLD     if (attributes.exists("size_x"))
// OLD         attributes["sizex"] =  attributes["size_x"];
// OLD     if (attributes.exists("size_y"))
// OLD         attributes["sizey"] =  attributes["size_y"];
// OLD 
// OLD     // if V.1.9 and lower : move y coordinate bottom to 0
// OLD     if (engine_version < 35 && attributes.exists("posy"))
// OLD         attributes["posy"] = float_to_string(string_to_float(attributes["posy"]) - 600.0f);
// OLD 
// OLD     // if V.1.9 and lower : change fire_1 animation to particles
// OLD     if (engine_version < 37) {
// OLD         attributes.relocate_image("animation/fire_1/1.png", "animation/particles/fire_1.png", "file");
// OLD         attributes.relocate_image("animation/fire_1/2.png", "animation/particles/fire_2.png", "file");
// OLD         attributes.relocate_image("animation/fire_1/3.png", "animation/particles/fire_3.png", "file");
// OLD         attributes.relocate_image("animation/fire_1/4.png", "animation/particles/fire_4.png", "file");
// OLD     }
// OLD 
// OLD     // if V.1.9 and lower : change file to image
// OLD     if (engine_version < 38 && attributes.exists("file")) {
// OLD         attributes["image"] = attributes["file"];
// OLD         attributes.erase("file");
// OLD     }
// OLD 
// OLD     if (attributes.exists("image") && !attributes.exists("particle_image")) {
// OLD         attributes["particle_image"] = attributes["image"] ;
// OLD         attributes.erase("image");
// OLD     }
// OLD 
// OLD     cParticle_Emitter* p_emitter = new cParticle_Emitter(attributes, p_sprite_manager);
// OLD     // set to not spawned
// OLD     p_emitter->Set_Spawned(false);
// OLD 
// OLD     result.push_back(p_emitter);
// OLD 
// OLD     return result;
// OLD }
// OLD 
// OLD // if V.1.9 and lower : convert global effect to an advanced particle emitter
// OLD std::vector<cSprite*> cLevelLoader::Create_Global_Effects_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD 
// OLD     // if V.0.99.4 and lower : change lieftime mod to time to live
// OLD     if (engine_version < 21 && !attributes.exists("time_to_live")) {
// OLD         attributes["time_to_live"] = float_to_string(attributes.fetch<float>("lifetime_mod", 20.0f) * 0.3f);
// OLD         attributes.erase("lifetime_mod");
// OLD     }
// OLD 
// OLD     // if V.0.99.7 and lower : change creation speed to emitter iteration interval
// OLD     if (engine_version < 22 && !attributes.exists("emitter_iteration_interval")) {
// OLD         attributes["emitter_iteration_interval"] = float_to_string((1.0f / attributes.fetch<float>("creation_speed", 0.3f)) * 0.032f);
// OLD         attributes.erase("creation_speed");
// OLD     }
// OLD 
// OLD     if (attributes.exists("image") && !attributes.exists("particle_image")) {
// OLD         attributes["particle_image"] = attributes["image"] ;
// OLD         attributes.erase("image");
// OLD     }
// OLD 
// OLD     // if V.1.9 and lower : change fire_1 animation to particles
// OLD     if (engine_version < 37) {
// OLD         attributes.relocate_image("animation/fire_1/1.png", "animation/particles/fire_1.png", "particle_image");
// OLD         attributes.relocate_image("animation/fire_1/2.png", "animation/particles/fire_2.png", "particle_image");
// OLD         attributes.relocate_image("animation/fire_1/3.png", "animation/particles/fire_3.png", "particle_image");
// OLD         attributes.relocate_image("animation/fire_1/4.png", "animation/particles/fire_4.png", "particle_image");
// OLD     }
// OLD 
// OLD     // change disabled type to quota 0
// OLD     if (attributes.exists("type") && attributes["type"] == "0") {
// OLD         attributes["quota"] = "0";
// OLD         attributes.erase("type");
// OLD     }
// OLD 
// OLD     // rename attributes
// OLD     attributes["posx"]         = int_to_string(attributes.fetch<int>("rect_x", 0));
// OLD     attributes["posy"]         = int_to_string(attributes.fetch<int>("rect_y", 0) - 600);
// OLD     attributes["sizex"]        = int_to_string(attributes.fetch<int>("rect_w", game_res_w));
// OLD     attributes["sizey"]        = int_to_string(attributes.fetch<int>("rect_h", 0));
// OLD     attributes["pos_z"]         = float_to_string(attributes.fetch<float>("z", 0.12f));
// OLD     attributes["pos_z_rand"]    = float_to_string(attributes.fetch<float>("z_rand", 0.0f));
// OLD     attributes["emitter_time_to_live"] = "-1.0";
// OLD     if (!attributes.exists("time_to_live"))
// OLD         attributes["time_to_live"] = "0.7";
// OLD     attributes["emitter_interval"]  = float_to_string(attributes.fetch<float>("emitter_iteration_interval", 0.3f));
// OLD     attributes["size_scale"]        = float_to_string(attributes.fetch<float>("scale", 0.2f));
// OLD     attributes["size_scale_rand"]   = float_to_string(attributes.fetch<float>("scale_rand", 0.2f));
// OLD     attributes["vel"]               = float_to_string(attributes.fetch<float>("speed", 0.2f));
// OLD     attributes["vel_rand"]          = float_to_string(attributes.fetch<float>("speed_rand", 8.0f));
// OLD     attributes["angle_start"]       = float_to_string(attributes.fetch<float>("dir_range_start", 0.0f));
// OLD     attributes["angle_range"]       = float_to_string(attributes.fetch<float>("dir_range_size", 90.0f));
// OLD     attributes["const_rot_z"]       = float_to_string(attributes.fetch<float>("const_rotz", -5.0f));
// OLD     attributes["const_rot_z_rand"]  = float_to_string(attributes.fetch<float>("const_rotz_rand", 10.0f));
// OLD 
// OLD     cParticle_Emitter* p_emitter = new cParticle_Emitter(attributes, p_sprite_manager);
// OLD     p_emitter->Set_Spawned(false);
// OLD 
// OLD     // clip to the camera
// OLD     p_emitter->Set_Clip_Rect(GL_rect(0.0f, 0.0f, static_cast<float>(game_res_w), static_cast<float>(game_res_h) + (attributes.fetch<int>("rect_y", 0) * -1)));
// OLD     p_emitter->Set_Based_On_Camera_Pos(1);
// OLD 
// OLD     result.push_back(p_emitter);
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Balls_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD     result.push_back(new cBall(attributes, p_sprite_manager));
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Lavas_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD     result.push_back(new cLava(attributes, p_sprite_manager));
// OLD     return result;
// OLD }
// OLD 
// OLD std::vector<cSprite*> cLevelLoader::Create_Crates_From_XML_Tag(const std::string& name, XmlAttributes& attributes, int engine_version, cSprite_Manager* p_sprite_manager)
// OLD {
// OLD     std::vector<cSprite*> result;
// OLD     result.push_back(new cCrate(attributes, p_sprite_manager));
// OLD     return result;
// OLD }

/***************************************************************************
 * box.cpp  -  class for the basic box handler
 *
 * Copyright © 2003 - 2011 Florian Richter
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
#include "../core/property_helper.hpp"
#include "../core/bintree.hpp"
#include "../core/i18n.hpp"
#include "../scripting/scriptable_object.hpp"
#include "../core/file_parser.hpp"
#include "../video/img_set.hpp"
#include "../video/img_settings.hpp"
#include "../video/img_manager.hpp"
#include "../video/color.hpp"
#include "../level/level.hpp"
#include "actor.hpp"
#include "sprite_actor.hpp"
#include "animated_actor.hpp"
#include "../core/collision.hpp"
#include "../core/errors.hpp"
#include "../core/xml_attributes.hpp"
#include "../level/level_player.hpp"
#include "../core/scene_manager.hpp"
#include "../core/tsc_app.hpp"
#include "box.hpp"

using namespace std;
using namespace TSC;

/* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

cBaseBox::cBaseBox()
    : cAnimatedActor()
{
    Init();
}

cBaseBox::cBaseBox(XmlAttributes& attributes, cLevel& level, const std::string type_name)
    : cAnimatedActor(attributes, level, type_name)
{
    Init();

    // Invisible
    Set_Box_Invisible(static_cast<Box_Invisible_Type>(string_to_float(attributes["invisible"])));

    // Usable count
    Set_Useable_Count(attributes.fetch<int>("useable_count", m_start_useable_count), true);
}

void cBaseBox::Init()
{
    Set_Collision_Type(COLTYPE_MASSIVE);

    m_name = _("Box");
    m_can_be_ground = 1;

    box_type = TYPE_UNDEFINED;
    // OLD m_item_image = NULL;
    m_pos_z = ZLAYER_CLIMBABLE + 0.005;

    m_move_col_dir = DIR_UNDEFINED;
    m_move_counter = 0.0f;
    m_move_back = 0;
    // default = usable once
    m_useable_count = 1;
    m_start_useable_count = 1;

    m_box_invisible = BOX_VISIBLE;

    m_particle_counter_active = 0.0f;

    Set_Animation_Type("Default");
}

cBaseBox::~cBaseBox(void)
{
    //
}

// OLD std::string cBaseBox::Get_XML_Type_Name()
// OLD {
// OLD     // return is just as good as break here
// OLD     switch (box_type) {
// OLD     case TYPE_SPIN_BOX:
// OLD         return "spin";
// OLD     case TYPE_TEXT_BOX:
// OLD         return "text";
// OLD     default:
// OLD         return "bonus";
// OLD     }
// OLD }

// OLD xmlpp::Element* cBaseBox::Save_To_XML_Node(xmlpp::Element* p_element)
// OLD {
// OLD     xmlpp::Element* p_node = cMovingSprite::Save_To_XML_Node(p_element);
// OLD 
// OLD     if (box_type != TYPE_SPIN_BOX && box_type != TYPE_TEXT_BOX) {
// OLD         // animation type
// OLD         Add_Property(p_node, "animation", m_anim_type);
// OLD         // best possible item
// OLD         Add_Property(p_node, "item", box_type);
// OLD     }
// OLD 
// OLD     // invisible
// OLD     Add_Property(p_node, "invisible", m_box_invisible);
// OLD     // useable count
// OLD     Add_Property(p_node, "useable_count", m_start_useable_count);
// OLD 
// OLD     return p_node;
// OLD }
// OLD 
// OLD void cBaseBox::Load_From_Savegame(cSave_Level_Object* save_object)
// OLD {
// OLD     // useable count
// OLD     int save_useable_count = string_to_int(save_object->Get_Value("useable_count"));
// OLD     Set_Useable_Count(save_useable_count);
// OLD }
// OLD 
// OLD bool cBaseBox::Save_To_Savegame_XML_Node(xmlpp::Element* p_element) const
// OLD {
// OLD     cMovingSprite::Save_To_Savegame_XML_Node(p_element);
// OLD 
// OLD     Add_Property(p_element, "useable_count", int_to_string(m_useable_count));
// OLD 
// OLD     return true;
// OLD }

void cBaseBox::Set_Animation_Type(const std::string& new_anim_type)
{
    // already set
    if (m_anim_type.compare(new_anim_type) == 0) {
        return;
    }

    Clear_Images();
    m_anim_type = new_anim_type;

    if (m_anim_type.compare("Bonus") == 0) {
        // disabled image
        Add_Image_Set("disabled", "game/box/disabled.imgset");
        // animation images
        Add_Image_Set("main", "game/box/yellow/bonus/bonus.imgset");

        Set_Image_Set("main");
    }
    else if (m_anim_type.compare("Default") == 0) {
        // disabled image
        Add_Image_Set("disabled", "game/box/disabled.imgset");
        // default
        Add_Image_Set("main", "game/box/yellow/default.imgset");

        Set_Image_Set("main");
    }
    else if (m_anim_type.compare("Power") == 0) {
        // disabled image
        Add_Image_Set("disabled", "game/box/disabled.imgset");
        // animation images
        Add_Image_Set("main", "game/box/yellow/power.imgset");

        Set_Image_Set("main");
    }
    else if (m_anim_type.compare("Spin") == 0) {
        // disabled image
        Add_Image_Set("disabled", "game/box/yellow/spin/disabled.imgset");
        // animation images
        Add_Image_Set("main", "game/box/yellow/spin/still.imgset");
        Add_Image_Set("spin", "game/box/yellow/spin/spin.imgset");

        Set_Image_Set("main");
    }
    else {
        cerr << "Warning : Unknown Box Animation Type : " << m_anim_type << endl;
        Set_Animation_Type("Bonus");
    }
}

void cBaseBox::Set_Useable_Count(int count, bool new_startcount /* = 0 */)
{
    m_useable_count = count;

    if (new_startcount) {
        m_start_useable_count = m_useable_count;

        // unlimited
        if (m_start_useable_count < -1) {
            m_start_useable_count = -1;
        }
    }
}

void cBaseBox::Set_Box_Invisible(Box_Invisible_Type type)
{
    // already set
    if (m_box_invisible == type) {
        return;
    }

    // remember old type
    Box_Invisible_Type type_old = m_box_invisible;
    // set new type
    m_box_invisible = type;

    // was invisible
    if (type_old == BOX_INVISIBLE_MASSIVE || type_old == BOX_INVISIBLE_SEMI_MASSIVE) {
        // was semi massive
        if (type_old == BOX_INVISIBLE_SEMI_MASSIVE) {
            Set_Collision_Type(COLTYPE_MASSIVE);
            Set_Invisible(false);
        }
    }
    // was ghost
    else if (type_old == BOX_GHOST) {
        // OLD Set_Color(255, 255, 255, 255);
        // OLD Set_Color_Combine(0, 0, 0, 0);
    }

    // got invisible
    if (type == BOX_INVISIBLE_MASSIVE || type == BOX_INVISIBLE_SEMI_MASSIVE) {
        // got semi massive
        if (type == BOX_INVISIBLE_SEMI_MASSIVE) {
            Set_Collision_Type(COLTYPE_PASSIVE);
            Set_Invisible(true);
        }
    }
    // got ghost
    else if (type == BOX_GHOST) {
        // OLD Set_Color(192, 192, 255, 128);
        // OLD Set_Color_Combine(0.2f, 0.2f, 0.55f, GL_ADD);
    }
}

void cBaseBox::Activate_Collision(ObjectDirection col_direction)
{
    // if already active ignore event
    if (m_move_col_dir != DIR_UNDEFINED) {
        return;
    }

    // not useable
    if (m_useable_count == 0) {
        return;
    }

    // if invisible go visible
    if (m_box_invisible) {
        // get massive
        if (m_box_invisible == BOX_INVISIBLE_SEMI_MASSIVE) {
            Set_Collision_Type(COLTYPE_MASSIVE);
            Set_Invisible(false);
        }
    }

    // set scaling based on direction
    if (col_direction == DIR_UP) {
        // OLD Set_Scale_Directions(1, 0, 1, 1);
    }
    else if (col_direction == DIR_DOWN) {
        // OLD Set_Scale_Directions(0, 1, 1, 1);
    }
    else if (col_direction == DIR_LEFT) {
        // OLD Set_Scale_Directions(1, 0, 1, 0);
    }
    else if (col_direction == DIR_RIGHT) {
        // OLD Set_Scale_Directions(1, 0, 0, 1);
    }

    // set collision direction
    m_move_col_dir = col_direction;
    Update_Valid_Update();

    Activate();
}

void cBaseBox::Update_Collision(void)
{
    // not moving
    if (m_move_col_dir == DIR_UNDEFINED) {
        return;
    }

    // invalid direction
    if (m_move_col_dir != DIR_UP && m_move_col_dir != DIR_DOWN && m_move_col_dir != DIR_RIGHT && m_move_col_dir != DIR_LEFT) {
        cerr << "Warning : wrong box collision direction " << m_move_col_dir << endl;
        m_move_col_dir = DIR_UNDEFINED;
        Update_Valid_Update();
        return;
    }

    // speed mod
    float mod = gp_app->Get_SceneManager().Get_Speedfactor() * 0.05f;
    // counter
    m_move_counter += gp_app->Get_SceneManager().Get_Speedfactor() * 0.2f;

    // move into the given direction
    if (!m_move_back) {
        // scale
        // OLD Add_Scale(mod);

        // Particles
        // OLD Generate_Activation_Particles();

        // check if reached final position
        if (m_move_counter > 1.0f) {
            m_move_back = 1;
            m_move_counter = 0.0f;
        }
    }
    // move back to the original position
    else {
        // scale
        // OLD Add_Scale(-mod);

        // check if reached original position
        if (m_move_counter > 1.0f) {
            m_move_col_dir = DIR_UNDEFINED;
            Update_Valid_Update();
            m_move_back = 0;
            m_move_counter = 0.0f;

            // reset rect
            // OLD m_col_pos = m_image->m_col_pos;
            // reset scale
            // OLD Set_Scale(1);
            // OLD Set_Scale_Directions(1, 1, 1, 1);
            // reset position
            Set_Pos(m_start_pos.x, m_start_pos.y);
        }
    }
}

void cBaseBox::Activate(void)
{
    if (m_useable_count > 0 || m_useable_count == -1) {
        // OLD Scripting::cActivate_Event evt;
        // OLD evt.Fire(pActive_Level->m_mruby, this);
    }

    // set useable count
    if (m_useable_count > 0) {
        Set_Useable_Count(m_useable_count - 1);
    }
}

void cBaseBox::Update(void)
{
    cAnimatedActor::Update();

    // animate only a visible box or an activated invisible box
    if (m_box_invisible == BOX_VISIBLE || (m_box_invisible == BOX_GHOST && gp_current_level->Get_Player()->m_alex_type == ALEX_GHOST) || m_useable_count != m_start_useable_count) {
        // Set_Image in Update_Animation overwrites col_pos
        // OLD GL_point col_pos_temp = m_col_pos;

        Update_Animation();

        // save col_pos
        // OLD m_col_pos = col_pos_temp;
    }

    Update_Collision();
}

void cBaseBox::Draw(sf::RenderWindow& stage) const
{
    // cSpriteActor::Draw() takes care of not drawing m_invisible actors.
    cAnimatedActor::Draw(stage);

    // editor disabled
    // OLD if (!editor_level_enabled) {
    // OLD }
    // OLD // editor enabled
    // OLD else {
    // OLD     // draw invisible box only in editor mode
    // OLD     if (m_box_invisible) {
    // OLD         Color color;
    // OLD 
    // OLD         // default invisible
    // OLD         if (m_box_invisible == BOX_INVISIBLE_MASSIVE) {
    // OLD             color = Color(static_cast<uint8_t>(240), 0, 30, 128);
    // OLD         }
    // OLD         // ghost
    // OLD         else if (m_box_invisible == BOX_GHOST) {
    // OLD             color = Color(static_cast<uint8_t>(20), 20, 150, 128);
    // OLD         }
    // OLD         // invisible semi massive
    // OLD         else if (m_box_invisible == BOX_INVISIBLE_SEMI_MASSIVE) {
    // OLD             color = Color(static_cast<uint8_t>(180), 0, 10, 128);
    // OLD         }
    // OLD 
    // OLD         pVideo->Draw_Rect(m_start_pos_x - pActive_Camera->m_x, m_start_pos_y - pActive_Camera->m_y, m_rect.m_w, m_rect.m_h, m_pos_z, &color);
    // OLD     }
    // OLD     // visible box
    // OLD     else {
    // OLD         cMovingSprite::Draw(request);
    // OLD     }
    // OLD 
    // OLD     // draw item image
    // OLD     if (m_item_image) {
    // OLD         // auto position
    // OLD         m_item_image->Blit(m_start_pos_x - ((m_item_image->m_w - m_rect.m_w) / 2) - pActive_Camera->m_x, m_start_pos_y - ((m_item_image->m_h - m_rect.m_h) / 2) - pActive_Camera->m_y, m_pos_z + 0.000003f);
    // OLD     }
    // OLD }
}

void cBaseBox::Generate_Activation_Particles(void)
{
    // OLD // no default/unimportant boxes
    // OLD if (m_type == TYPE_SPIN_BOX || m_type == TYPE_TEXT_BOX || box_type == TYPE_GOLDPIECE) {
    // OLD     return;
    // OLD }
    // OLD 
    // OLD m_particle_counter_active += pFramerate->m_speed_factor;

    if (m_particle_counter_active > 0.0f) {
        // OLD cParticle_Emitter* anim = new cParticle_Emitter(m_sprite_manager);
        // OLD if (box_type == TYPE_MUSHROOM_POISON) {
        // OLD     anim->Set_Image(pVideo->Get_Package_Surface("animation/particles/green_ghost_light.png"));
        // OLD     anim->Set_Speed(1.0f, 0.1f);
        // OLD     anim->Set_Time_to_Live(1.4f);
        // OLD }
        // OLD else {
        // OLD     anim->Set_Image(pVideo->Get_Package_Surface("animation/particles/light.png"));
        // OLD     anim->Set_Speed(7.0f, 0.5f);
        // OLD     anim->Set_Time_to_Live(0.2f);
        // OLD }
        // OLD anim->Set_Quota(static_cast<int>(m_particle_counter_active));
        // OLD anim->Set_Emitter_Rect(m_pos_x + 4.0f, m_pos_y + 8.0f, m_rect.m_w - 8.0f, 1.0f);
        // OLD if (box_type == TYPE_UNDEFINED) {
        // OLD     anim->Set_Emitter_Time_to_Live(0.2f);
        // OLD     anim->Set_Scale(0.2f, 0.05f);
        // OLD }
        // OLD else {
        // OLD     anim->Set_Emitter_Time_to_Live(0.5f);
        // OLD     anim->Set_Scale(0.3f, 0.1f);
        // OLD }
        // OLD anim->Set_Pos_Z(m_pos_z - 0.000001f);
        // OLD anim->Set_Direction_Range(260, 20);

        Color color;
        Color color_rand;

        if (box_type == TYPE_UNDEFINED) {
            color = Color(static_cast<uint8_t>(128), 128, 128, 128);
            color_rand = Color(static_cast<uint8_t>(0), 0, 0, 128);
        }
        else if (box_type == TYPE_MUSHROOM_DEFAULT) {
            color = Color(static_cast<uint8_t>(180), 140, 120);
            color_rand = Color(static_cast<uint8_t>(70), 30, 30, 0);
        }
        else if (box_type == TYPE_FIREPLANT) {
            color = Color(static_cast<uint8_t>(220), 150, 70);
            color_rand = Color(static_cast<uint8_t>(30), 10, 10, 0);
        }
        else if (box_type == TYPE_MUSHROOM_BLUE) {
            color = Color(static_cast<uint8_t>(180), 180, 240);
            color_rand = Color(static_cast<uint8_t>(40), 40, 10, 0);
        }
        else if (box_type == TYPE_MUSHROOM_GHOST) {
            color = Color(static_cast<uint8_t>(100), 100, 100);
            color_rand = Color(static_cast<uint8_t>(10), 10, 10, 0);
        }
        else if (box_type == TYPE_MUSHROOM_LIVE_1) {
            color = Color(static_cast<uint8_t>(120), 200, 120);
            color_rand = Color(static_cast<uint8_t>(20), 50, 20, 0);
        }
        else if (box_type == TYPE_STAR) {
            color = Color(static_cast<uint8_t>(180), 150, 100);
            color_rand = Color(static_cast<uint8_t>(70), 70, 0, 0);
        }
        else if (box_type == TYPE_MUSHROOM_POISON) {
            color = Color(static_cast<uint8_t>(80), 100, 10);
            color_rand = Color(static_cast<uint8_t>(20), 90, 20, 0);
        }
        else {
            color = white;
            color_rand = Color(static_cast<uint8_t>(0), 0, 0, 0);
        }
        // OLD anim->Set_Color(color, color_rand);
        // OLD 
        // OLD anim->Set_Blending(BLEND_ADD);
        // OLD anim->Emit();
        // OLD pActive_Animation_Manager->Add(anim);

        m_particle_counter_active -= static_cast<int>(m_particle_counter_active);
    }
}

// OLD bool cBaseBox::Is_Update_Valid()
// OLD {
// OLD     // if not activateable and not animating
// OLD     if (!m_useable_count && m_move_col_dir == DIR_UNDEFINED) {
// OLD         return 0;
// OLD     }
// OLD 
// OLD     return cAnimatedActor::Is_Update_Valid();
// OLD }


bool cBaseBox::Handle_Collision_Player(cCollision* p_collision)
{
    const cLevel_Player* p_player = static_cast<cLevel_Player*>(p_collision->Get_Collision_Sufferer());

    // if player jumps from below or flies against it
    if (p_collision->Is_Collision_Bottom() && p_player->m_state != STA_FLY) {
        if (m_useable_count != 0) {
            Activate_Collision(DIR_DOWN);
        }
        else {
            // OLD if (Is_Visible_On_Screen()) {
            // OLD     pAudio->Play_Sound("wall_hit.wav", RID_ALEX_WALL_HIT);
            // OLD }
        }
    }

    return true;
}

bool cBaseBox::Handle_Collision_Enemy(cCollision* p_collision)
{
    // OLD cEnemy* enemy = static_cast<cEnemy*>(p_collision->Get_Collision_Sufferer());

    // OLD // if army
    // OLD if (enemy->m_type == TYPE_ARMY || enemy->m_type == TYPE_SHELL) {
    // OLD     cArmy* army = static_cast<cArmy*>(enemy);  // Shell is a army anyway
    // OLD 
    // OLD     // if not shell
    // OLD     if (army->m_army_state != ARMY_SHELL_RUN && army->m_army_state != ARMY_SHELL_STAND) {
    // OLD         return;
    // OLD     }
    // OLD }
    // OLD // if turtle boss
    // OLD else if (enemy->m_type == TYPE_TURTLE_BOSS) {
    // OLD     cTurtleBoss* turtleboss = static_cast<cTurtleBoss*>(enemy);
    // OLD 
    // OLD     // if not shell
    // OLD     if (turtleboss->m_turtle_state != TURTLEBOSS_SHELL_RUN && turtleboss->m_turtle_state != TURTLEBOSS_SHELL_STAND) {
    // OLD         return;
    // OLD     }
    // OLD }
    // OLD // not a valid enemy
    // OLD else {
    // OLD     return;
    // OLD }
    // OLD 
    // OLD if (collision->m_direction == DIR_RIGHT || collision->m_direction == DIR_LEFT || collision->m_direction == DIR_BOTTOM) {
    // OLD     if (m_useable_count != 0 && m_type != TYPE_TEXT_BOX) {
    // OLD         Activate_Collision(Get_Opposite_Direction(collision->m_direction));
    // OLD     }
    // OLD     else {
    // OLD         if (Is_Visible_On_Screen()) {
    // OLD             pAudio->Play_Sound("wall_hit.wav");
    // OLD         }
    // OLD     }
    // OLD }

    return true;
}


// OLD void cBaseBox::Editor_Activate(void)
// OLD {
// OLD     CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
// OLD 
// OLD     if (m_type != TYPE_TEXT_BOX) {
// OLD         // useable count
// OLD         CEGUI::Editbox* editbox = static_cast<CEGUI::Editbox*>(wmgr.createWindow("TaharezLook/Editbox", "editor_basebox_useable_count"));
// OLD         Editor_Add(UTF8_("Useable Count"), UTF8_("Useable Count"), editbox, 80);
// OLD 
// OLD         editbox->setText(int_to_string(m_start_useable_count));
// OLD         editbox->subscribeEvent(CEGUI::Editbox::EventTextChanged, CEGUI::Event::Subscriber(&cBaseBox::Editor_Useable_Count_Text_Changed, this));
// OLD     }
// OLD 
// OLD     // Invisible
// OLD     CEGUI::Combobox* combobox = static_cast<CEGUI::Combobox*>(wmgr.createWindow("TaharezLook/Combobox", "editor_basebox_invisible"));
// OLD     Editor_Add(UTF8_("Invisible"), UTF8_("Massive is invisible until activated.\nGhost is only visible and activateable if in ghost mode.\nSemi Massive is like Massive but only touchable in the activation direction."), combobox, 120, 100);
// OLD 
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Disabled")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Massive")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Ghost")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Semi Massive")));
// OLD 
// OLD     if (m_box_invisible == BOX_INVISIBLE_MASSIVE) {
// OLD         combobox->setText(UTF8_("Massive"));
// OLD     }
// OLD     else if (m_box_invisible == BOX_GHOST) {
// OLD         combobox->setText(UTF8_("Ghost"));
// OLD     }
// OLD     else if (m_box_invisible == BOX_INVISIBLE_SEMI_MASSIVE) {
// OLD         combobox->setText(UTF8_("Semi Massive"));
// OLD     }
// OLD     else {
// OLD         combobox->setText(UTF8_("Disabled"));
// OLD     }
// OLD 
// OLD     combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&cBaseBox::Editor_Invisible_Select, this));
// OLD 
// OLD     if (m_type == TYPE_SPIN_BOX) {
// OLD         // init
// OLD         Editor_Init();
// OLD     }
// OLD }
// OLD 
// OLD bool cBaseBox::Editor_Useable_Count_Text_Changed(const CEGUI::EventArgs& event)
// OLD {
// OLD     const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(event);
// OLD     std::string str_text = static_cast<CEGUI::Editbox*>(windowEventArgs.window)->getText().c_str();
// OLD 
// OLD     Set_Useable_Count(string_to_int(str_text), 1);
// OLD 
// OLD     return 1;
// OLD }
// OLD 
// OLD bool cBaseBox::Editor_Invisible_Select(const CEGUI::EventArgs& event)
// OLD {
// OLD     const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(event);
// OLD     CEGUI::ListboxItem* item = static_cast<CEGUI::Combobox*>(windowEventArgs.window)->getSelectedItem();
// OLD 
// OLD     if (item->getText().compare(UTF8_("Massive")) == 0) {
// OLD         Set_Invisible(BOX_INVISIBLE_MASSIVE);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Ghost")) == 0) {
// OLD         Set_Invisible(BOX_GHOST);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Semi Massive")) == 0) {
// OLD         Set_Invisible(BOX_INVISIBLE_SEMI_MASSIVE);
// OLD     }
// OLD     else {
// OLD         Set_Invisible(BOX_VISIBLE);
// OLD     }
// OLD 
// OLD     return 1;
// OLD }
// OLD 
// OLD std::string cBaseBox::Create_Name(void) const
// OLD {
// OLD     std::string name = m_name; // dup
// OLD 
// OLD     if (m_box_invisible == BOX_INVISIBLE_MASSIVE) {
// OLD         name += " " + std::string(_("(Invisible)"));
// OLD     }
// OLD     else if (m_box_invisible == BOX_GHOST) {
// OLD         name += " " + std::string(_("(Ghost block)"));
// OLD     }
// OLD     else if (m_box_invisible == BOX_INVISIBLE_SEMI_MASSIVE) {
// OLD         name += " " + std::string(_("(Invisible Semi Massive)"));
// OLD     }
// OLD 
// OLD     return name;
// OLD }
// OLD 
// OLD void cBaseBox::Set_Massive_Type(MassiveType type)
// OLD {
// OLD     // Ignore to prevent "m" toggling in level editor
// OLD }

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

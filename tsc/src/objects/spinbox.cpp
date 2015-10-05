/***************************************************************************
 * spinbox.cpp  -  spinning box
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
#include "spinbox.hpp"

using namespace TSC;

/* *** *** *** *** *** *** *** *** cSpinBox *** *** *** *** *** *** *** *** *** */

cSpinBox::cSpinBox()
    : cBaseBox()
{
    cSpinBox::Init();
}

cSpinBox::cSpinBox(XmlAttributes& attributes, cLevel& level, const std::string type_name)
    : cBaseBox(attributes, level, type_name)
{
    cSpinBox::Init();
}

cSpinBox::~cSpinBox(void)
{
    //
}

void cSpinBox::Init(void)
{
    // OLD m_type = TYPE_SPIN_BOX;
    box_type = TYPE_SPIN_BOX;
    m_name = _("Spinbox");
    // OLD m_camera_range = 5000;

    m_spin_counter = 0.0f;
    m_spin = 0;

    // default is infinite times activate-able
    Set_Useable_Count(-1, 1);
    // Spinbox Animation
    Set_Animation_Type("Spin");

    // editor image
    // OLD m_item_image = pVideo->Get_Package_Surface("game/arrow/small/white/up.png");
}

// OLD cSpinBox* cSpinBox::Copy(void) const
// OLD {
// OLD     cSpinBox* spinbox = new cSpinBox(m_sprite_manager);
// OLD     spinbox->Set_Pos(m_start_pos_x, m_start_pos_y, 1);
// OLD     spinbox->Set_Invisible(m_box_invisible);
// OLD     spinbox->Set_Useable_Count(m_start_useable_count, 1);
// OLD     return spinbox;
// OLD }

// OLD xmlpp::Element* cSpinBox::Save_To_XML_Node(xmlpp::Element* p_element)
// OLD {
// OLD     return cBaseBox::Save_To_XML_Node(p_element);
// OLD }
// OLD 
// OLD void cSpinBox::Load_From_Savegame(cSave_Level_Object* save_object)
// OLD {
// OLD     if(save_object->exists("spin_counter")) {
// OLD     // Let Activate do the work, then just set the counter
// OLD         Activate();
// OLD         m_spin_counter = string_to_float(save_object->Get_Value("spin_counter"));
// OLD     }
// OLD }
// OLD 
// OLD bool cSpinBox::Save_To_Savegame_XML_Node(xmlpp::Element* p_element) const
// OLD {
// OLD     cBaseBox::Save_To_Savegame_XML_Node(p_element);
// OLD 
// OLD     // spin counter
// OLD     if (m_spin) {
// OLD         Add_Property(p_element, "spin_counter", float_to_string(m_spin_counter));
// OLD     }
// OLD 
// OLD     return true;
// OLD }

void cSpinBox::Activate(void)
{
    // already spinning
    if (m_spin) {
        return;
    }

    cBaseBox::Activate();

    m_spin = 1;
    // OLD Update_Valid_Update();
    // passive box for spinning
    Set_Collision_Type(COLTYPE_PASSIVE);

    m_spin_counter = 0.0f;
    // enable animation
    Set_Image_Set("spin");
}

void cSpinBox::Stop(void)
{
    // already stopped spinning
    if (!m_spin) {
        return;
    }

    // disabled image
    if (!m_useable_count) {
        Set_Image_Set("disabled");
    }
    // default image
    else {
        Set_Image_Set("main");
    }
    // reset
    m_spin = 0;
    Update_Valid_Update();
    m_spin_counter = 0.0f;

    // back to a massive box
    Set_Collision_Type(COLTYPE_MASSIVE);
}

void cSpinBox::Update(void)
{
    cBaseBox::Update();

    if (m_spin) {
        m_spin_counter += gp_app->Get_SceneManager().Get_Speedfactor();
        // FIXME: Doesn’t stop spinning?

        // spinning animation finished
        if (m_curr_img == m_anim_img_start) {
            // spinning time finished
            if (m_spin_counter > speedfactor_fps * 5) {
                // reset spin counter
                m_spin_counter = 0.0f;
                // OLD // set to massive for collision check
                // OLD m_massive_type = MASS_MASSIVE;
                // OLD // collision data
                // OLD cObjectCollisionType* col_list = Collision_Check(&m_col_rect, COLLIDE_ONLY_BLOCKING);
                // OLD 
                // check if spinning should continue
                bool spin_again = 0;
                // OLD 
                // OLD // colliding with player or enemy
                // OLD if (col_list->Is_Included(TYPE_PLAYER) || col_list->Is_Included(ARRAY_ENEMY)) {
                // OLD     spin_again = 1;
                // OLD }
                // OLD // colliding with an active object
                // OLD else if (col_list->Is_Included(ARRAY_ACTIVE)) {
                // OLD     cSprite* col_obj = col_list->Find_First(ARRAY_ACTIVE)->m_obj;
                // OLD 
                // OLD     // check for items
                // OLD     if (col_obj->m_type == TYPE_MUSHROOM_LIVE_1 || col_obj->m_type == TYPE_MUSHROOM_DEFAULT ||
                // OLD             col_obj->m_type == TYPE_MUSHROOM_POISON || col_obj->m_type == TYPE_MUSHROOM_BLUE || col_obj->m_type == TYPE_MUSHROOM_GHOST ||
                // OLD             col_obj->m_type == TYPE_FIREPLANT || col_obj->m_type == TYPE_STAR || col_obj->m_type == TYPE_FALLING_GOLDPIECE) {
                // OLD         // found blocking active object
                // OLD         spin_again = 1;
                // OLD     }
                // OLD }
                // OLD 
                // OLD delete col_list;

                // continue spinning
                if (spin_again) {
                    // spin some time again
                    m_spin_counter = speedfactor_fps * 2;
                    // passive for spinning
                    Set_Collision_Type(COLTYPE_PASSIVE);
                }
                // finished spinning
                else {
                    Stop();
                }
            }
        }
    }
}

// OLD bool cSpinBox::Is_Update_Valid(void)
// OLD {
// OLD     if (m_spin) {
// OLD         return 1;
// OLD     }
// OLD 
// OLD     return cBaseBox::Is_Update_Valid();
// OLD }

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

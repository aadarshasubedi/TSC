/***************************************************************************
 * enemystopper.cpp  -  enemystopper class
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
#include "../level/level.hpp"
#include "actor.hpp"
#include "../core/collision.hpp"
#include "../core/errors.hpp"
#include "../core/xml_attributes.hpp"
#include "enemystopper.hpp"

using namespace TSC;

/* *** *** *** *** *** cEnemyStopper *** *** *** *** *** *** *** *** *** *** *** *** */

cEnemyStopper::cEnemyStopper()
    : cActor()
{
    Init();
}


cEnemyStopper::cEnemyStopper(XmlAttributes& attributes, cLevel& level, const std::string type_name)
    : cActor(attributes, level, type_name)
{
    Init();
}

cEnemyStopper::~cEnemyStopper(void)
{
    //
}

// OLD cEnemyStopper* cEnemyStopper::Copy(void) const
// OLD {
// OLD     cEnemyStopper* enemystopper = new cEnemyStopper(m_sprite_manager);
// OLD     enemystopper->Set_Pos(m_start_pos_x, m_start_pos_y, 1);
// OLD     return enemystopper;
// OLD }

void cEnemyStopper::Init(void)
{
    // OLD m_type = TYPE_ENEMY_STOPPER;

    m_name = _("Enemystopper");

    // size
    Set_Collision_Rect(sf::FloatRect(0, 0, 15.0f, 15.0f));
}

void cEnemyStopper::Draw(sf::RenderWindow& stage)
{
    cActor::Draw(stage);
}

// OLD bool cEnemyStopper::Is_Draw_Valid(void)
// OLD {
// OLD     // if editor not enabled
// OLD     if (!editor_enabled) {
// OLD         return 0;
// OLD     }
// OLD 
// OLD     // if not active or not on the screen
// OLD     if (!m_active || !Is_Visible_On_Screen()) {
// OLD         return 0;
// OLD     }
// OLD 
// OLD     return 1;
// OLD }

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

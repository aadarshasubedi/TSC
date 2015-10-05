/***************************************************************************
 * bonusbox.cpp  -  class for bonusbox
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
#include "bonusbox.hpp"

using namespace std;
using namespace TSC;

/* *** *** *** *** *** *** *** *** cBonusBox *** *** *** *** *** *** *** *** *** */

cBonusBox::cBonusBox()
    : cBaseBox()
{
    cBonusBox::Init();
}

cBonusBox::cBonusBox(XmlAttributes& attributes, cLevel& level, const std::string type_name)
    : cBaseBox(attributes, level, type_name)
{
    cBonusBox::Init();

    // item
    Set_Bonus_Type(static_cast<SpriteType>(string_to_int(attributes["item"])));
    // force best possible item
    Set_Force_Best_Item(string_to_int(attributes["force_best_item"]));
    // gold color
    if (box_type == TYPE_GOLDPIECE)
        Set_Goldcolor(Get_Color_Id(attributes.fetch("gold_color", Get_Color_Name(m_gold_color))));
}

cBonusBox::~cBonusBox(void)
{
    // OLD for (MovingSpriteList::iterator itr = m_active_items.begin(); itr != m_active_items.end(); ++itr) {
    // OLD     delete *itr;
    // OLD }
    // OLD 
    // OLD m_active_items.clear();
}

void cBonusBox::Init(void)
{
    // OLD m_type = TYPE_BONUS_BOX;
    m_force_best_item = 0;
    // OLD m_camera_range = 5000;

    Set_Animation_Type("Bonus");
    m_gold_color = COL_DEFAULT;
    Set_Goldcolor(COL_YELLOW);

    box_type = TYPE_UNDEFINED;
    m_name = "Bonusbox Empty";
}

// OLD cBonusBox* cBonusBox::Copy(void) const
// OLD {
// OLD     cBonusBox* bonusbox = new cBonusBox(m_sprite_manager);
// OLD     bonusbox->Set_Pos(m_start_pos_x, m_start_pos_y);
// OLD     bonusbox->Set_Animation_Type(m_anim_type);
// OLD     bonusbox->Set_Bonus_Type(box_type);
// OLD     bonusbox->Set_Invisible(m_box_invisible);
// OLD     bonusbox->Set_Force_Best_Item(m_force_best_item);
// OLD     bonusbox->Set_Goldcolor(m_gold_color);
// OLD     bonusbox->Set_Useable_Count(m_start_useable_count, 1);
// OLD     return bonusbox;
// OLD }

// OLD xmlpp::Element* cBonusBox::Save_To_XML_Node(xmlpp::Element* p_element)
// OLD {
// OLD     xmlpp::Element* p_node = cBaseBox::Save_To_XML_Node(p_element);
// OLD 
// OLD     // force best possible item
// OLD     Add_Property(p_node, "force_best_item", m_force_best_item);
// OLD     // gold color
// OLD     if (box_type == TYPE_GOLDPIECE)
// OLD         Add_Property(p_node, "gold_color", Get_Color_Name(m_gold_color));
// OLD 
// OLD     return p_node;
// OLD }

void cBonusBox::Set_Useable_Count(int count, bool new_startcount /* = 0 */)
{
    cBaseBox::Set_Useable_Count(count, new_startcount);

    // disable
    if (!m_useable_count) {
        Set_Image_Set("disabled");
    }
    // enable
    else {
        Set_Image_Set("main");
    }

    Update_Valid_Update();
}

void cBonusBox::Set_Bonus_Type(SpriteType bonus_type)
{
    // already set
    if (box_type == bonus_type) {
        return;
    }

    box_type = bonus_type;

    // set item image
    if (box_type == TYPE_UNDEFINED) {
        // OLD m_item_image = NULL;
        m_name = _("Bonusbox Empty");
    }
    else if (box_type == TYPE_POWERUP) {
        // force always best item
        m_force_best_item = 1;
        // OLD m_item_image = pVideo->Get_Package_Surface("game/editor/unknown.png");
    }
    else if (box_type == TYPE_MUSHROOM_DEFAULT) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/berry_big.imgset");
        m_name = _("Bonusbox Berry");
    }
    else if (box_type == TYPE_FIREPLANT) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/berry_fire.imgset");
        m_name = _("Bonusbox Fire berry");
    }
    else if (box_type == TYPE_MUSHROOM_BLUE) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/berry_ice.imgset");
        m_name = _("Bonusbox Ice berry");
    }
    else if (box_type == TYPE_MUSHROOM_GHOST) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/berry_ghost.imgset");
        m_name = _("Bonusbox Ghost berry");
    }
    else if (box_type == TYPE_MUSHROOM_LIVE_1) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/berry_life.imgset");
        m_name = _("Bonusbox 1-Up berry");
    }
    else if (box_type == TYPE_STAR) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/lemon.imgset");
        m_name = _("Bonusbox Lemon");
    }
    else if (box_type == TYPE_GOLDPIECE) {
        if (m_gold_color == COL_RED) {
            // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/goldpiece/red/jewel.imgset");
        }
        else {
            // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/goldpiece/yellow/jewel.imgset");
        }
    }
    else if (box_type == TYPE_MUSHROOM_POISON) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/berry_poison.imgset");
        m_name = _("Bonusbox Poisonous berry");
    }
    else {
        // OLD m_item_image = NULL;
        cerr << "Error : Unknown BonusBox Item type : " << box_type << endl;
    }
}

void cBonusBox::Set_Force_Best_Item(bool enable)
{
    // can't be set if random
    if (box_type == TYPE_POWERUP) {
        return;
    }

    m_force_best_item = enable;
}

void cBonusBox::Set_Goldcolor(DefaultColor new_color)
{
    /* FIXME: Due to bad code design, we have to re-set this even
     * if the colour is already set. Background: The box is created
     * with goldcolor set to YELLOW, but the box type itself is set
     * to UNDEFINED (= empty box). When you set the box type to GOLDPIECE,
     * this results in a call to Set_Goldcolor() (this method), but as the
     * goldcolor is already set, it would immediately return. Problem with
     * this is that m_name doesn’t get adjusted, it will show whatever
     * the box was previously (usually UNDEFINED => "Bonusbox Empty").
     * The proper solution to this is to divide gold boxes and powerup boxes;
     * it doesn’t make sense to set the gold color on a fireberry box for
     * example anyway. */
    // already set
    //if (m_gold_color == new_color) {
    //    return;
    //}

    if (new_color == COL_DEFAULT) {
        m_gold_color = COL_YELLOW;
    }

    m_gold_color = new_color;

    if (box_type != TYPE_GOLDPIECE) {
        return;
    }

    if (m_gold_color == COL_YELLOW) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/goldpiece/yellow/jewel.imgset");
        m_name = _("Bonusbox Jewel");
    }
    else if (m_gold_color == COL_RED) {
        // OLD m_item_image = cImageSet::Fetch_Single_Image("game/items/goldpiece/red/jewel.imgset");
        m_name = _("Bonusbox Red Jewel");
    }
    else {
        cerr << "Warning : Unknown Bonusbox Gold Color " << m_gold_color << endl;
    }
}

void cBonusBox::Activate(void)
{
    cBaseBox::Activate();

    bool random = 0;

    // random
    if (box_type == TYPE_POWERUP) {
        int r = rand() % 5;

        if (r == 0) {
            box_type = TYPE_MUSHROOM_DEFAULT;
        }
        else if (r == 1) {
            box_type = TYPE_FIREPLANT;
        }
        else if (r == 2) {
            box_type = TYPE_MUSHROOM_BLUE;
        }
        else if (r == 3) {
            box_type = TYPE_MUSHROOM_GHOST;
        }
        else if (r == 4) {
            box_type = TYPE_STAR;
        }

        random = 1;
    }

    Alex_type current_alex_type;
    const cLevel_Player* p_player = gp_current_level->Get_Player();

    // use original type
    if (p_player->m_alex_type == ALEX_GHOST) {
        // OLD current_alex_type = p_player->m_alex_type_temp_power;
    }
    // already using original type
    else {
        current_alex_type = p_player->m_alex_type;
    }

    // the item from this box
    cActor* box_item = NULL;

    // OLD // no item
    // OLD if (box_type == TYPE_UNDEFINED) {
    // OLD     // OLD pAudio->Play_Sound("item/empty_box.wav");
    // OLD }
    // OLD // check if lower item should be used if no force best item
    // OLD else if (!m_force_best_item && (box_type == TYPE_FIREPLANT || box_type == TYPE_MUSHROOM_BLUE) &&
    // OLD          (current_alex_type == ALEX_SMALL || ((current_alex_type == ALEX_FIRE || current_alex_type == ALEX_ICE) /* OLD && !pHud_Itembox->m_item_id */))) {
    // OLD     // OLD pAudio->Play_Sound("sprout_1.ogg");
    // OLD 
    // OLD     cMushroom* mushroom = new cMushroom(m_sprite_manager);
    // OLD     mushroom->Set_Pos(m_start_pos_x - ((m_item_image->m_w - m_rect.m_w) / 2), m_start_pos_y - ((m_item_image->m_h - m_rect.m_h) / 2), 1);
    // OLD     box_item = static_cast<cMovingSprite*>(mushroom);
    // OLD }
    // OLD else if (box_type == TYPE_FIREPLANT) {
    // OLD     pAudio->Play_Sound("sprout_1.ogg");
    // OLD     box_item = static_cast<cMovingSprite*>(new cFirePlant(m_sprite_manager));
    // OLD     box_item->Set_Pos(m_start_pos_x - ((m_item_image->m_w - m_rect.m_w) / 2), m_start_pos_y, 1);
    // OLD }
    // OLD else if (box_type == TYPE_MUSHROOM_DEFAULT || box_type == TYPE_MUSHROOM_LIVE_1 || box_type == TYPE_MUSHROOM_POISON || box_type == TYPE_MUSHROOM_BLUE || box_type == TYPE_MUSHROOM_GHOST) {
    // OLD     pAudio->Play_Sound("sprout_1.ogg");
    // OLD 
    // OLD     cMushroom* mushroom = new cMushroom(m_sprite_manager);
    // OLD     mushroom->Set_Pos(m_start_pos_x - ((m_item_image->m_w - m_rect.m_w) / 2), m_start_pos_y - ((m_item_image->m_h - m_rect.m_h) / 2), 1);
    // OLD     mushroom->Set_Type(box_type);
    // OLD     box_item = static_cast<cMovingSprite*>(mushroom);
    // OLD }
    // OLD else if (box_type == TYPE_STAR) {
    // OLD     pAudio->Play_Sound("sprout_1.ogg");
    // OLD     cjStar* star = new cjStar(m_sprite_manager);
    // OLD     star->Set_Pos(m_start_pos_x - ((m_item_image->m_w - m_rect.m_w) / 2), m_start_pos_y, 1);
    // OLD     star->Set_On_Top(this);
    // OLD     star->Set_Spawned(1);
    // OLD     // add to global objects
    // OLD     m_sprite_manager->Add(star);
    // OLD }
    // OLD else if (box_type == TYPE_GOLDPIECE) {
    // OLD     pAudio->Play_Sound("item/jewel_1.ogg");
    // OLD 
    // OLD     cJGoldpiece* goldpiece = new cJGoldpiece(m_sprite_manager);
    // OLD     goldpiece->Set_Pos(m_start_pos_x - ((m_item_image->m_w - m_rect.m_w) / 2), m_pos_y, 1);
    // OLD     goldpiece->Set_Gold_Color(m_gold_color);
    // OLD     // add to global objects
    // OLD     m_sprite_manager->Add(goldpiece);
    // OLD }
    // OLD else {
    // OLD     cerr << "Error : Unknown bonusbox item type " << box_type << endl;
    // OLD     return;
    // OLD }

    // set back random state
    if (random) {
        box_type = TYPE_POWERUP;
    }

    if (box_item) {
        // set posz behind box
        box_item->m_pos_z = m_pos_z - 0.000001f;
        // set spawned
        box_item->Set_Spawned(1);

        // add to item list
        // OLD m_active_items.push_back(box_item);
        Update_Valid_Update();
    }
}

void cBonusBox::Update(void)
{
    // update active items
    // OLD for (MovingSpriteList::iterator itr = m_active_items.begin(); itr != m_active_items.end();) {
    // OLD     cPowerUp* powerup = static_cast<cPowerUp*>(*itr);
    // OLD 
    // OLD     if (!powerup->m_active) {
    // OLD         ++itr;
    // OLD         continue;
    // OLD     }
    // OLD 
    // OLD     float diff = powerup->m_pos_y - (m_pos_y - powerup->m_col_rect.m_h - powerup->m_col_pos.m_y);
    // OLD 
    // OLD     // position over the box reached
    // OLD     if (diff < 0.0f) {
    // OLD         // clear animation counter
    // OLD         powerup->m_counter = 0.0f;
    // OLD 
    // OLD         // set powerup default posz
    // OLD         powerup->m_pos_z = 0.05f;
    // OLD 
    // OLD         // set the item on top
    // OLD         powerup->Set_On_Top(this, 0);
    // OLD         // add the item to the level objects
    // OLD         m_sprite_manager->Add(powerup);
    // OLD 
    // OLD         // remove from array
    // OLD         itr = m_active_items.erase(itr);
    // OLD         Update_Valid_Update();
    // OLD     }
    // OLD     // move upwards
    // OLD     else {
    // OLD         powerup->m_counter += pFramerate->m_speed_factor;
    // OLD         powerup->Move(0.0f, -1.0f - (diff * 0.1f));
    // OLD 
    // OLD         ++itr;
    // OLD     }
    // OLD }

    cBaseBox::Update();
}

void cBonusBox::Draw(sf::RenderWindow& stage) const
{
    // OLD for (MovingSpriteList::iterator itr = m_active_items.begin(); itr != m_active_items.end(); ++itr) {
    // OLD     cMovingSprite* obj = (*itr);
    // OLD 
    // OLD     if (!obj->m_active) {
    // OLD         continue;
    // OLD     }
    // OLD 
    // OLD     obj->Draw();
    // OLD }

    cBaseBox::Draw(stage);
}

// OLD bool cBonusBox::Is_Update_Valid()
// OLD {
// OLD     // OLD // if item is in animation
// OLD     // OLD if (!m_active_items.empty()) {
// OLD     // OLD     return 1;
// OLD     // OLD }
// OLD 
// OLD     return cBaseBox::Is_Update_Valid();
// OLD }

// OLD void cBonusBox::Editor_Activate(void)
// OLD {
// OLD     // BaseBox Settings first
// OLD     cBaseBox::Editor_Activate();
// OLD 
// OLD     // get window manager
// OLD     CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
// OLD 
// OLD     // Animation
// OLD     CEGUI::Combobox* combobox = static_cast<CEGUI::Combobox*>(wmgr.createWindow("TaharezLook/Combobox", "editor_bonusbox_animation"));
// OLD     Editor_Add(UTF8_("Animation"), UTF8_("Use the Power animation if the box has a good or needed item for this level"), combobox, 120, 100);
// OLD 
// OLD     combobox->addItem(new CEGUI::ListboxTextItem("Default"));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem("Bonus"));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem("Power"));
// OLD 
// OLD     combobox->setText(reinterpret_cast<const CEGUI::utf8*>(m_anim_type.c_str()));
// OLD 
// OLD     combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&cBonusBox::Editor_Animation_Select, this));
// OLD 
// OLD     // Item
// OLD     combobox = static_cast<CEGUI::Combobox*>(wmgr.createWindow("TaharezLook/Combobox", "editor_bonusbox_item"));
// OLD     Editor_Add(UTF8_("Item"), UTF8_("The item that gets spawned"), combobox, 160, 140);
// OLD 
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Empty")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Random")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Berry")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Fire berry")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Ice berry")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Ghost berry")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("1-UP berry")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Lemon")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Jewel")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Poisonous berry")));
// OLD 
// OLD     if (box_type == TYPE_MUSHROOM_DEFAULT) {
// OLD         combobox->setText(UTF8_("Berry"));
// OLD     }
// OLD     else if (box_type == TYPE_FIREPLANT) {
// OLD         combobox->setText(UTF8_("Fire berry"));
// OLD     }
// OLD     else if (box_type == TYPE_MUSHROOM_BLUE) {
// OLD         combobox->setText(UTF8_("Ice berry"));
// OLD     }
// OLD     else if (box_type == TYPE_MUSHROOM_GHOST) {
// OLD         combobox->setText(UTF8_("Ghost berry"));
// OLD     }
// OLD     else if (box_type == TYPE_MUSHROOM_LIVE_1) {
// OLD         combobox->setText(UTF8_("1-UP berry"));
// OLD     }
// OLD     else if (box_type == TYPE_STAR) {
// OLD         combobox->setText(UTF8_("Lemon"));
// OLD     }
// OLD     else if (box_type == TYPE_GOLDPIECE) {
// OLD         combobox->setText(UTF8_("Jewel"));
// OLD     }
// OLD     else if (box_type == TYPE_MUSHROOM_POISON) {
// OLD         combobox->setText(UTF8_("Poisonous berry"));
// OLD     }
// OLD     else if (box_type == TYPE_POWERUP) {
// OLD         combobox->setText(UTF8_("Random"));
// OLD     }
// OLD     else {
// OLD         combobox->setText(UTF8_("Empty"));
// OLD     }
// OLD 
// OLD     combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&cBonusBox::Editor_Item_Select, this));
// OLD 
// OLD     // Force best item
// OLD     combobox = static_cast<CEGUI::Combobox*>(wmgr.createWindow("TaharezLook/Combobox", "editor_bonusbox_force_best_item"));
// OLD     Editor_Add(UTF8_("Force item"), UTF8_("Force best available item when activated"), combobox, 120, 80);
// OLD 
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Enabled")));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem(UTF8_("Disabled")));
// OLD 
// OLD     if (m_force_best_item) {
// OLD         combobox->setText(UTF8_("Enabled"));
// OLD     }
// OLD     else {
// OLD         combobox->setText(UTF8_("Disabled"));
// OLD     }
// OLD 
// OLD     combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&cBonusBox::Editor_Force_best_item_Select, this));
// OLD 
// OLD     // gold color
// OLD     combobox = static_cast<CEGUI::Combobox*>(wmgr.createWindow("TaharezLook/Combobox", "editor_bonusbox_gold_color"));
// OLD     Editor_Add(UTF8_("Jewel color"), UTF8_("Jewel color if the item is a jewel"), combobox, 100, 80);
// OLD 
// OLD     combobox->addItem(new CEGUI::ListboxTextItem("yellow"));
// OLD     combobox->addItem(new CEGUI::ListboxTextItem("red"));
// OLD     combobox->setText(Get_Color_Name(m_gold_color));
// OLD 
// OLD     combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&cBonusBox::Editor_Gold_Color_Select, this));
// OLD 
// OLD     // init
// OLD     Editor_Init();
// OLD }
// OLD 
// OLD void cBonusBox::Editor_State_Update(void)
// OLD {
// OLD     CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
// OLD 
// OLD     // Force best item
// OLD     CEGUI::Combobox* combobox = static_cast<CEGUI::Combobox*>(wmgr.getWindow("editor_bonusbox_force_best_item"));
// OLD 
// OLD     if (box_type == TYPE_UNDEFINED || box_type == TYPE_POWERUP || box_type == TYPE_MUSHROOM_DEFAULT || box_type == TYPE_MUSHROOM_LIVE_1 || box_type == TYPE_MUSHROOM_POISON ||
// OLD             box_type == TYPE_MUSHROOM_GHOST || box_type == TYPE_STAR || box_type == TYPE_GOLDPIECE) {
// OLD         combobox->setEnabled(0);
// OLD     }
// OLD     else {
// OLD         combobox->setEnabled(1);
// OLD     }
// OLD 
// OLD     // gold color
// OLD     combobox = static_cast<CEGUI::Combobox*>(wmgr.getWindow("editor_bonusbox_gold_color"));
// OLD 
// OLD     if (box_type != TYPE_GOLDPIECE) {
// OLD         combobox->setEnabled(0);
// OLD     }
// OLD     else {
// OLD         combobox->setEnabled(1);
// OLD     }
// OLD }
// OLD 
// OLD bool cBonusBox::Editor_Animation_Select(const CEGUI::EventArgs& event)
// OLD {
// OLD     const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(event);
// OLD     CEGUI::ListboxItem* item = static_cast<CEGUI::Combobox*>(windowEventArgs.window)->getSelectedItem();
// OLD 
// OLD     Set_Animation_Type(item->getText().c_str());
// OLD 
// OLD     return 1;
// OLD }
// OLD 
// OLD bool cBonusBox::Editor_Item_Select(const CEGUI::EventArgs& event)
// OLD {
// OLD     const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(event);
// OLD     CEGUI::ListboxItem* item = static_cast<CEGUI::Combobox*>(windowEventArgs.window)->getSelectedItem();
// OLD 
// OLD     if (item->getText().compare(UTF8_("Berry")) == 0) {
// OLD         Set_Bonus_Type(TYPE_MUSHROOM_DEFAULT);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Fire berry")) == 0) {
// OLD         Set_Bonus_Type(TYPE_FIREPLANT);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Ice berry")) == 0) {
// OLD         Set_Bonus_Type(TYPE_MUSHROOM_BLUE);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Ghost berry")) == 0) {
// OLD         Set_Bonus_Type(TYPE_MUSHROOM_GHOST);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("1-UP berry")) == 0) {
// OLD         Set_Bonus_Type(TYPE_MUSHROOM_LIVE_1);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Lemon")) == 0) {
// OLD         Set_Bonus_Type(TYPE_STAR);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Jewel")) == 0) {
// OLD         Set_Bonus_Type(TYPE_GOLDPIECE);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Poisonous berry")) == 0) {
// OLD         Set_Bonus_Type(TYPE_MUSHROOM_POISON);
// OLD     }
// OLD     else if (item->getText().compare(UTF8_("Random")) == 0) {
// OLD         Set_Bonus_Type(TYPE_POWERUP);
// OLD     }
// OLD     else {
// OLD         Set_Bonus_Type(TYPE_UNDEFINED);
// OLD     }
// OLD 
// OLD     Editor_State_Update();
// OLD 
// OLD     return 1;
// OLD }
// OLD 
// OLD bool cBonusBox::Editor_Force_best_item_Select(const CEGUI::EventArgs& event)
// OLD {
// OLD     const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(event);
// OLD     CEGUI::ListboxItem* item = static_cast<CEGUI::Combobox*>(windowEventArgs.window)->getSelectedItem();
// OLD 
// OLD     if (item->getText().compare(UTF8_("Enabled")) == 0) {
// OLD         Set_Force_Best_Item(1);
// OLD     }
// OLD     else {
// OLD         Set_Force_Best_Item(0);
// OLD     }
// OLD 
// OLD     return 1;
// OLD }
// OLD 
// OLD bool cBonusBox::Editor_Gold_Color_Select(const CEGUI::EventArgs& event)
// OLD {
// OLD     const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(event);
// OLD     CEGUI::ListboxItem* item = static_cast<CEGUI::Combobox*>(windowEventArgs.window)->getSelectedItem();
// OLD 
// OLD     Set_Goldcolor(Get_Color_Id(item->getText().c_str()));
// OLD 
// OLD     return 1;
// OLD }

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */


/***************************************************************************
 * text_box.cpp  -  box speaking to you
 *
 * Copyright © 2007 - 2011 Florian Richter
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
#include "text_box.hpp"

using namespace TSC;

/* *** *** *** *** *** *** *** *** cText_Box *** *** *** *** *** *** *** *** *** */

static unsigned int text_box_window_width = 300;
static unsigned int text_box_window_height = 200;

cText_Box::cText_Box()
    : cBaseBox()
{
    cText_Box::Init();
}

cText_Box::cText_Box(XmlAttributes& attributes, cLevel& level, const std::string type_name)
    : cBaseBox(attributes, level, type_name)
{
    cText_Box::Init();

    // text
    Set_Text(xml_string_to_string(attributes["text"]));
}

cText_Box::~cText_Box(void)
{

}

void cText_Box::Init(void)
{
    // OLD m_type = TYPE_TEXT_BOX;
    box_type = TYPE_TEXT_BOX;
    m_name = _("Text Box");

    // default is infinite times activate-able
    Set_Useable_Count(-1, 1);
    // Spinbox Animation
    Set_Animation_Type("Default");

    // todo : editor image needed
    //item_image = NULL;
}

// OLD cText_Box* cText_Box::Copy(void) const
// OLD {
// OLD     cText_Box* text_box = new cText_Box(m_sprite_manager);
// OLD     text_box->Set_Pos(m_start_pos_x, m_start_pos_y);
// OLD     text_box->Set_Text(m_text);
// OLD     text_box->Set_Invisible(m_box_invisible);
// OLD     return text_box;
// OLD }

// OLD xmlpp::Element* cText_Box::Save_To_XML_Node(xmlpp::Element* p_element)
// OLD {
// OLD     xmlpp::Element* p_node = cBaseBox::Save_To_XML_Node(p_element);
// OLD 
// OLD     // text
// OLD     Add_Property(p_node, "text", m_text);
// OLD 
// OLD     return p_node;
// OLD }

void cText_Box::Activate(void)
{
    cBaseBox::Activate();

    // OLD CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
    // OLD CEGUI::MultiLineEditbox* editbox = static_cast<CEGUI::MultiLineEditbox*>(wmgr.createWindow("TaharezLook/MultiLineEditbox", "text_box_text"));
    // OLD 
    // OLD // add to main window
    // OLD pGuiSystem->getGUISheet()->addChildWindow(editbox);
    // OLD 
    // OLD 
    // OLD // set on top
    // OLD editbox->setAlwaysOnTop(1);
    // OLD // set position
    // OLD float text_pos_x = m_pos_x - (text_box_window_width * 0.5f) + (m_rect.m_w * 0.5f);
    // OLD float text_pos_y = m_pos_y - 5 - text_box_window_height;
    // OLD 
    // OLD // if not on screen on the left side
    // OLD if (text_pos_x < 0) {
    // OLD     // put it on screen
    // OLD     text_pos_x = 0;
    // OLD }
    // OLD // if not on screen on the right side
    // OLD if (text_pos_x > pActive_Camera->m_limit_rect.m_x + pActive_Camera->m_limit_rect.m_w - text_box_window_width) {
    // OLD     // put it on screen
    // OLD     text_pos_x = pActive_Camera->m_limit_rect.m_x + pActive_Camera->m_limit_rect.m_w - text_box_window_width;
    // OLD }
    // OLD 
    // OLD editbox->setXPosition(CEGUI::UDim(0, (text_pos_x - pActive_Camera->m_x) * global_upscalex));
    // OLD editbox->setYPosition(CEGUI::UDim(0, (text_pos_y - pActive_Camera->m_y) * global_upscaley));
    // OLD // set size
    // OLD editbox->setWidth(CEGUI::UDim(0, text_box_window_width * global_upscalex));
    // OLD editbox->setHeight(CEGUI::UDim(0, text_box_window_height * global_upscaley));
    // OLD 
    // OLD // set text
    // OLD editbox->setText(reinterpret_cast<const CEGUI::utf8*>(m_text.c_str()));
    // OLD // always hide horizontal scrollbar
    // OLD editbox->getHorzScrollbar()->hide();
    // OLD 
    // OLD bool display = 1;
    // OLD 
    // OLD while (display) {
    // OLD     while (SDL_PollEvent(&input_event)) {
    // OLD         if (input_event.type == SDL_KEYDOWN) {
    // OLD             pKeyboard->m_keys[input_event.key.keysym.sym] = 1;
    // OLD 
    // OLD             // exit keys
    // OLD             if (input_event.key.keysym.sym == pPreferences->m_key_action || input_event.key.keysym.sym == SDLK_ESCAPE || input_event.key.keysym.sym == SDLK_RETURN || input_event.key.keysym.sym == SDLK_SPACE) {
    // OLD                 display = 0;
    // OLD                 break;
    // OLD             }
    // OLD             // handled keys
    // OLD             else if (input_event.key.keysym.sym == pPreferences->m_key_right || input_event.key.keysym.sym == pPreferences->m_key_left) {
    // OLD                 pKeyboard->Key_Down(input_event.key.keysym.sym);
    // OLD             }
    // OLD         }
    // OLD         else if (input_event.type == SDL_KEYUP) {
    // OLD             pKeyboard->m_keys[input_event.key.keysym.sym] = 0;
    // OLD 
    // OLD             // handled keys
    // OLD             if (input_event.key.keysym.sym == pPreferences->m_key_right || input_event.key.keysym.sym == pPreferences->m_key_left) {
    // OLD                 pKeyboard->Key_Up(input_event.key.keysym.sym);
    // OLD             }
    // OLD         }
    // OLD         else if (input_event.type == SDL_JOYBUTTONDOWN) {
    // OLD             pJoystick->Set_Button(input_event.jbutton.button, 1);
    // OLD 
    // OLD             if (input_event.jbutton.button == pPreferences->m_joy_button_action || input_event.jbutton.button == pPreferences->m_joy_button_exit) {
    // OLD                 display = 0;
    // OLD                 break;
    // OLD             }
    // OLD         }
    // OLD         else if (input_event.type == SDL_JOYBUTTONUP) {
    // OLD             pJoystick->Set_Button(input_event.jbutton.button, 0);
    // OLD         }
    // OLD         else if (input_event.type == SDL_JOYHATMOTION) {
    // OLD             pJoystick->Handle_Hat(&input_event);
    // OLD             break;
    // OLD         }
    // OLD         else if (input_event.type == SDL_JOYAXISMOTION) {
    // OLD             pJoystick->Handle_Motion(&input_event);
    // OLD             break;
    // OLD         }
    // OLD     }
    // OLD 
    // OLD     Uint8* keys = SDL_GetKeyState(NULL);
    // OLD     Sint16 joy_ver_axis = 0;
    // OLD 
    // OLD     // if joystick enabled
    // OLD     if (pPreferences->m_joy_enabled) {
    // OLD         joy_ver_axis = SDL_JoystickGetAxis(pJoystick->m_joystick, pPreferences->m_joy_axis_ver);
    // OLD     }
    // OLD 
    // OLD     // down
    // OLD     if (keys[pPreferences->m_key_down] || joy_ver_axis > pPreferences->m_joy_axis_threshold) {
    // OLD         editbox->getVertScrollbar()->setScrollPosition(editbox->getVertScrollbar()->getScrollPosition() + (editbox->getVertScrollbar()->getStepSize() * 0.25f * pFramerate->m_speed_factor));
    // OLD     }
    // OLD     // up
    // OLD     if (keys[pPreferences->m_key_up] || joy_ver_axis < -pPreferences->m_joy_axis_threshold) {
    // OLD         editbox->getVertScrollbar()->setScrollPosition(editbox->getVertScrollbar()->getScrollPosition() - (editbox->getVertScrollbar()->getStepSize() * 0.25f * pFramerate->m_speed_factor));
    // OLD     }
    // OLD 
    // OLD     // move camera because text could not be completely visible
    // OLD     if (pActive_Camera->m_y_offset > 0) {
    // OLD         pActive_Camera->m_y_offset -= 2;
    // OLD         // set position
    // OLD         pActive_Camera->Center();
    // OLD 
    // OLD         // set position
    // OLD         editbox->setXPosition(CEGUI::UDim(0, (text_pos_x - pActive_Camera->m_x) * global_upscalex));
    // OLD         editbox->setYPosition(CEGUI::UDim(0, (text_pos_y - pActive_Camera->m_y) * global_upscaley));
    // OLD     }
    // OLD 
    // OLD     // update animation
    // OLD     Update();
    // OLD 
    // OLD     // update audio
    // OLD     pAudio->Update();
    // OLD     // draw
    // OLD     Draw_Game();
    // OLD     // render
    // OLD     pVideo->Render();
    // OLD     pFramerate->Update();
    // OLD }
    // OLD 
    // OLD wmgr.destroyWindow(editbox);
}

void cText_Box::Update(void)
{
    cBaseBox::Update();
}

void cText_Box::Set_Text(const std::string& str_text)
{
    m_text = str_text;
}

// OLD void cText_Box::Editor_Activate(void)
// OLD {
// OLD     // BaseBox Settings first
// OLD     cBaseBox::Editor_Activate();
// OLD 
// OLD     // get window manager
// OLD     CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
// OLD 
// OLD     // text
// OLD     CEGUI::MultiLineEditbox* editbox = static_cast<CEGUI::MultiLineEditbox*>(wmgr.createWindow("TaharezLook/MultiLineEditbox", "text_box_text"));
// OLD     Editor_Add(UTF8_("Text"), UTF8_("Text to display when activated"), editbox, static_cast<float>(text_box_window_width), static_cast<float>(text_box_window_height));
// OLD 
// OLD     editbox->setText(reinterpret_cast<const CEGUI::utf8*>(m_text.c_str()));
// OLD     editbox->subscribeEvent(CEGUI::MultiLineEditbox::EventTextChanged, CEGUI::Event::Subscriber(&cText_Box::Editor_Text_Text_Changed, this));
// OLD 
// OLD     // init
// OLD     Editor_Init();
// OLD }
// OLD 
// OLD bool cText_Box::Editor_Text_Text_Changed(const CEGUI::EventArgs& event)
// OLD {
// OLD     const CEGUI::WindowEventArgs& windowEventArgs = static_cast<const CEGUI::WindowEventArgs&>(event);
// OLD     std::string str_text = static_cast<CEGUI::MultiLineEditbox*>(windowEventArgs.window)->getText().c_str();
// OLD 
// OLD     Set_Text(str_text);
// OLD 
// OLD     return 1;
// OLD }

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

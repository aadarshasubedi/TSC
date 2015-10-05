/***************************************************************************
 * box.h
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

#ifndef TSC_BOX_HPP
#define TSC_BOX_HPP

namespace TSC {

    /* *** *** *** *** *** *** *** *** Box invisible types *** *** *** *** *** *** *** *** *** */

    enum Box_Invisible_Type {
        // always visible
        BOX_VISIBLE = 0,
        // visible after activation
        BOX_INVISIBLE_MASSIVE = 1,
        // only visible in ghost mode
        BOX_GHOST = 2,
        // visible after activation and only touchable in the activation direction
        BOX_INVISIBLE_SEMI_MASSIVE = 3
    };

    /* *** *** *** *** *** *** *** *** cBaseBox *** *** *** *** *** *** *** *** *** */

    class cBaseBox: public cAnimatedActor {
    public:
        cBaseBox();
        cBaseBox(XmlAttributes& attributes, cLevel& level, const std::string type_name = "box");

        virtual ~cBaseBox(void);

        /* Set the Animation Type
         * new_anim_type can be : Bonus, Default or Power
        */
        void Set_Animation_Type(const std::string& new_anim_type);
        // sets the count this object can be activated
        virtual void Set_Useable_Count(int count, bool new_startcount = 0);
        // Set invisible type
        void Set_Box_Invisible(Box_Invisible_Type type);

        // OLD virtual void Set_Massive_Type(MassiveType type);

        // activates collision movement
        void Activate_Collision(ObjectDirection col_direction);
        // updates the collision movement
        void Update_Collision(void);

        // collision with the given enemy
        void Col_Enemy(cSprite* obj);

        // activate
        virtual void Activate(void);

        // update
        virtual void Update();
        // draw
        virtual void Draw(sf::RenderWindow& stage) const;

        // Generate activation Particles
        void Generate_Activation_Particles(void);

        // if update is valid for the current state
        // OLD virtual bool Is_Update_Valid();

        // handle the basic box player collision
        virtual bool Handle_Collision_Player(cCollision* p_collision);
        // handle the basic box enemy collision
        virtual bool Handle_Collision_Enemy(cCollision* p_collision);

        // editor activation
        // OLD virtual void Editor_Activate(void);
        // editor useable count text changed event
        // OLD bool Editor_Useable_Count_Text_Changed(const CEGUI::EventArgs& event);
        // editor invisible option selected event
        // OLD bool Editor_Invisible_Select(const CEGUI::EventArgs& event);

        // animation type
        std::string m_anim_type;
        // box type
        SpriteType box_type;

        // leveleditor item image
        // OLD cGL_Surface* m_item_image;

        // moving direction when activated
        ObjectDirection m_move_col_dir;
        // current moving counter ( if activated )
        float m_move_counter;
        // if object is moving back to the original position
        bool m_move_back;
        /* number of times this box object can be activated
         * the box is visible if useable_count != start_useable_count
         * if set to -1 it is infinite (and is set to -2 if activated)
        */
        int m_start_useable_count;
        int m_useable_count;

        // box invisible type
        Box_Invisible_Type m_box_invisible;

        // active particle animation counter
        float m_particle_counter_active;

        // Save to XML node
        // OLD virtual xmlpp::Element* Save_To_XML_Node(xmlpp::Element* p_element);
        // OLD virtual std::string Create_Name(void) const;

    protected:
        // OLD virtual std::string Get_XML_Type_Name();
    private:
        void Init();
    };

    /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC

#endif

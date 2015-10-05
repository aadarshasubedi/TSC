/***************************************************************************
 * bonusbox.h
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

#ifndef TSC_BONUSBOX_HPP
#define TSC_BONUSBOX_HPP

namespace TSC {

    /* *** *** *** *** *** *** *** *** cBonusBox *** *** *** *** *** *** *** *** *** */

    class cBonusBox: public cBaseBox {
    public:
        // constructor
        cBonusBox();
        // create from stream
        cBonusBox(XmlAttributes& attributes, cLevel& level, const std::string type_name = "box");
        // destructor
        virtual ~cBonusBox(void);

        // OLD // load from stream
        // OLD virtual void Load_From_XML(XmlAttributes& attributes);
        // OLD 
        // OLD // Create the MRuby object for this
        // OLD virtual mrb_value Create_MRuby_Object(mrb_state* p_state)
        // OLD {
        // OLD     return mrb_obj_value(Data_Wrap_Struct(p_state, mrb_class_get(p_state, "BonusBox"), &Scripting::rtTSC_Scriptable, this));
        // OLD }

        // sets the count this object can be activated
        virtual void Set_Useable_Count(int count, bool new_startcount = 0);
        /* Set the bonus type
         * TYPE_UNDEFINED           for empty
         * TYPE_MUSHROOM_DEFAULT    for Mushroom
         * TYPE_FIREPLANT           for Mushroom or Fireplant
         * TYPE_MUSHROOM_BLUE       for Mushroom or Blue Mushroom
         * TYPE_MUSHROOM_GHOST      for Mushroom or Ghost Mushroom
         * TYPE_MUSHROOM_LIVE_1     for 1-UP Mushroom
         * TYPE_STAR                for Star
         * TYPE_GOLDPIECE           for Goldpiece
         * TYPE_MUSHROOM_POISON     for Poison Mushroom
        */
        void Set_Bonus_Type(SpriteType bonus_type);
        /* Set if to force the best possible item
         * and not to use lower items automatically
        */
        void Set_Force_Best_Item(bool enable);
        // Set the gold color
        void Set_Goldcolor(DefaultColor new_color);

        // Activates the bonus item
        virtual void Activate(void);

        // update
        virtual void Update();
        // draw
        virtual void Draw(sf::RenderWindow& stage) const;

        // OLD // editor activation
        // OLD virtual void Editor_Activate(void);
        // OLD // editor state update
        // OLD virtual void Editor_State_Update(void);
        // OLD // editor animation option selected event
        // OLD bool Editor_Animation_Select(const CEGUI::EventArgs& event);
        // OLD // editor item option selected event
        // OLD bool Editor_Item_Select(const CEGUI::EventArgs& event);
        // OLD // editor force best item option selected event
        // OLD bool Editor_Force_best_item_Select(const CEGUI::EventArgs& event);
        // OLD // editor gold color option selected event
        // OLD bool Editor_Gold_Color_Select(const CEGUI::EventArgs& event);

        // force best possible item
        bool m_force_best_item;

        /* active items
         * list of active item is available
         * and gets added to the level objects if finished
        */
        // OLD typedef vector<cMovingSprite*> MovingSpriteList;
        // OLD MovingSpriteList m_active_items;
        // Goldpiece color
        DefaultColor m_gold_color;

        // Save to node
        // OLD virtual xmlpp::Element* Save_To_XML_Node(xmlpp::Element* p_element);

    protected:
        // typename inherited
    private:
        void Init();
    };

    /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC

#endif

/***************************************************************************
 * spinbox.h
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

#ifndef TSC_SPINBOX_HPP
#define TSC_SPINBOX_HPP

namespace TSC {

    /* *** *** *** *** *** *** *** *** cSpinBox *** *** *** *** *** *** *** *** *** */

    class cSpinBox: public cBaseBox {
    public:
        // constructor
        cSpinBox();
        // create from stream
        cSpinBox(XmlAttributes& attributes, cLevel& level, const std::string type_name = "box");
        // destructor
        virtual ~cSpinBox(void);

        // copy
        // OLD virtual cSpinBox* Copy(void) const;

        // OLD // load from savegame
        // OLD virtual void Load_From_Savegame(cSave_Level_Object* save_object);
        // OLD // save to savegame
        // OLD virtual bool Save_To_Savegame_XML_Node(xmlpp::Element* p_element) const;

        // OLD // Create the MRuby object for this
        // OLD virtual mrb_value Create_MRuby_Object(mrb_state* p_state)
        // OLD {
        // OLD     return mrb_obj_value(Data_Wrap_Struct(p_state, mrb_class_get(p_state, "SpinBox"), &Scripting::rtTSC_Scriptable, this));
        // OLD }

        // Activate the Spinning
        virtual void Activate(void);
        // Stop the Spinning
        void Stop(void);
        // update
        virtual void Update();

        // if update is valid for the current state
        // OLD virtual bool Is_Update_Valid(void);

        // spin counter
        float m_spin_counter;

        // if spinning
        bool m_spin;

        // Save below given XML node
        // OLD virtual xmlpp::Element* Save_To_XML_Node(xmlpp::Element* p_element);

    protected:
        // typename inherited
    private:
        // init defaults
        void Init(void);
    };

    /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC

#endif

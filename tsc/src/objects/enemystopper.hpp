/***************************************************************************
 * enemystopper.h
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

#ifndef TSC_ENEMYSTOPPER_HPP
#define TSC_ENEMYSTOPPER_HPP

namespace TSC {

    /* *** *** *** *** *** cEnemyStopper *** *** *** *** *** *** *** *** *** *** *** *** */

    class cEnemyStopper : public cActor {
    public:
        // constructor
        cEnemyStopper();
        cEnemyStopper(XmlAttributes& attributes, cLevel& level, const std::string type_name = "enemystopper");
        // destructor
        virtual ~cEnemyStopper(void);

        // init defaults
        void Init(void);
        // copy
        // OLD virtual cEnemyStopper* Copy(void) const;

        // Create the MRuby object for this
        // OLD virtual mrb_value Create_MRuby_Object(mrb_state* p_state)
        // OLD {
        // OLD     return mrb_obj_value(Data_Wrap_Struct(p_state, mrb_class_get(p_state, "Enemy_Stopper"), &Scripting::rtTSC_Scriptable, this));
        // OLD }

        // draw
        virtual void Draw(sf::RenderWindow& stage);

    protected:
        // save to XML node inherited
        virtual std::string Get_XML_Type_Name()
        {
            return "";
        }
    };

    /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

} // namespace TSC

#endif

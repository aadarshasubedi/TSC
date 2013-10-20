#include "mrb_textbox.h"
#include "../../../objects/text_box.h"
#include "../../../level/level.h"
#include "../../../core/sprite_manager.h"
#include "mrb_box.h"

/**
 * Class: TextBox
 *
 * Parent [Box](box.html)
 * {: .superclass}
 *
 * The classic way to display messages to the user
 * involves the creatin of a _Text Box_. When the
 * user jumps against this kind of box, a small dialog
 * window opens containing the previously set, possibly
 * multiline text. Note that with scripting you can
 * easily change the box’ text in the running level depending
 * on whatever condition you may find suitable.
 */

using namespace SMC;
using namespace SMC::Scripting;

// Extern
struct RClass* SMC::Scripting::p_rcText_Box = NULL;

/**
 * Method: TextBox::new
 *
 *   new() → a_text_box
 *
 * Creates a new TextBox with the default values.
 */
static mrb_value Initialize(mrb_state* p_state, mrb_value self)
{
	cText_Box* p_box = new cText_Box(pActive_Level->m_sprite_manager);
	DATA_PTR(self) = p_box;
	DATA_TYPE(self) = &rtSMC_Scriptable;

	// This is a generated object
	p_box->Set_Spawned(true);

	// Let SMC manage the memory
	pActive_Level->m_sprite_manager->Add(p_box);

	return self;
}

/**
 * Method: TextBox#text=
 *
 *   text=( str ) → str
 *
 * Specify the textbox’ text.
 *
 * #### Parameters
 * str
 * : The textbox’ new text. A possibly multiline string.
 */
static mrb_value Set_Text(mrb_state* p_state, mrb_value self)
{
	char* text = NULL;
	mrb_get_args(p_state, "z", &text);

	cText_Box* p_box = Get_Data_Ptr<cText_Box>(p_state, self);
	p_box->Set_Text(text);

	return mrb_str_new_cstr(p_state, text);
}

/**
 * Method: TextBox#text
 *
 *   text() → a_string
 *
 * Returns the textbox’ text.
 */
static mrb_value Get_Text(mrb_state* p_state, mrb_value self)
{
	cText_Box* p_box = Get_Data_Ptr<cText_Box>(p_state, self);
	return mrb_str_new_cstr(p_state, p_box->m_text.c_str());
}

void SMC::Scripting::Init_TextBox(mrb_state* p_state)
{
	p_rcText_Box = mrb_define_class(p_state, "TextBox", p_rcBox);
	MRB_SET_INSTANCE_TT(p_rcText_Box, MRB_TT_DATA);

	mrb_define_method(p_state, p_rcText_Box, "initialize", Initialize, MRB_ARGS_NONE());
	mrb_define_method(p_state, p_rcText_Box, "text=", Set_Text, MRB_ARGS_REQ(1));
	mrb_define_method(p_state, p_rcText_Box, "text", Get_Text, MRB_ARGS_NONE());
}
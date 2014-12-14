/*
	Yelo: Open Sauce SDK
		Halo 1 (CE) Edition

	See license\OpenSauce\Halo1_CE for specific license information
*/
#include "Common/Precompile.hpp"
#include "Interface/OpenSauceUI/GwenUI/ControlBuilders/c_control_builder_gwen_textbox.hpp"

#if !PLATFORM_IS_DEDI

#include <Gwen/Controls/TextBox.h>
#include <YeloLib/cseries/value_conversion.hpp>

#include "Interface/OpenSauceUI/resource_id.hpp"
#include "Interface/OpenSauceUI/Control/c_control_base.hpp"

namespace Yelo
{
	namespace Interface { namespace OpenSauceUI { namespace GwenUI { namespace ControlBuilders
	{
		DEFINE_PROPERTY_INTERFACE(textbox_editable,
			nullptr,
			[](Control::i_control& control, const Control::s_interface_value& value)
			{
				auto& gwen_control = GWEN_CTRL_REF(TextBox, control.GetControlPtr());

				gwen_control.SetEditable(value.m_bool);
			},
			[](Control::i_control& control, cstring value_string)
			{
				auto& gwen_control = GWEN_CTRL_REF(TextBox, control.GetControlPtr());

				bool value;
				ValueConversion::FromString(value_string, value);
				gwen_control.SetEditable(value);
			}
		);

		Gwen::Controls::Base* c_control_builder_gwen_textbox::CreateInstance(Gwen::Controls::Base* parent) const
		{
			return new Gwen::Controls::TextBox(parent);
		}

		void c_control_builder_gwen_textbox::SetDefaultValues(Gwen::Controls::Base* control) const
		{
			c_control_builder_gwen_label::SetDefaultValues(control);

			auto& gwen_control = GWEN_CTRL_REF(TextBox, control);
			gwen_control.SetEditable(true);
			gwen_control.SetText("");
		}

		void c_control_builder_gwen_textbox::GetPropertyInterfaces(Control::t_property_interface_map& property_interfaces) const
		{
			c_control_builder_gwen_label::GetPropertyInterfaces(property_interfaces);
			
			property_interfaces[RESOURCE_ID_DEBUG("Editable")] = GET_PROPERTY_INTERFACE(textbox_editable);
		}

		void c_control_builder_gwen_textbox::GetEventHandlers(Gwen::Controls::Base* control, Control::t_event_handler_map& event_handlers) const
		{
			c_control_builder_gwen_label::GetEventHandlers(control, event_handlers);
			
			auto& gwen_control = GWEN_CTRL_REF(TextBox, control);
			
			event_handlers[RESOURCE_ID_DEBUG("TextChanged")] = std::make_unique<c_event_handler_gwen>(gwen_control.onTextChanged);
			event_handlers[RESOURCE_ID_DEBUG("ReturnPressed")] = std::make_unique<c_event_handler_gwen>(gwen_control.onReturnPressed);
		}
	};};};};
};
#endif
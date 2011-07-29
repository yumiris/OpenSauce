/*
    Yelo: Open Sauce SDK

    Copyright (C) 2005-2010  Kornner Studios (http://kornner.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

namespace Yelo
{
	namespace Enums
	{
		enum hs_script_type
		{
			_hs_script_type_startup,
			_hs_script_type_dormant,
			_hs_script_type_continuous,
			_hs_script_type_static,
			_hs_script_type_stub,
			_hs_script_type_command_script,
			_hs_script_type,
		};

		enum hs_type : _enum
		{
			_hs_type_unparsed,
			_hs_type_special_form,
			_hs_type_function_name,
			_hs_type_passthrough,
			_hs_type_void,
			_hs_type_boolean,
			_hs_type_real,
			_hs_type_short,
			_hs_type_long,
			_hs_type_string,
			_hs_type_script,
			_hs_type_string_id,
			_hs_type_unit_seat_mapping,
			_hs_type_trigger_volume,
			_hs_type_cutscene_flag,
			_hs_type_cutscene_camera_point,
			_hs_type_cutscene_title,
			_hs_type_cutscene_recording,
			_hs_type_device_group,
			_hs_type_ai,
			_hs_type_ai_command_list,
			_hs_type_ai_command_script,
			_hs_type_ai_behavior,
			_hs_type_ai_orders,
			_hs_type_starting_profile,
			_hs_type_conversation,
			_hs_type_structure_bsp,
			_hs_type_navpoint,
			_hs_type_point_reference,
			_hs_type_style,
			_hs_type_hud_message,
			_hs_type_object_list,
			_hs_type_sound,
			_hs_type_effect,
			_hs_type_damage,
			_hs_type_looping_sound,
			_hs_type_animation_graph,
			_hs_type_object_definition,
			_hs_type_bitmap,
			_hs_type_shader,
			_hs_type_render_model,
			_hs_type_structure_definition,
			_hs_type_lightmap_definition,
			_hs_type_game_difficulty,
			_hs_type_team,
			_hs_type_actor_type,
			_hs_type_hud_corner,
			_hs_type_model_state,
			_hs_type_network_event,
			_hs_type_object,
			_hs_type_unit,
			_hs_type_vehicle,
			_hs_type_weapon,
			_hs_type_device,
			_hs_type_scenery,
			_hs_type_object_name,
			_hs_type_unit_name,
			_hs_type_vehicle_name,
			_hs_type_weapon_name,
			_hs_type_device_name,
			_hs_type_scenery_name,
			_hs_type,
		};
#define HS_TYPE(hstype) BOOST_JOIN(Yelo::Enums::_hs_type_,hstype)

	};

	namespace Scripting
	{
		typedef void (PLATFORM_API* hs_parse_proc)(int32 function_index, datum_index script_datum);
		typedef void (PLATFORM_API* hs_evaluate_proc)(int32 function_index, datum_index script_datum, bool* execute);

		typedef void* (API_FUNC* hs_yelo_function_proc)();
		typedef void* (API_FUNC* hs_yelo_function_with_params_proc)(void** arguments);


		// halo script function definition
		struct hs_function_definition
		{
			_enum return_type;
			uint16 flags; // padding in halo, special flags in project yellow
#if PLATFORM_IS_EDITOR || PLATFORM_ID == PLATFORM_H2_ALPHA
			cstring name;
#endif
			long_flags unknown; // TODO: was this in alpha?
#if PLATFORM_IS_EDITOR
			hs_parse_proc parse;
#endif
			hs_evaluate_proc evaluate;
#if PLATFORM_IS_EDITOR
			cstring info;
#endif
			cstring param_info;
			int16 paramc;
#pragma warning( push )
#pragma warning( disable : 4200 ) // nonstandard extension used : zero-sized array in struct/union, Cannot generate copy-ctor or copy-assignment operator when UDT contains a zero-sized array
			int16 params[];
#pragma warning( pop )
		};

		// halo script accessible value
		struct hs_global_definition
		{
#if PLATFORM_IS_EDITOR
			cstring name;
#endif
			_enum type;
			uint16 flags; // padding in halo, special flags in project yellow
			union {
				void* address;

				union {
					bool* _bool;
					real* _real;
					int16* _short;
					int32* _long;
					char* _string;
					datum_index _datum;
				}Value;
			};
		};
	};
};
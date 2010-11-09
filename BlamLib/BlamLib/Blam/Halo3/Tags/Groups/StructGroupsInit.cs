/*
    BlamLib: .NET SDK for the Blam Engine

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
﻿using System;
using BlamLib.TagInterface;

namespace BlamLib.Blam.Halo3
{
	partial class StructGroups
	{
		static StructGroups()
		{
			GroupsInitialize();
			play.Definition = new Tags.cache_file_resource_layout_table().State;
			mphp.Definition = new Tags.material_physics_properties_struct().State;
			msst.Definition = new Tags.materials_sweeteners_struct().State;
			snpl.Definition = new Tags.sound_playback_parameters_struct().State;
			snsc.Definition = new Tags.sound_scale_modifiers_struct().State;
			masd_sound.Definition = new Tags.sound_response_extra_sounds_struct().State;
			sszd.Definition = new Tags.static_spawn_zone_data_struct().State;
			MAPP.Definition = new Tags.mapping_function().State;
			uncs.Definition = new Tags.unit_camera_struct().State;
			usas.Definition = new Tags.unit_seat_acceleration_struct().State;
			uHnd.Definition = new Tags.unit_additional_node_names_struct().State;
			ubms.Definition = new Tags.unit_boarding_melee_struct().State;
			_1234.Definition = new Tags.unit_boost_struct().State;
			ulYc.Definition = new Tags.unit_lipsync_scales_struct().State;
			HVPH.Definition = new Tags.havok_vehicle_physics_struct().State;
			blod.Definition = new Tags.biped_lock_on_data_struct().State;
			cbsp.Definition = new Tags.global_collision_bsp_struct().State;
			csbs.Definition = new Tags.constraint_bodies_struct().State;
			aaim.Definition = new Tags.animation_aiming_screen_struct().State;

			MAgr.Definition = new Tags.animation_graph_resources_struct().State;

			MAgc.Definition = new Tags.animation_graph_contents_struct().State;

			spdf.Definition = new Tags.global_structure_physics_struct().State;

			snpr.Definition = new Tags.sound_promotion_parameters_struct().State;

			for (int x = 0; x < Groups.Count; x++)
				Groups[x].InitializeHandle(BlamVersion.Halo3, x, true);
		}
	};
};
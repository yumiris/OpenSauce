/*
	Yelo: Open Sauce SDK
		Halo 1 (CE) Edition

	See license\OpenSauce\Halo1_CE for specific license information
*/
#pragma once

#include <blamlib/cseries/cseries_base.hpp>
#include <blamlib/models/model_animation_definitions.hpp>
#include <blamlib/models/model_animations.hpp>
#include <blamlib/tag_files/tag_groups_base.hpp>
#include <blamlib/tag_files/tag_reference.h>
#include <yelolib/tag_files/tag_groups_markup.hpp>

namespace Yelo
{
	namespace Enums
	{
		enum {
			k_max_effect_references_per_graph = Enums::k_max_sound_references_per_graph_upgrade,
		};
		enum equipment_animation
		{
			k_number_of_equipment_animations,
		};

		enum projectile_animation
		{
			k_number_of_projectile_animations,
		};
	};

	namespace Animations
	{
#if PLATFORM_IS_EDITOR
		extern s_animation_list equipment_animation_list;
		extern s_animation_list projectile_animation_list;
#endif
	};

	namespace TagGroups
	{
		// The actual definition part of an OS-extended
		struct model_animation_graph_yelo_definition
		{
			TAG_FIELD(tag_reference, stock_animation, 'antr');
			TAG_PAD(tag_block, 13);
		}; ASSERT_SIZE(model_animation_graph_yelo_definition, 0xAC);
		struct model_animation_graph_yelo
		{
			enum { k_group_tag = 'magy' };

			model_animation_graph stock_graph;
			model_animation_graph_yelo_definition yelo_graph;
		};
	};
};
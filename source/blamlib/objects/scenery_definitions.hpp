/*
	Yelo: Open Sauce SDK

	See license\OpenSauce\OpenSauce for specific license information
*/
#pragma once

#include <blamlib/objects/object_definitions.hpp>

namespace Yelo
{
	namespace TagGroups
	{
		struct _scenery_definition
		{
			TAG_PAD(int32, 32);
		}; ASSERT_SIZE(_scenery_definition, 0x80);

		struct s_scenery_definition : s_object_definition
		{
			enum { k_group_tag = 'scen' };

			_scenery_definition scenery;
		};
	};
};
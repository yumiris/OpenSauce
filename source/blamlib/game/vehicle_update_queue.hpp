/*
	Yelo: Open Sauce SDK
		Halo 1 (CE) Edition

	See license\OpenSauce\Halo1_CE for specific license information
*/
#pragma once

#include <blamlib/game/simple_circular_queue.hpp>
#include <YeloLib/cseries/cseries_yelo_base.hpp>

namespace Yelo
{
	namespace Game
	{
		struct s_vehicle_update_queue_entry : TStructImpl(72)
		{
		};
		struct s_vehicle_update_queue
		{
			Memory::s_simple_circular_queue queue;
			s_vehicle_update_queue_entry* entries;	// 0x14, #30, user_data
		}; ASSERT_SIZE(s_vehicle_update_queue, 0x18);
	};
};
/*
	Yelo: Open Sauce SDK
		Halo 1 (CE) Edition

	See license\OpenSauce\Halo1_CE for specific license information
*/
#pragma once

// See this header for more "Engine pointer markup system" documentation
#include <Memory/MemoryInterfaceShared.hpp>

//////////////////////////////////////////////////////////////////////////
// Engine pointer markup system
//
// [ce_offset]		: (Client) Latest version's memory address
// [cededi_offset]	: (Dedi server) Latest version's memory address
//
//////////////////////////////////////////////////////////////////////////

#if PLATFORM_IS_DEDI
	#define PLATFORM_VALUE(ce_value, cededi_value) cededi_value
#elif PLATFORM_IS_USER
	#define PLATFORM_VALUE(ce_value, cededi_value) ce_value
#endif

#define ENGINE_DPTR(type, name, ce_offset, cededi_offset) \
	static type##** const pp##name = CAST_PTR(type##**, PLATFORM_VALUE(ce_offset,cededi_offset));								BOOST_STATIC_ASSERT( PLATFORM_VALUE(ce_offset,cededi_offset) != NULL );

#define ENGINE_PTR(type, name, ce_offset, cededi_offset) \
	static type##* const p##name = CAST_PTR(type##*, PLATFORM_VALUE(ce_offset,cededi_offset));									BOOST_STATIC_ASSERT( PLATFORM_VALUE(ce_offset,cededi_offset) != NULL );

#define FUNC_PTR(name, ce_offset, cededi_offset) enum FUNC_PTR_##name { PTR_##name = PLATFORM_VALUE(ce_offset,cededi_offset) }; BOOST_STATIC_ASSERT( GET_FUNC_PTR(name) != NULL );
#define DATA_PTR(name, ce_offset, cededi_offset) enum DATA_PTR_##name { PTR_##name = PLATFORM_VALUE(ce_offset,cededi_offset) }; BOOST_STATIC_ASSERT( GET_DATA_PTR(name) != NULL );

namespace Yelo
{
	namespace Memory
	{
		bool IsYeloEnabled();
		HMODULE YeloModuleHandle();

		uint32 CRC(uint32& crc_reference, const void* buffer, int32 size);
	};
};
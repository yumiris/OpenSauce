/*
	Yelo: Open Sauce SDK

	See license\OpenSauce\OpenSauce for specific license information
*/
// NOTE: NON-STANDARD ENGINE SOURCE FILE
#include "Common/Precompile.hpp"
#include <blamlib/Halo1/tag_files/tag_groups.hpp>

#include <YeloLib/Halo1/tag_files/string_id_yelo.hpp>

namespace Yelo
{
	namespace TagGroups
	{
		//////////////////////////////////////////////////////////////////////////
		// c_tag_field_scanner
		c_tag_field_scanner::c_tag_field_scanner(const tag_field* fields, void* block_element)
		{
			blam::tag_field_scan_state_new(m_state, fields, block_element);
			m_state.SetYeloScanState();
		}

		c_tag_field_scanner& c_tag_field_scanner::AddFieldType(Enums::field_type field_type)
		{
			blam::tag_field_scan_state_add_field_type(m_state, field_type);
			return *this;
		}

		c_tag_field_scanner& c_tag_field_scanner::AddAllFieldTypes()
		{
			memset(m_state.field_types, -1, sizeof(m_state.field_types));
			return *this;
		}

		bool c_tag_field_scanner::Scan()
		{
			return blam::tag_field_scan(m_state);
		}
		c_tag_field_scanner& c_tag_field_scanner::ScanToEnd()
		{
			for(int debug_scan_count = 0; Scan(); debug_scan_count++)
				assert(debug_scan_count <= Enums::k_maximum_field_byte_swap_codes);

			return *this;
		}

		bool c_tag_field_scanner::TagFieldIsStringId() const
		{
			return TagGroups::TagFieldIsStringId(m_state.field);
		}
		bool c_tag_field_scanner::TagFieldIsOldStringId() const
		{
			return TagGroups::TagFieldIsOldStringId(m_state.field);
		}

		bool c_tag_field_scanner::s_iterator::operator!=(const c_tag_field_scanner::s_iterator& other) const
		{
			if(other.IsEndHack())
				return !m_scanner->IsDone();
			else if(this->IsEndHack())
				return !other.m_scanner->IsDone();

			return m_scanner != other.m_scanner;
		}
	};

	namespace blam
	{
		bool PLATFORM_API tag_field_scan(TagGroups::s_tag_field_scan_state& state)
		{
			YELO_ASSERT( !state.done );

			const tag_field* field;
			do {
				field = &state.fields[state.field_index++];

				switch(field->type)
				{
				case Enums::_field_array_start:
					YELO_ASSERT( state.stack_index<Enums::k_tag_field_scan_stack_size );
					state.stack[state.stack_index].field_index = state.field_index;
					state.stack[state.stack_index].count = field->DefinitionCast<int16>(); // TODO: asm has this as int16, but all other array_start code treats the def as int32. is this cast really safe?
					YELO_ASSERT( state.stack[state.stack_index].count>0 );
					state.stack_index++;
					break;

				case Enums::_field_array_end:
					{
						YELO_ASSERT( state.stack_index>0 );
						auto& stack = state.stack[state.stack_index-1];

						if( --stack.count > 0 )
							state.field_index = stack.field_index;
						else
							--state.stack_index;
					}
					break;
				}

				state.field = field;
				state.field_address = state.block_element == nullptr ?
					nullptr :
					state.field_address = CAST_PTR(byte*,state.block_element) + state.field_end_offset;

				int field_size;
				switch(field->type)
				{
				case Enums::_field_string:
					field_size = TagGroups::StringFieldGetSize(field);
					break;

				case Enums::_field_pad:
				case Enums::_field_skip:
					field_size = field->DefinitionCast<int32>();
					break;

				default:
					field_size = CAST(int, TagGroups::k_tag_field_definitions[field->type].size);
					break;
				}
				state.field_size = CAST(int16, field_size);
				state.field_end_offset += field_size;

				if(field->type == Enums::_field_terminator)
				{
					state.done = true;
					return false;
				}

			} while( !BIT_VECTOR_TEST_FLAG32(state.field_types, field->type) );

			return true;
		}
	};
};
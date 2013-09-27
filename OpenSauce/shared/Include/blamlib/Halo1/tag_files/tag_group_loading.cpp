/*
	Yelo: Open Sauce SDK

	See license\OpenSauce\OpenSauce for specific license information
*/
#include "Common/Precompile.hpp"
#include <blamlib/Halo1/tag_files/tag_group_loading.hpp>

#include <blamlib/Halo1/memory/byte_swapping.hpp>
#include <blamlib/Halo1/memory/data.hpp>
#include <blamlib/Halo1/tag_files/tag_files.hpp>
#include <blamlib/Halo1/tag_files/tag_groups.hpp>

#include "Engine/EngineFunctions.hpp"

namespace Yelo
{
	namespace blam
	{
		static void tag_group_loading_add_non_loaded_tag(tag group_tag, cstring name)
		{
			// TODO
		}

		bool PLATFORM_API tag_data_load(void* block_element, tag_data* data, void* address)
		{
			YELO_ASSERT( block_element );
			YELO_ASSERT( data && data->definition );
			auto* data_definition = data->definition;

			if(address == nullptr)
			{
				YELO_ASSERT( data->address==nullptr );
				address = data->address = YELO_MALLOC(data->size, false);

				if(address == nullptr)
				{
					YELO_ERROR(_error_message_priority_warning, "couldn't allocate memory for #%d bytes %s data (load)",
						data->size, data_definition->name); // NOTE: added this warning

					return false;
				}
			}

			//YELO_ASSERT( data_definition ); // engine does this, but I don't see why

			if( !tag_file_read(data->stream_position, data->size, data->address) )
			{
				YELO_ERROR(_error_message_priority_warning, "couldn't read #%d bytes for %s data",
					data->size, data_definition->name); // NOTE: added this warning

				return false;
			}

			data_definition->byte_swap_proc(block_element, data->address, data->size);

			return true;
		}

		void PLATFORM_API tag_data_unload(void* block_element, tag_data* data)
		{
			YELO_ASSERT( data && data->definition );
			YELO_ASSERT( data->address );

			YELO_FREE( data->address );
			data->address = nullptr;
		}

		static bool PLATFORM_API tag_data_read_recursive(tag_data_definition* data_definition, void* block_element, tag_data* data, 
			int32 *position_reference, long_flags read_flags)
		{
			YELO_ASSERT( data_definition );
			void* data_address = nullptr;
			bool success = false;

			if( TEST_FLAG(data_definition->flags, Flags::_tag_data_never_streamed_bit)==0 ||
				TEST_FLAG(read_flags, Flags::_tag_load_for_editor_bit))
			{
				int size = data->size;
				if(size < 0 || size > data_definition->maximum_size)
				{
					YELO_ERROR(_error_message_priority_warning, "tag data '%s' too large. #%d not in [0,#%d]",
						data_definition->name, data->size, data_definition->maximum_size); // NOTE: added bounds info to warning
				}
				else if( !(data_address = YELO_MALLOC(size, false)) )
				{
					YELO_ERROR(_error_message_priority_warning, "couldn't allocate #%d tag data for '%s'",
						size, data_definition->name); // NOTE: added size info to warning
				}
				else if( !tag_data_load(block_element, data, data->address = data_address) )
				{
					// tag_data_load provides warnings, so we don't provide any here
					YELO_FREE( data->address );
					data_address = nullptr;
				}
				else
					success = true;
			}
			else
				success = true;

			*position_reference += data->size;
			data->address = data_address;
			return success;
		}

		static bool PLATFORM_API tag_reference_read_recursive(tag_reference_definition* definition, tag_reference* reference,
			int32 *position_reference, long_flags read_flags)
		{
			// NOTE: engine doesn't ASSERT anything

			if(reference->group_tag == 0) // handles cases were tag_reference fields were added to old useless padding
				reference->group_tag = definition->group_tag;

			reference->tag_index = datum_index::null;

			if( !(reference->name = CAST_PTR(char*,YELO_MALLOC(Enums::k_max_tag_name_length+1, false))) )
			{
				YELO_ERROR(_error_message_priority_warning, "couldn't allocate name memory for tag_reference @%p",
					definition); // NOTE: added this warning

				return false;
			}

			reference->name[0] = '\0';
			int name_length = reference->name_length;
			if(name_length == 0)
			{
				return true;
			}
			else if(name_length < 0 || name_length > Enums::k_max_tag_name_length)
			{
				YELO_ERROR(_error_message_priority_warning, "tag reference name too large #%d (this tag is corrupted).",
					name_length); // NOTE: added length info to warning

				return false;
			}
			else if( !tag_file_read(*position_reference, name_length+1, reference->name) )
			{
				YELO_ERROR(_error_message_priority_warning, "couldn't read #%d characters for @%p tag reference name",
					name_length+1, definition); // NOTE: added this warning

				return false;
			}
			else if(strlen(reference->name) != name_length)
			{
				// NOTE: engine uses the 'too large' warning for this case
				YELO_ERROR(_error_message_priority_warning, "tag reference name @%X doesn't match length #%d (this tag is corrupted).",
					*position_reference, name_length); // NOTE: added length info to warning

				return false;
			}

			_strlwr(reference->name);
			*position_reference += name_length+1;
			return true;
		}

		bool PLATFORM_API tag_block_read_recursive(const tag_block_definition* definition, tag_block* block,
			int32 *position_reference, long_flags read_flags);
		bool PLATFORM_API tag_block_read_children_recursive(const tag_block_definition *definition, void *address, int32 count, 
			int32 *position_reference, long_flags read_flags)
		{
			bool success = true;
			if(count == 0)
				return success;

			for(int x = 0; x < count; x++)
			{
				void* block_element = CAST_PTR(byte*,address) + (definition->element_size * x);
				for(auto field : TagGroups::c_tag_field_scanner(definition->fields, block_element)
					.AddFieldType(Enums::_field_block)
					.AddFieldType(Enums::_field_data)
					.AddFieldType(Enums::_field_tag_reference) )
				{
					bool read_result;

					switch(field.GetType())
					{
					case Enums::_field_data: read_result = 
						tag_data_read_recursive(field.DefinitionAs<tag_data_definition>(), block_element,
							field.As<tag_data>(), 
							position_reference, read_flags);
						break;

					case Enums::_field_block: read_result = 
						tag_block_read_recursive(field.DefinitionAs<tag_block_definition>(),
							field.As<tag_block>(), 
							position_reference, read_flags);
						break;

					case Enums::_field_tag_reference: read_result = 
						tag_reference_read_recursive(field.DefinitionAs<tag_reference_definition>(),
							field.As<tag_reference>(), 
							position_reference, read_flags);
						break;

					YELO_ASSERT_CASE_UNREACHABLE();
					}

					if(!read_result)
						success = read_result;
				}
			}

			return success;
		}

		static bool PLATFORM_API tag_block_postprocess(const tag_block_definition* definition, tag_block* block, 
			Enums::tag_postprocess_mode mode)
		{
			auto proc = definition->postprocess_proc;
			bool success = true;

			if(proc != nullptr)
			{
				for(int x = 0; x < block->count; x++)
				{
					void* element = tag_block_get_element(block, x);
					if(!proc(element, mode))
					{
						YELO_ERROR(_error_message_priority_warning, "failed to postprocess element #%d element for %s block",
							x, definition->name); // NOTE: added this warning message

						success = false;
					}
				}
			}

			return success;
		}
		static bool PLATFORM_API tag_block_read_recursive(const tag_block_definition* definition, tag_block* block,
			int32 *position_reference, long_flags read_flags)
		{
			int count = block->count;
			if(count < 0 || count > definition->maximum_element_count)
			{
				YELO_ERROR(_error_message_priority_warning, "%s block has invalid element count: #%d not in [0,#%d]",
					definition->name, count, definition->maximum_element_count);

				block->count = 0;
				return false;
			}
			else if(count == 0)
				return true;

			size_t elements_size = definition->element_size * count;
			if( !(block->address = YELO_MALLOC(elements_size, false)) )
			{
				YELO_ERROR(_error_message_priority_warning, "couldn't allocate memory for #%d element %s block",
					block->count, definition->name);

				return false;
			}

			if(!tag_file_read(*position_reference, elements_size, block->address))
			{
				YELO_ERROR(_error_message_priority_warning, "couldn't read #%d elements for %s block",
					block->count, definition->name); // NOTE: added count info to error

				return false;
			}

			byte_swap_data_explicit(definition->name, definition->element_size, definition->byte_swap_codes,
				block->count, block->address);

			*position_reference += elements_size;

			if( TEST_FLAG(definition->flags, Flags::_tag_block_has_children_bit) )
			{
				if(!tag_block_read_children_recursive(definition, block->address, block->count, 
					position_reference, read_flags))
					return false;
			}

			// engine doesn't do this logic, but they also don't use an enum for 'postprocess_mode'
			auto postprocess_mode = TEST_FLAG(read_flags, Flags::_tag_load_for_editor_bit) 
				? Enums::_tag_postprocess_mode_for_editor
				: Enums::_tag_postprocess_mode_for_runtime;
			if( !tag_block_postprocess(definition, block, postprocess_mode) )
				return false; // postprocess warns about failures, so we don't YELO_ERROR here

			return true;
		}

		static bool tag_reference_resolve_recursive(tag_reference_definition* definition, tag_reference* reference, long_flags read_flags)
		{
			datum_index tag_index = datum_index::null;
			bool success = true;

			if( reference->group_tag != NULL_HANDLE &&
				TEST_FLAG(read_flags, Flags::_tag_load_non_resolving_references_bit)==0 &&
				TEST_FLAG(definition->flags, Flags::_tag_reference_non_resolving_bit)==0
				)
			{
				if( strlen(reference->name) != 0 ||
					(TEST_FLAG(read_flags, Flags::_tag_load_for_editor_bit)==0 &&
					 TEST_FLAG(definition->flags, Flags::_tag_reference_unknown0_bit))
					)
				{
					tag_index = tag_load(reference->group_tag, reference->name, 0);
					success = !tag_index.IsNull();
				}
			}

			reference->tag_index = tag_index;
			return success;
		}
		static bool tag_references_resolve_recursive(tag_block_definition* definition, tag_block* block, long_flags read_flags)
		{
			bool success = true;

			for(void* block_element : *block)
			{
				for(auto field : TagGroups::c_tag_field_scanner(definition->fields, block_element)
					.AddFieldType(Enums::_field_block)
					.AddFieldType(Enums::_field_tag_reference) )
				{
					bool read_result;

					switch(field.GetType())
					{
					case Enums::_field_block: read_result = 
						tag_references_resolve_recursive(field.DefinitionAs<tag_block_definition>(),
							  field.As<tag_block>(), 
							  read_flags);
						break;

					case Enums::_field_tag_reference: read_result = 
						tag_reference_resolve_recursive(field.DefinitionAs<tag_reference_definition>(),
							field.As<tag_reference>(), 
							read_flags);
						break;

					YELO_ASSERT_CASE_UNREACHABLE();
					}

					if(!read_result)
						success = read_result;
				}
			}
			return success;
		}

		static bool verify_tag_string_field(TagGroups::c_tag_field_scanner::s_iterator_result field,
			tag_block_definition* block_definition)
		{
			auto& string = *field.As<tag_string>();
			if( strnlen_s(string, Enums::k_tag_string_length) == Enums::k_tag_string_length && 
				string[Enums::k_tag_string_length] != '\0' )
			{
				YELO_ERROR(_error_message_priority_warning, "fixed corrupt tag string field (%s).",
					field.GetName());

				string[Enums::k_tag_string_length] = '\0';
				return false;
			}

			// engine turns non-printable characters into underscores and nulls trailing spaces
			// TODO: shouldn't we note when we correct non-printable characters?
			for(size_t non_space_character_found = 0,
				x = strlen(string)-1; x >= 0; --x)
			{
				char& c = string[x];

				if(!isprint(c))
					c = '_';

				if(isspace(c))
				{
					if(!non_space_character_found)
						c = '\0';
				}
				else
					non_space_character_found = TRUE;
			}

			return true;
		}
		template<typename TEnum>
		static bool verify_enum_field(TagGroups::c_tag_field_scanner::s_iterator_result field,
			tag_block_definition* block_definition)
		{
			auto& value = *field.As<TEnum>();
			int32 count = field.DefinitionAs<string_list>()->count;
			if(value >= 0 && value < count)
				return true;

			YELO_ERROR(_error_message_priority_warning, "fixed corrupt tag enum field (%s) in %s.",
				field.GetName(), block_definition->name); // NOTE: added owner block name to info

			value = 0;
			return false;
		}
		template<typename TFlags>
		static bool verify_flags_field(TagGroups::c_tag_field_scanner::s_iterator_result field,
			tag_block_definition* block_definition)
		{
			auto& flags = *field.As<TFlags>();
			int32 bit_count = field.DefinitionAs<string_list>()->count;
			if(bit_count >= BIT_COUNT(TFlags) || (flags >> bit_count) == 0)
				return true;

			YELO_ERROR(_error_message_priority_warning, "fixed corrupt tag flags field (%s) in %s.",
				field.GetName(), block_definition->name); // NOTE: added owner block name to info

			flags &= MASK(bit_count);
			return false;
		}
		template<typename TIndex>
		static bool verify_block_index_field(TagGroups::c_tag_field_scanner::s_iterator_result field,
			tag_block_definition* block_definition)
		{
			auto& index = *field.As<TIndex>();
			auto* indexed_block_definition = field.DefinitionAs<tag_block_definition>();
			if(index >= NONE && index < indexed_block_definition->maximum_element_count)
				return true;

			YELO_ERROR(_error_message_priority_warning, "fixed corrupt tag block index field (%s) in %s (reset to NONE).",
				field.GetName(), block_definition->name); // NOTE: added owner block name to info

			index = NONE;
			return false;
		}
		static bool tag_block_verify_recursive(tag_block_definition* definition, tag_block* block)
		{
			bool valid = true;

			for(int x = 0; x < block->count; x++)
			{
				void* element = CAST_PTR(byte*,block->address) +
					(definition->element_size * x); // NOTE: engine doesn't use tag_block_get_element here

				for(auto field : TagGroups::c_tag_field_scanner(definition->fields, element)
					.AddAllFieldTypes() )
				{
					switch(field.GetType())
					{
					case Enums::_field_string: if(!verify_tag_string_field(field, definition)) valid = false;
						break;

					// NOTE: technically the engine treats enum fields as signed
					case Enums::_field_enum: if(!verify_enum_field<int16>(field, definition)) valid = false;
						break;

					// NOTE: engine only verified long_flags, we added support for the others
					case Enums::_field_long_flags: if(!verify_flags_field<long_flags>(field, definition)) valid = false;
						break;
					case Enums::_field_word_flags: if(!verify_flags_field<word_flags>(field, definition)) valid = false;
						break;
					case Enums::_field_byte_flags: if(!verify_flags_field<byte_flags>(field, definition)) valid = false;
						break;

					case Enums::_field_pad:
						std::memset(field.GetAddress(), 0, field.GetSize());
						break;

					case Enums::_field_block:
						if( !tag_block_verify_recursive(field.DefinitionAs<tag_block_definition>(), field.As<tag_block>()) )
							valid = false;
						break;

					// NOTE: engine doesn't verify block indices, we added support for them
					// TODO: should we enable this as an option via XML setting instead?
					case Enums::_field_short_block_index:	if(!verify_block_index_field<int16>(field, definition)) valid = false;
						break;
					case Enums::_field_long_block_index:	if(!verify_block_index_field<int32>(field, definition)) valid = false;
						break;
					}
				}
			}

			return valid;
		}

		static void tag_group_postprocess_instance(datum_index tag_index, Enums::tag_postprocess_mode mode)
		{
			for(const tag_group* group = tag_group_get(tag_get_group_tag(tag_index));
				group != nullptr;
				group = tag_group_get(group->parent_group_tag))
			{
				if(group->child_count > 0 && group->header_block_definition->postprocess_proc != nullptr)
				{
					group->header_block_definition->postprocess_proc(tag_get(NONE, tag_index), mode);
				}

				auto proc = group->postprocess_proc;
				if(proc != nullptr)
					proc(tag_index, mode);
			}
		}

		static void tag_instance_setup_groups_and_name(s_tag_instance* instance, const tag_group* group, cstring name)
		{
			strncpy_s(instance->filename, name, Enums::k_max_tag_name_length);
			instance->filename[Enums::k_max_tag_name_length] = '\0';
			instance->group_tag = group->group_tag;

			if((instance->parent_group_tags[0] = group->parent_group_tag) != NONE)
				instance->parent_group_tags[1] = tag_group_get(group->parent_group_tag)->parent_group_tag;
			else
				instance->parent_group_tags[1] = NONE;
		}

		datum_index PLATFORM_API tag_new_impl(tag group_tag, cstring name)
		{
			YELO_ASSERT(name);

			tag_group* group = tag_group_get(group_tag);
			// engine just asserts here: YELO_ASSERT(group);
			if(group == nullptr)
			{
				TagGroups::group_tag_to_string group_string = {group_tag};
				YELO_ERROR(_error_message_priority_assert, "the group tag '%s' does not exist (can't create '%s')",
					group_string.Terminate().TagSwap().str, name);
				return datum_index::null;
			}
			YELO_ASSERT( group->child_count==0 ); // TODO: don't we disable this assert in CheApe?

			datum_index tag_index = find_tag_instance(group_tag, name);
			if(!tag_index.IsNull())
			{
				YELO_ERROR(_error_message_priority_warning, "there is already a %s tag named '%s' loaded",
					group->name, name);
				return datum_index::null;
			}

			tag_index = TagGroups::TagInstances().New();
			if(tag_index.IsNull())
			{
				YELO_ERROR(_error_message_priority_warning, "there are no more free tag slots for new %s tag file '%s'.",
					group->name, name);
				return datum_index::null;
			}

			tag_block_definition* root_definition = group->header_block_definition;
			void* root_element = YELO_MALLOC(root_definition->element_size, false);
			do { // 'break' on error

				if(root_element == nullptr)
				{
					YELO_ERROR(_error_message_priority_warning, "couldn't allocate memory for new %s tag '%s'.",
						group->name, name);

					break;
				}

				auto* instance = TagGroups::TagInstances()[tag_index];
				tag_instance_setup_groups_and_name(instance, group, name);
				instance->is_read_only = tag_file_read_only(group_tag, name);
				instance->reload_index = datum_index::null;
				instance->root_block.count = 1;
				instance->root_block.address = root_element;
				instance->root_block.definition = root_definition;
				tag_block_generate_default_element(root_definition, root_element);

				int32 position = root_definition->element_size;
				if( !tag_block_read_children_recursive(root_definition, root_element, 1, 
						&position, FLAG(Flags::_tag_load_for_editor_bit)) )
				{
					YELO_ERROR(_error_message_priority_warning, "couldn't create new %s tag '%s'.",
						group->name, name); // NOTE: included group name

					break;
				}

				tag_group_postprocess_instance(tag_index, Enums::_tag_postprocess_mode_for_editor);
				return tag_index;

			} while(false);

			TagGroups::TagInstances().Delete(tag_index);

			if(root_element != nullptr)
				YELO_FREE( root_element );

			return datum_index::null;
		}

		datum_index PLATFORM_API tag_load_impl(tag group_tag, cstring name, long_flags flags)
		{
			YELO_ASSERT(name);

			tag_group* group = tag_group_get(group_tag);
			if(group == nullptr)
			{
				TagGroups::group_tag_to_string group_string = {group_tag};
				YELO_ERROR(_error_message_priority_warning, "the group tag '%s' does not exist (can't load '%s')",
					group_string.Terminate().TagSwap().str, name);
				return datum_index::null;
			}

			datum_index tag_index = find_tag_instance(group_tag, name);
			do {
				if(!tag_index.IsNull())
					break;

				bool is_readonly; uint32 checksum;
				if( !tag_file_open(group_tag, name, &is_readonly, &checksum, TEST_FLAG(flags, Flags::_tag_load_verify_exist_first_bit)) )
				{
					tag_group_loading_add_non_loaded_tag(group_tag, name);

					break;
				}

				tag_index = TagGroups::TagInstances().New();
				if(tag_index.IsNull())
				{
					YELO_ERROR(_error_message_priority_warning, "there are no more free tag slots.");

					break;
				}

				auto* instance = TagGroups::TagInstances()[tag_index];
				tag_instance_setup_groups_and_name(instance, group, name);

				instance->is_verified = TEST_FLAG(flags, Flags::_tag_load_for_editor_bit);
				// engine doesn't do this logic, but they also don't use an enum for 'postprocess_mode'
				auto postprocess_mode = TEST_FLAG(flags, Flags::_tag_load_for_editor_bit) 
					? Enums::_tag_postprocess_mode_for_editor
					: Enums::_tag_postprocess_mode_for_runtime;
				instance->reload_index = datum_index::null;
				instance->is_read_only = is_readonly;
				instance->file_checksum = checksum;
				instance->root_block.count = 1;
				instance->root_block.address = nullptr;
				instance->root_block.definition = group->header_block_definition;

				if( !tag_block_read_recursive(group->header_block_definition, &instance->root_block, 0, flags) &&
					!tag_references_resolve_recursive(group->header_block_definition, &instance->root_block, flags) )
				{
					YELO_ERROR(_error_message_priority_warning, "failed to load %s tag '%s'",
						group->name, name);

					TagGroups::TagInstances().Delete(tag_index);

					tag_index = datum_index::null;
					break;
				}

				if(	TEST_FLAG(flags, Flags::_tag_load_for_editor_bit) &&
					!tag_block_verify_recursive(group->header_block_definition, &instance->root_block) )
				{
					YELO_ERROR(_error_message_priority_warning, "the %s tag '%s' may be corrupt.",
						group->name, name);
				}

				tag_group_postprocess_instance(tag_index, postprocess_mode);
			}
			while(false);

			tag_files_flush();
			return tag_index;
		}

		datum_index PLATFORM_API tag_reload_impl(tag group_tag, cstring name)
		{
			tag_group* group = tag_group_get(group_tag);
			datum_index tag_index = datum_index::null;

			if( !TEST_FLAG(group->flags, Flags::_tag_group_reloadable_bit) )
				return tag_index;

			tag_index = find_tag_instance(group_tag, name);
			if(tag_index.IsNull())
				return tag_load(group_tag, name, 0);

			auto* instance = TagGroups::TagInstances()[tag_index];
			instance->is_orphan = true;

			// this instance was already reloaded once before, clear its old reloaded instance
			if(!instance->reload_index.IsNull())
				tag_unload(instance->reload_index);
			instance->reload_index = datum_index::null;

			datum_index reload_tag_index = tag_load(group_tag, name, 
				instance->is_verified ? FLAG(Flags::_tag_load_for_editor_bit) : 0);
			if(reload_tag_index.IsNull())
			{
				// TODO: shouldn't we log here?

				instance->is_orphan = false;
				return datum_index::null;
			}

			auto* reloaded_instance = TagGroups::TagInstances()[reload_tag_index];
			tag_block_delete_element(&instance->root_block, 0);
			std::memcpy(&instance->root_block, &reloaded_instance->root_block, sizeof(instance->root_block));
			reloaded_instance->is_reload = true;
			instance->is_orphan = false;
			instance->reload_index = reload_tag_index;

			return tag_index;
		}

		void PLATFORM_API tag_unload_impl(datum_index tag_index)
		{
			auto* instance = TagGroups::TagInstances()[tag_index];

			if(instance->root_block.count > 0 && !instance->is_reload)
				tag_block_delete_element(&instance->root_block, 0);

			TagGroups::TagInstances().Delete(tag_index);
		}
	};
};
/*
	Yelo: Open Sauce SDK
		Halo 1 (CE) Edition

	See license\OpenSauce\Halo1_CE for specific license information
*/
#include <halo/halo.h>
#include "Game/EngineFunctions.hpp"

#include <blam/cache/cache_files.hpp>
#include <blam/camera/director.hpp>
#include <blam/interface/hud_draw.hpp>
#include <blam/main/console.hpp>
#include <blam/main/main_structures.hpp>
#include <blam/math/periodic_functions.hpp>
#include <blam/memory/data.hpp>
#include <blam/models/model_animation_definitions.hpp>
#include <blam/objects/damage.hpp>
#include <blam/objects/objects.hpp>
#include <blam/tag_files/tag_groups.hpp>
#include <blam/ai/actor_structures.hpp>
#include <blam/units/unit_structures.hpp>
#include <blam/units/unit_script.hpp>

#include "Game/Console.hpp"
#include "Game/GameState.hpp"
#include "Game/ScriptLibrary.hpp"
#include "Networking/MDP.hpp"
#include "Objects/Objects.hpp"
#include <blam/tag_files/s_tag_iterator.h>
#include <Pointers/Game.EngineFunctions.hpp>

// comment the following to just use the parameters as-is 
// instead of copying to the stack. I believe there was some 
// concurrency issues originally which caused me to use local 
// buffers instead...
#define ENGINE_FUNCTIONS_USE_LOCAL

enum { k_engine_function_string_buffer_size = 512, };

#define API_FUNC_NAKED_END_NO_STACK_POP()	\
		__asm pop	ebp						\
		__asm retn							\
	}

namespace Yelo
{
	namespace Physics
	{
		struct s_collision_result;
	};

	namespace Players
	{
		struct s_unit_camera_info;
	};

	namespace TagGroups
	{
		struct s_bitmap_data;

		struct s_sound_permutation;

		struct predicted_resource;
	};

	namespace Engine
	{
		void GatherException(
			const void* data,
			int32 arg_0,
			int32 arg_4,
			int32 arg_8)
		{
			static const uintptr_t TEMP_CALL_ADDR = GET_FUNC_PTR(GATHER_EXCEPTION);

			GET_PTR(gather_exception_data) = data;

			__asm {
				push arg_8
				push arg_4
				#if PLATFORM_IS_DEDI
				mov ecx, arg_0
				#else
				push	arg_0
				#endif
				call TEMP_CALL_ADDR
				#if PLATFORM_IS_DEDI
				add esp, 4 * 2
				#else
				add		esp, 4 * 3
				#endif
			}
		}

		bool SetTextureSamplerStage(
			TagGroups::s_bitmap_data* bitmap,
			uint32 texture_stage)
		{
			// use this instead of TextureCacheRequestTexture where applicable as this function
			// tests whether the texture is already loaded and sets the texture sampler itself

			#if !PLATFORM_IS_DEDI
			static const uintptr_t TEMP_CALL_ADDR = GET_FUNC_PTR(RASTERIZER_SET_TEXTURE_BITMAP_DATA);

			_asm
			{
				mov		esi, bitmap
				push	texture_stage
				call	TEMP_CALL_ADDR
				add		esp, 4
			}
			#else
			return false;
			#endif
		}

		IDirect3DBaseTexture9** TextureCacheBitmapGetHardwareFormat(
			TagGroups::s_bitmap_data* bitmap,
			bool block_thread,
			bool load)
		{
			#if !PLATFORM_IS_DEDI
			static const uintptr_t FUNCTION = GET_FUNC_PTR(TEXTURE_CACHE_BITMAP_GET_HARDWARE_FORMAT);

			__asm {
				xor		eax, eax
				mov		al, load
				push	eax
				mov		al, block_thread
				push	eax
				mov		eax, bitmap
				call	FUNCTION
				add		esp, 4 * 2
			}
			#else
			return nullptr;
			#endif
		}

		bool SoundCacheRequestSound(
			TagGroups::s_sound_permutation* sound_perm,
			bool block_thread,
			bool load,
			bool reference)
		{
			#if !PLATFORM_IS_DEDI
			static const uintptr_t FUNCTION = GET_FUNC_PTR(SOUND_CACHE_SOUND_REQUEST);

			__asm {
				push	edi
				push	ebx

				xor		eax, eax
				mov		al, reference
				push	eax
				mov		al, load
				push	eax
				xor		ebx, ebx
				mov		bl, block_thread
				mov		edi, sound_perm
				call	FUNCTION
				add		esp, 4 * 2

				pop		ebx
				pop		edi
			}
			#else
			return false;
			#endif
		}


		void RasterizerAddResolution(
			uint32 width,
			uint32 height,
			uint32 refresh_rate)
		{
			#if !PLATFORM_IS_DEDI
			static const uintptr_t TEMP_CALL_ADDR = GET_FUNC_PTR(RESOLUTION_LIST_ADD_RESOLUTION);

			_asm {
				mov		ecx, width
				mov		eax, height
				mov		edx, refresh_rate
				push	edx
				push	ecx
				call	TEMP_CALL_ADDR
				add		esp, 4 * 2
			}
			#endif
		}

		int16 API_FUNC_NAKED AnimationPickRandomPermutation(
			long_enum render_or_affects_game_state,
			datum_index animation_graph_index,
			int32 animation_index)
		{
			static const uintptr_t TEMP_CALL_ADDR = GET_FUNC_PTR(ANIMATION_CHOOSE_RANDOM_PERMUTATION_INTERNAL);

			API_FUNC_NAKED_START()
				mov edx, animation_index
				mov eax, animation_graph_index
				push render_or_affects_game_state
				call TEMP_CALL_ADDR
				add esp, 4 * 1
			API_FUNC_NAKED_END(3)
		}

		void API_FUNC_NAKED GSMD5Digest(
			byte* input,
			unsigned int input_length,
			char output[33])
		{
			static const uintptr_t FUNCTION = GET_FUNC_PTR(GSMD5DIGEST);

			API_FUNC_NAKED_START()
				push output
				push input_length
				push input
				call FUNCTION
				API_FUNC_NAKED_END_CDECL(3);
		}

		bool CompareMD5(
			byte* input,
			unsigned int input_length,
			const char comparison_md5[33])
		{
			char input_md5[33];

			input_md5[0] = '\0';
			GSMD5Digest(input, input_length, input_md5);

			return _strnicmp(input_md5, comparison_md5, NUMBEROF(input_md5)) == 0;
		}

		namespace Cache
		{
			API_FUNC_NAKED bool FileReadRequest(
				/*datum_index tag_index,*/
				uint32 offset_,
				uint32 size_,
				void* buffer,
				const Yelo::Cache::s_cache_file_request_params& params,
				bool block,
				Enums::cache_file_request_source source)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(CACHE_FILE_READ_REQUEST);

				API_FUNC_NAKED_START()
					push esi
					xor eax, eax

					movzx eax, source
					push eax
					movzx eax, block
					push eax
					mov esi, params
					push buffer
					push size_
					push offset_
					call FUNCTION
					add esp, 4 * 5

					pop esi
				API_FUNC_NAKED_END(6); // not including the unused tag_index
			}
		};

		namespace Effects
		{
			void API_FUNC_NAKED NewOnObjectMarker(
				datum_index effect_definition_index,
				datum_index object_index,
				cstring marker_name)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(EFFECT_NEW_ON_OBJECT_MARKER);

				API_FUNC_NAKED_START()
					push esi
					push edi
					push ecx

					mov ecx, marker_name
					push ecx
					mov esi, object_index
					mov edi, effect_definition_index
					call FUNCTION
					add esp, 4 * 1

					pop ecx
					pop edi
					pop esi
				API_FUNC_NAKED_END(3)
			}
		};

		namespace HS
		{
			void ObjectListAdd(
				datum_index object_list,
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_LIST_ADD);

				if (object_list.IsNull())
					return;

				__asm {
					push object_index
					mov eax, object_list
					call FUNCTION
					add esp, 4 * 1
				}
			}
		};

		namespace Networking
		{
			void EncodeObjectDeletionMessage(
				datum_index object_index)
			{
				static const uintptr_t CALL_ADDR = GET_FUNC_PTR(OBJECT_DELETE_TO_NETWORK);

				if (object_index.IsNull())
					return;

				__asm {
					push edi

					mov edi, object_index
					call CALL_ADDR

					pop edi
				}
			}

			void EncodeHudChatNetworkData(
				int32 player_number,
				long_enum chat_type,
				wcstring msg)
			{
				static const uintptr_t CALL_ADDR = GET_FUNC_PTR(HUD_CHAT_TO_NETWORK);

				#if defined(ENGINE_FUNCTIONS_USE_LOCAL)
				wchar_t local[255];
				memset(local, 0, sizeof(local));
				wcscpy_s(local, msg);
				#else
				cstring local = msg;
				#endif

				__asm {
					mov eax, chat_type
					#if defined(ENGINE_FUNCTIONS_USE_LOCAL)
					lea edx, local
					#else
					mov		edx, local
					#endif
					push player_number // really a byte, but you can't push a byte!
					call CALL_ADDR
					add esp, 4 * 1
				}
			}

			#ifndef YELO_NO_NETWORK
			API_FUNC_NAKED int32 TranslateIndex(
				MessageDeltas::s_index_resolution_table& table,
				datum_index local_index)
			{
				// Function is actually part of the index_resolution module in objects/
				static const uintptr_t CALL_ADDR = GET_FUNC_PTR(INDEX_RESOLUTION_TABLE_TRANSLATE);

				API_FUNC_NAKED_START()
					push esi

					mov ecx, local_index
					mov esi, table
					call CALL_ADDR

					pop esi
				API_FUNC_NAKED_END(2)
			}

			int32 TranslateObject(
				datum_index local_object_index)
			{
				if (local_object_index.IsNull())
					return 0;

				return TranslateIndex(MessageDeltas::ObjectIndexParameters()->table, local_object_index);
			}

			int32 TranslatePlayer(
				datum_index local_player_index)
			{
				if (local_player_index.IsNull())
					return 0;

				return TranslateIndex(MessageDeltas::PlayerIndexParameters()->table, local_player_index);
			}

			int32 TranslatedIndexRegister(
				MessageDeltas::s_index_resolution_table& table,
				datum_index local_index)
			{
				if (local_index.IsNull())
					return 0;

				return TranslateIndex(table, local_index);
			}
			#endif
		};

		namespace Objects
		{
			API_FUNC_NAKED bool HeaderBlockAllocate(
				datum_index object_index,
				size_t block_reference_offset,
				size_t _size)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_HEADER_BLOCK_ALLOCATE);

				API_FUNC_NAKED_START()
					push _size
					push block_reference_offset
					mov eax, object_index
					call FUNCTION
					add esp, 4 * 2
				API_FUNC_NAKED_END(3)
			}

			API_FUNC_NAKED Yelo::Objects::s_object_data* IteratorNext(
				Yelo::Objects::s_object_iterator& iter)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_ITERATOR_NEXT);

				API_FUNC_NAKED_START()
					push iter
					call FUNCTION
					API_FUNC_NAKED_END_CDECL(1)
			}

			void PlacementDataNew(
				Yelo::Objects::s_object_placement_data& data,
				datum_index object_definition_index,
				datum_index owner_object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_PLACEMENT_DATA_NEW);

				if (object_definition_index.IsNull())
					return;

				__asm {
					push owner_object_index
					push object_definition_index
					mov eax, data
					call FUNCTION
					add esp, 4 * 2
				}
			}

			datum_index New(
				Yelo::Objects::s_object_placement_data& data)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_NEW);

				__asm {
					mov ecx, data
					call FUNCTION
				}
			}

			datum_index NewWithRole(
				Yelo::Objects::s_object_placement_data& data,
				long_enum /*Enums::networked_datum*/ role)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_NEW_WITH_ROLE);

				__asm {
					push role
					push data
					call FUNCTION
					add esp, 4 * 2
				}
			}

			API_FUNC_NAKED void Delete(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_DELETE);

				API_FUNC_NAKED_START()
					mov eax, object_index
					call FUNCTION
				API_FUNC_NAKED_END(1)
			}

			API_FUNC_NAKED void Attach(
				datum_index target_object_index,
				cstring target_marker_name,
				datum_index object_index,
				cstring marker_name)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_ATTACH_TO_MARKER);

				API_FUNC_NAKED_START()
					push esi
					push edi

					mov edi, marker_name
					mov esi, object_index
					mov ecx, target_marker_name
					push ecx
					mov edx, target_object_index
					push edx
					call FUNCTION
					add esp, 4 * 2

					pop edi
					pop esi
				API_FUNC_NAKED_END(4)
			}

			API_FUNC_NAKED void Detach(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_DETACH);

				API_FUNC_NAKED_START()
					push object_index
					call FUNCTION
					API_FUNC_NAKED_END_CDECL(1)
			}

			API_FUNC_NAKED int16 FindInSphere(
				Flags::objects_find_flags find_flags,
				long_flags object_type_flags,
				const s_scenario_location& location,
				const real_point3d& center,
				real radius,
				datum_index object_indices[],
				int16 maximum_object_indices)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECTS_FIND_IN_SPHERE);

				API_FUNC_NAKED_START()
					push maximum_object_indices
					push object_indices
					push radius
					push center
					push location
					push object_type_flags
					push find_flags
					call FUNCTION
					API_FUNC_NAKED_END_CDECL(7)
			}


			void StartInterpolation(
				datum_index object_index,
				int32 interpolation_ticks) // Until PY's netcode is finalized, this function don't mean shit for MP games (read: doesn't sync)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_START_INTERPOLATION);

				if (object_index.IsNull())
					return;

				__asm {
					mov eax, object_index
					mov edx, interpolation_ticks
					call FUNCTION
				}
			}

			void Reset(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_RESET);

				if (object_index.IsNull())
					return;

				__asm {
					push ebx

					mov ebx, object_index
					call FUNCTION

					pop ebx
				}
			}

			void ReconnectToMap(
				datum_index object_index,
				s_scenario_location* location_reference)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_RECONNECT_TO_MAP);

				if (object_index.IsNull())
					return;

				__asm {
					push location_reference
					push object_index
					call FUNCTION
					add esp, 4 * 2
				}
			}

			void DisconnectFromMap(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_DISCONNECT_FROM_MAP);

				if (object_index.IsNull())
					return;

				__asm {
					push object_index
					call FUNCTION
					add esp, 4 * 1
				}
			}

			real_point3d& GetOrigin(
				datum_index object_index,
				real_point3d& out_origin)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_GET_ORIGIN);

				if (object_index.IsNull())
					return out_origin;

				__asm {
					mov eax, out_origin
					mov ecx, object_index
					call FUNCTION
				}
			}

			void GetOrientation(
				datum_index object_index,
				real_vector3d* out_forward,
				real_vector3d* out_up)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_GET_ORIENTATION);

				if (object_index.IsNull())
					return;

				__asm {
					mov ecx, object_index
					mov eax, out_forward
					push out_up
					call FUNCTION
					add esp, 4 * 1
				}
			}

			s_scenario_location& GetLocation(
				datum_index object_index,
				s_scenario_location& out_location)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_GET_LOCATION);

				if (object_index.IsNull())
					return out_location;

				__asm {
					push edi

					mov edi, object_index
					mov eax, out_location
					call FUNCTION

					pop edi
				}
			}

			void SetPosition(
				datum_index object_index,
				real_point3d* new_pos,
				real_vector3d* new_forward,
				real_vector3d* new_up)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_SET_POSITION);

				if (object_index.IsNull())
					return;

				__asm {
					push edi

					mov edi, new_pos
					push new_up
					push new_forward
					push object_index
					call FUNCTION
					add esp, 4 * 3

					pop edi
				}
			}

			void SetPositionNetwork(
				datum_index object_index,
				real_point3d* new_pos)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_SET_POSITION_NETWORK);

				if (object_index.IsNull())
					return;

				__asm {
					push edi
					push esi

					mov edi, object_index
					mov esi, new_pos
					call FUNCTION

					pop esi
					pop edi
				}
			}

			bool RestoreBody(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_RESTORE_BODY);

				if (object_index.IsNull())
					return false;

				__asm {
					mov eax, object_index
					call FUNCTION
				}
			}

			void DepleteBody(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_DEPLETE_BODY);

				if (object_index.IsNull())
					return;

				__asm {
					mov eax, object_index
					call FUNCTION
				}
			}

			void DepleteShield(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_DEPLETE_SHIELD);

				if (object_index.IsNull())
					return;

				__asm {
					push edi

					mov edi, object_index
					call FUNCTION

					pop edi
				}
			}

			void DoubleChargeShield(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_DOUBLE_CHARGE_SHIELD);

				if (object_index.IsNull())
					return;

				__asm {
					mov eax, object_index
					call FUNCTION
				}
			}

			void ObjectCauseDamage(
				Yelo::Objects::s_damage_data& damage_data,
				datum_index object_index,
				int32 node_index,
				int32 region_index,
				int32 damage_materials_index,
				real_vector3d* normal)
			{
				typedef void (PLATFORM_API *object_cause_damage_t)(
					Yelo::Objects::s_damage_data&,
					datum_index,
					int32,
					int32,
					int32,
					real_vector3d*);
				static const object_cause_damage_t FUNCTION = CAST_PTR(object_cause_damage_t, GET_FUNC_PTR(OBJECT_CAUSE_DAMAGE));

				FUNCTION(damage_data, object_index, node_index, region_index, damage_materials_index, normal);
			}

			void DefinitionPredict(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_GET_CAMERA_POSITION);

				if (object_index.IsNull())
					return;

				__asm {
					mov eax, object_index
					call FUNCTION
				}
			}

			API_FUNC_NAKED void SetScale(
				datum_index object_index,
				real scale,
				int32 ticks)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_SET_SCALE);

				API_FUNC_NAKED_START()
					push ticks
					push scale
					mov eax, object_index
					call FUNCTION
					add esp, 4 * 2
				API_FUNC_NAKED_END(3)
			}

			void ObjectOrient(
				datum_index unit_index,
				int32 cutscene_flag_index,
				bool teleport,
				bool set_facing)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(HS_OBJECT_ORIENT);

				if (unit_index.IsNull())
					return;

				__asm {
					movzx eax, set_facing
					push eax
					movzx eax, teleport
					push eax
					mov eax, cutscene_flag_index
					push unit_index
					call FUNCTION
					add esp, 4 * 3
				}
			}

			void UnitGetCameraPosition(
				datum_index unit_index,
				real_point3d* out_position)
			{
				#if !PLATFORM_IS_DEDI
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_GET_CAMERA_POSITION);

				if (unit_index.IsNull()) return;

				__asm {
					push	edi

					mov		edi, out_position
					mov		ecx, unit_index
					call	FUNCTION

					pop		edi
				}
				#else
				memset(out_position, 0, sizeof(*out_position));
				#endif
			}

			void UnitSetAnimation(
				datum_index unit_index,
				datum_index animation_graph_index,
				int32 animation_index)
			{
				// Code is in the units.c code region but only touches s_object_datum data, so technically unit_index 
				// *could* safely be an generic object_index.
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_SET_ANIMATION);

				if (unit_index.IsNull())
					return;

				__asm {
					push animation_index
					push animation_graph_index
					mov eax, unit_index
					call FUNCTION
					add esp, 4 * 2
				}
			}

			API_FUNC_NAKED int16 UnitGetCustomAnimationTime(
				datum_index unit_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_GET_CUSTOM_ANIMATION_TIME);

				API_FUNC_NAKED_START()
					mov eax, unit_index
					call FUNCTION
				API_FUNC_NAKED_END(1)
			}

			API_FUNC_NAKED bool UnitCanEnterSeat(
				datum_index unit_index,
				datum_index vehicle_index,
				int32 vehicle_seat_index,
				datum_index* unit_in_seat)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_CAN_ENTER_SEAT);

				API_FUNC_NAKED_START()
					push unit_in_seat
					push vehicle_seat_index
					mov edx, vehicle_index
					mov eax, unit_index
					call FUNCTION
					add esp, 4 * 2
				API_FUNC_NAKED_END(4)
			}

			API_FUNC_NAKED bool UnitEnterSeat(
				datum_index unit_index,
				datum_index vehicle_index,
				int32 vehicle_seat_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_ENTER_SEAT);

				API_FUNC_NAKED_START()
					push vehicle_seat_index
					push vehicle_index
					mov eax, unit_index
					call FUNCTION
					add esp, 4 * 2
				API_FUNC_NAKED_END(3)
			}

			API_FUNC_NAKED void UnitExitVehicle(
				datum_index unit_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_EXIT_VEHICLE);

				API_FUNC_NAKED_START()
					mov eax, unit_index
					call FUNCTION
				API_FUNC_NAKED_END(1)
			}

			API_FUNC_NAKED void UnitOpen(
				datum_index unit_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_OPEN);

				API_FUNC_NAKED_START()
					mov eax, unit_index
					call FUNCTION
				API_FUNC_NAKED_END(1)
			}

			API_FUNC_NAKED void UnitClose(
				datum_index unit_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_CLOSE);

				API_FUNC_NAKED_START()
					mov eax, unit_index
					call FUNCTION
				API_FUNC_NAKED_END(1)
			}

			API_FUNC_NAKED int16 UnitFindNearbySeat(
				datum_index unit_index,
				datum_index vehicle_index,
				int16& seat_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_FIND_NEARBY_SEAT);

				API_FUNC_NAKED_START()
					push seat_index
					push vehicle_index
					push unit_index
					call FUNCTION
					add esp, 4 * 3
				API_FUNC_NAKED_END(3)
			}

			API_FUNC_NAKED void UnitExitSeatEnd(
				datum_index unit_index,
				bool unk2,
				bool unk3,
				bool unk4)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNIT_EXIT_SEAT_END);

				API_FUNC_NAKED_START()
					movzx eax, unk4
					push eax
					movzx eax, unk3
					push eax
					movzx eax, unk2
					push eax
					push unit_index
					call FUNCTION
					add esp, 4 * 4
				API_FUNC_NAKED_END(4)
			}

			API_FUNC_NAKED real GetLevelOfDetailPixels(
				datum_index object_index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_GET_LEVEL_OF_DETAIL_PIXELS);

				API_FUNC_NAKED_START()
					mov eax, object_index
					call FUNCTION
				API_FUNC_NAKED_END(1)
			}

			API_FUNC_NAKED void ObjectRenderStateRefresh(
				datum_index object_render_state_index,
				datum_index object_index,
				real level_of_detail_pixels,
				int arg4)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(OBJECT_RENDER_STATE_REFRESH);

				API_FUNC_NAKED_START()
					push arg4
					push level_of_detail_pixels
					push object_index
					push object_render_state_index
					call FUNCTION
					add esp, 4 * 4
				API_FUNC_NAKED_END(4)
			}
		};

		namespace Players
		{
			datum_index IndexFromUnitIndex(
				datum_index unit_index)
			{
				static const uintptr_t TEMP_CALL_ADDR = GET_FUNC_PTR(PLAYER_INDEX_FROM_UNIT_INDEX);

				if (unit_index.IsNull())
					return datum_index::null;

				__asm {
					push unit_index
					call TEMP_CALL_ADDR
					add esp, 4 * 1
				}
			}

			bool Teleport(
				datum_index player_index,
				const real_point3d& position,
				datum_index source_unit_index)
			{
				static const uintptr_t TEMP_CALL_ADDR = GET_FUNC_PTR(PLAYER_TELEPORT);

				if (player_index.IsNull())
					return false;

				__asm {
					push position
					push source_unit_index
					push player_index
					call TEMP_CALL_ADDR
					add esp, 4 * 3
				}
			}


			static void player_screen_effect(
				datum_index player_index,
				uint32 func)
			{
				#if !PLATFORM_IS_DEDI
				__asm {
					mov		edx, player_index
					mov		eax, func
					call	eax
				}
				#endif
			}

			void ScreenEffectOvershield(
				datum_index player_index)
			{
				#if !PLATFORM_IS_DEDI
				if (player_index.IsNull()) return;
				player_screen_effect(player_index, GET_FUNC_PTR(PLAYER_OVER_SHIELD_SCREEN_EFFECT));
				#endif
			}

			void ScreenEffectCamo(
				datum_index player_index)
			{
				#if !PLATFORM_IS_DEDI
				if (player_index.IsNull()) return;
				player_screen_effect(player_index, GET_FUNC_PTR(PLAYER_CAMO_SCREEN_EFFECT));
				#endif
			}

			void ScreenEffectHealth(
				datum_index player_index)
			{
				#if !PLATFORM_IS_DEDI
				if (player_index.IsNull()) return;
				player_screen_effect(player_index, GET_FUNC_PTR(PLAYER_HEALTH_PACK_SCREEN_EFFECT));
				#endif
			}

			datum_index FindClosestPlayerIndex(
				datum_index player_index)
			{
				#if !PLATFORM_IS_DEDI
				static uint32 TEMP_CALL_ADDR = GET_FUNC_PTR(FIND_CLOSEST_PLAYER_INDEX);

				if (player_index.IsNull()) return datum_index::null;

				__asm {
					push	player_index
					call	TEMP_CALL_ADDR
					add		esp, 4 * 1
				}
				#else
				return datum_index::null;
				#endif
			}
		};

		namespace Scenario
		{
			bool TriggerVolumeTestPoint(
				int32 trigger_volume_index,
				const real_point3d& point)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(SCENARIO_TRIGGER_VOLUME_TEST_POINT);

				__asm {
					mov ecx, point
					mov eax, trigger_volume_index
					call FUNCTION
				}
			}

			bool TriggerVolumeTestObject(
				int32 trigger_volume_index,
				datum_index object)
			{
				if (object.IsNull())
					return false;

				Yelo::Objects::s_object_data* obj = Yelo::Objects::ObjectHeader()[object]->_object;

				return TriggerVolumeTestPoint(trigger_volume_index, obj->center);
			}
		};

		namespace TagGroups
		{
			void PredictResources(
				TagBlock<Yelo::TagGroups::predicted_resource>& resources_block)
			{
				#if !PLATFORM_IS_DEDI // dedi doesn't need to predict resources since it doesn't render bitmaps or play sounds
				static const uintptr_t FUNCTION = GET_FUNC_PTR(PHYSICAL_MEMORY_MAP_PREDICT_RESOURCES);

				__asm {
					push	esi

					mov		esi, resources_block
					call	FUNCTION

					pop		esi
				}
				#endif
			}

			datum_index TagLoaded(
				tag group_tag,
				cstring name)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(TAG_LOADED);

				#if defined(ENGINE_FUNCTIONS_USE_LOCAL)
				char local[256];
				memset(local, 0, sizeof(local));
				strcpy_s(local, name);
				#else
				cstring local = name;
				#endif

				__asm {
					push edi

					#if defined(ENGINE_FUNCTIONS_USE_LOCAL)
					lea eax, local
					#else
					mov		eax, local
					#endif
					push eax
					mov edi, group_tag
					call FUNCTION
					add esp, 4 * 1

					pop edi
				}
			}

			wcstring UnicodeStringListGetString(
				datum_index ustr,
				int32 index)
			{
				static const uintptr_t FUNCTION = GET_FUNC_PTR(UNICODE_STRING_LIST_GET_STRING);

				__asm {
					mov edx, index
					mov ecx, ustr
					call FUNCTION
				}
			}
		};
	};

	//////////////////////////////////////////////////////////////////////////
	// ai
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// actors.c
		API_FUNC_NAKED void PLATFORM_API actor_delete(
			const datum_index actor_index,
			const bool is_dead)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_DELETE);

			API_FUNC_NAKED_START()
				pushad

				movzx eax, is_dead
				push eax
				mov ebx, actor_index
				call FUNCTION
				add esp, 4

				popad
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API actor_update(
			const datum_index actor_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_UPDATE);

			API_FUNC_NAKED_START()
				push eax
				push ecx
				push edx
				push esi

				mov esi, actor_index
				call FUNCTION

				pop esi
				pop edx
				pop ecx
				pop eax
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API actor_customize_unit(
			const datum_index actor_variant,
			const datum_index unit_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_CUSTOMIZE_UNIT);

			API_FUNC_NAKED_START()
				push eax
				push ecx
				push edx

				mov eax, actor_variant
				push unit_index
				call FUNCTION
				add esp, 4

				pop edx
				pop ecx
				pop eax
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API actor_set_active(
			const datum_index actor_index,
			const bool active)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_SET_ACTIVE);

			API_FUNC_NAKED_START()
				push eax
				push edi

				xor eax, eax
				mov al, active
				mov edi, actor_index
				call FUNCTION

				pop edi
				pop eax
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API actor_set_dormant(
			const datum_index actor_index,
			const bool dormant)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_SET_DORMANT);

			API_FUNC_NAKED_START()
				pushad

				xor ebx, ebx
				mov bl, dormant
				mov eax, actor_index
				call FUNCTION

				popad
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API actor_delete_props(
			const datum_index actor_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_DELETE_PROPS);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API actor_freeze(
			const datum_index actor_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_FREEZE);

			API_FUNC_NAKED_START()
				push eax
				push ecx
				push edx

				mov eax, actor_index
				call FUNCTION

				pop edx
				pop ecx
				pop eax
				pop ebp
			API_FUNC_NAKED_END_()
		}

		void PLATFORM_API actor_braindead(
			const datum_index actor_index,
			const bool braindead)
		{
			const auto actor_data = AI::Actors()[actor_index];

			if (braindead)
			{
				actor_data->current_state = AI::e_actor_default_state::none;
				actor_data->state.action = AI::e_actor_action::none;

				actor_delete_props(actor_index);
				actor_freeze(actor_index);
				actor_set_dormant(actor_index, false);
			}
			else
			{
				if (actor_data->current_state == AI::e_actor_default_state::none)
				{
					actor_data->current_state = AI::e_actor_default_state::alert;
				}
			}
		}

		API_FUNC_NAKED datum_index PLATFORM_API actor_create_for_unit(
			const bool is_swarm,
			const datum_index unit_index,
			const datum_index actor_variant,
			const datum_index encounter_index,
			const int32 squad_index,
			const int32 arg7,
			const int32 arg6,
			const bool magic_sight_after_timer,
			const AI::e_actor_default_state::type_t initial_state,
			const AI::e_actor_default_state::type_t return_state,
			const int32 command_list_index,
			const int32 sequence_id)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_CREATE_FOR_UNIT);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API actor_action_change(
			const datum_index actor_index,
			const AI::e_actor_action::type_t new_action_type,
			const byte* const new_action_data)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(ACTOR_ACTION_CHANGE);

			_asm jmp FUNCTION;
		}

		//////////////////////////////////////////////////////////////////////////
		// ai_script.c
		API_FUNC_NAKED void PLATFORM_API ai_scripting_attach_free(
			const datum_index unit_index,
			const datum_index actor_variant_definition_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(AI_SCRIPTING_ATTACH_FREE);

			API_FUNC_NAKED_START()
				push eax

				mov eax, actor_variant_definition_index
				push unit_index
				call FUNCTION
				add esp, 4

				pop eax
				pop ebp
			API_FUNC_NAKED_END_()
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// bink
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// bink_playback.c
		API_FUNC_NAKED void PLATFORM_API bink_playback_start(
			cstring bik_path)
		{
			#if !PLATFORM_IS_DEDI
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(BINK_PLAYBACK_START);

			API_FUNC_NAKED_START()
				push	bik_path
				call	FUNCTION
				add		esp, 4 * 1
			API_FUNC_NAKED_END_NO_STACK_POP()
			#else
			API_FUNC_NAKED_START_()
			API_FUNC_NAKED_END_()
			#endif
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// cache
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// cache_files_windows.c
		bool PLATFORM_API cache_file_read_request(
			/*datum_index tag_index,*/
			uint32 offset,
			uint32 size,
			void* buffer,
			const Cache::s_cache_file_request_params& params,
			bool block,
			Enums::cache_file_request_source source)
		{
			return Engine::Cache::FileReadRequest(offset, size, buffer, params, block, source);
		}

		//////////////////////////////////////////////////////////////////////////
		// pc_sound_cache.c
		bool PLATFORM_API sound_cache_sound_request(
			TagGroups::s_sound_permutation* sound_perm,
			bool block_thread,
			bool load,
			bool reference)
		{
			return Engine::SoundCacheRequestSound(sound_perm, block_thread, load, reference);
		}

		//////////////////////////////////////////////////////////////////////////
		// pc_texture_cache.c
		IDirect3DBaseTexture9** PLATFORM_API texture_cache_bitmap_get_hardware_format(
			TagGroups::s_bitmap_data* bitmap,
			// block
			bool block_thread,
			bool load)
		{
			return Engine::TextureCacheBitmapGetHardwareFormat(bitmap, block_thread, load);
		}

		//////////////////////////////////////////////////////////////////////////
		// physical_memory_map.c
		void predicted_resources_precache(
			TagBlock<TagGroups::predicted_resource>& resources)
		{
			return Engine::TagGroups::PredictResources(resources);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// camera
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// director.c
		API_FUNC_NAKED void PLATFORM_API director_save_camera()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DIRECTOR_SAVE_CAMERA);

			__asm {
				call FUNCTION
				retn
			}
		}

		API_FUNC_NAKED void PLATFORM_API director_load_camera()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DIRECTOR_LOAD_CAMERA);

			__asm {
				call FUNCTION
				retn
			}
		}

		API_FUNC_NAKED int16 PLATFORM_API director_desired_perspective(
			const datum_index unit_index,
			Enums::game_perspective& desired_perspective)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DIRECTOR_DESIRED_PERSPECTIVE);

			API_FUNC_NAKED_START()
				mov ecx, unit_index
				mov edx, [desired_perspective]
				push edx
				call FUNCTION
				add esp, 4
				pop ebp
			API_FUNC_NAKED_END_()
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// game
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// cheats.c
		API_FUNC_NAKED void PLATFORM_API cheat_all_weapons()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CHEAT_ALL_WEAPONS);

			__asm {
				call FUNCTION
				retn
			}
		}

		API_FUNC_NAKED void PLATFORM_API cheat_spawn_warthog()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CHEAT_SPAWN_WARTHOG);

			__asm {
				call FUNCTION
				retn
			}
		}

		API_FUNC_NAKED void PLATFORM_API cheat_teleport_to_camera()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CHEAT_TELEPORT_TO_CAMERA);

			__asm {
				call FUNCTION
				retn
			}
		}

		API_FUNC_NAKED void PLATFORM_API cheat_active_camouflage()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CHEAT_ACTIVE_CAMOFLAGE);

			__asm {
				call FUNCTION
				retn
			}
		}

		API_FUNC_NAKED void PLATFORM_API cheat_active_camouflage_local_player()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CHEAT_ACTIVE_CAMOFLAGE_LOCAL_PLAYER);

			__asm {
				call FUNCTION
				retn
			}
		}

		API_FUNC_NAKED datum_index PLATFORM_API cheat_local_player()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CHEAT_PLAYER_INDEX);

			__asm {
				call FUNCTION
				retn
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// game_allegiance.c
		bool PLATFORM_API game_team_is_enemy(
			long_enum team,
			long_enum team_to_test)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(GAME_TEAM_IS_ENEMY);

			__asm {
				mov ecx, team_to_test
				mov edx, team
				call FUNCTION
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// game_engine.c
		datum_index PLATFORM_API find_closest_player_index(
			datum_index player_index)
		{
			return Engine::Players::FindClosestPlayerIndex(player_index);
		}


		API_FUNC_NAKED void PLATFORM_API game_engine_rasterize_message(
			wcstring message,
			real alpha)
		{
			#if !PLATFORM_IS_DEDI
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(GAME_ENGINE_RASTERIZE_MESSAGE);

			API_FUNC_NAKED_START()
				push	alpha
				push	message
				call	FUNCTION
				add		esp, 4 * 2
			API_FUNC_NAKED_END_NO_STACK_POP()
			#else
			API_FUNC_NAKED_START_()
			API_FUNC_NAKED_END_()
			#endif
		}

		//////////////////////////////////////////////////////////////////////////
		// game_engine_multiplayer_sounds.c
		API_FUNC_NAKED void PLATFORM_API game_engine_play_multiplayer_sound(
			datum_index player_index,
			_enum multiplayer_sound_index,
			bool should_replicate)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(GAME_ENGINE_PLAY_MULTIPLAYER_SOUND);

			API_FUNC_NAKED_START()
				push esi
				push edi

				movzx esi, should_replicate
				push esi
				movsx esi, multiplayer_sound_index
				mov edi, player_index
				call FUNCTION
				add esp, 4 * 1

				pop edi
				pop esi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		//////////////////////////////////////////////////////////////////////////
		// players.c
		API_FUNC_NAKED void PLATFORM_API player_examine_nearby_vehicle(
			datum_index player_index,
			datum_index vehicle_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(PLAYER_EXAMINE_NEARBY_VEHICLE);

			API_FUNC_NAKED_START()
				push vehicle_index
				push player_index
				call FUNCTION
				add esp, 4 * 2
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		API_FUNC_NAKED void PLATFORM_API player_set_action_result(
			datum_index player_index,
			datum_index action_object_index,
			int32 action_result,
			int32 action_seat_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(PLAYER_SET_ACTION_RESULT);

			API_FUNC_NAKED_START()
				push ebx

				push action_seat_index
				push action_result
				mov ebx, action_object_index
				mov eax, player_index
				call FUNCTION
				add esp, 4 * 2

				pop ebx
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		API_FUNC_NAKED void PLATFORM_API player_set_action_result_to_network(
			datum_index player_index,
			datum_index action_object_index,
			int32 action_result_type,
			int32 action_result,
			int32 action_seat_index,
			datum_index next_weapon_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(PLAYER_SET_ACTION_RESULT_TO_NETWORK);

			API_FUNC_NAKED_START()
				push edi

				push next_weapon_index
				push action_seat_index
				push action_result
				push action_result_type
				mov edi, action_object_index
				mov ecx, player_index
				call FUNCTION
				add esp, 4 * 4

				pop edi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}


		datum_index PLATFORM_API player_index_from_unit_index(
			datum_index unit_index)
		{
			return Engine::Players::IndexFromUnitIndex(unit_index);
		}

		bool PLATFORM_API player_teleport(
			datum_index player_index,
			datum_index source_unit_index,
			const real_point3d& position)
		{
			return Engine::Players::Teleport(player_index, position, source_unit_index);
		}

		void PLATFORM_API player_over_shield_screen_effect(
			datum_index player_index)
		{
			Engine::Players::ScreenEffectOvershield(player_index);
		}

		void PLATFORM_API player_active_camo_screen_effect(
			datum_index player_index)
		{
			Engine::Players::ScreenEffectCamo(player_index);
		}

		void PLATFORM_API player_health_pack_screen_effect(
			datum_index player_index)
		{
			Engine::Players::ScreenEffectHealth(player_index);
		}

		//////////////////////////////////////////////////////////////////////////
		// player_control.c
		API_FUNC_NAKED void PLATFORM_API player_control_get_unit_camera_info(
			const int16 player_index,
			Players::s_unit_camera_info& camera_info)
		{
			#if !PLATFORM_IS_DEDI
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(PLAYER_CONTROL_GET_UNIT_CAMERA_INFO);

			API_FUNC_NAKED_START()
				push	eax
				push	ecx
				push	edx
				push	esi

				mov		ax, player_index
				mov		esi, camera_info
				call	FUNCTION
				
				pop		esi
				pop		edx
				pop		ecx
				pop		eax
				pop		ebp
			API_FUNC_NAKED_END_()
			#else
			API_FUNC_NAKED_START()
				pop ebp
			API_FUNC_NAKED_END_()
			#endif
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// hs
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// hs_library_external.c
		void PLATFORM_API hs_object_orient(
			datum_index object_index,
			int32 cutscene_flag_index,
			bool teleport,
			bool set_facing)
		{
			Engine::Objects::ObjectOrient(object_index, cutscene_flag_index, teleport, set_facing);
		}


		void PLATFORM_API hs_effect_new_from_object_marker(
			datum_index effect_definition_index,
			datum_index object_index,
			cstring marker_name)
		{
			Engine::Effects::NewOnObjectMarker(effect_definition_index, object_index, marker_name);
		}

		//////////////////////////////////////////////////////////////////////////
		// object_lists.c
		void PLATFORM_API object_list_add(
			datum_index object_list_index,
			datum_index object_index)
		{
			Engine::HS::ObjectListAdd(object_list_index, object_index);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// input
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// input_windows.c
		API_FUNC_NAKED bool PLATFORM_API input_key_is_down(
			_enum key_code)
		{
			#if !PLATFORM_IS_DEDI
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(INPUT_KEY_IS_DOWN);

			API_FUNC_NAKED_START()
				push	ecx

				mov		cx, key_code
				call	FUNCTION

				pop		ecx
			API_FUNC_NAKED_END_NO_STACK_POP()
			#else
			API_FUNC_NAKED_START_()
				xor eax, eax
			API_FUNC_NAKED_END_()
			#endif
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// interface
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// hud.c
		API_FUNC_NAKED wcstring PLATFORM_API hud_get_item_string(
			int16 reference_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(HUD_GET_ITEM_MESSAGE);

			API_FUNC_NAKED_START()
				movsx edx, reference_index
				call FUNCTION
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		//////////////////////////////////////////////////////////////////////////
		// hud_chat.c
		void PLATFORM_API hud_chat_to_network(
			int32 player_number,
			long_enum chat_type,
			wcstring text)
		{
			Engine::Networking::EncodeHudChatNetworkData(player_number, chat_type, text);
		}

		API_FUNC_NAKED void PLATFORM_API hud_chat_display_message(
			wcstring message)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(KEYSTONE_CHAT_LOG_ADD_STRING);

			API_FUNC_NAKED_START()
				push message
				call FUNCTION
				add esp, 4 * 1
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		//////////////////////////////////////////////////////////////////////////
		// hud_draw.c
		API_FUNC_NAKED void PLATFORM_API hud_draw_meter(
			int16 /*local_player_index*/,
			TagGroups::s_hud_absolute_placement* placement,
			TagGroups::s_hud_element_meter* element,
			byte arg_C,
			byte arg_10,
			long_flags flags,
			real arg_18,
			real arg_1C)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(HUD_DRAW_METER);

			API_FUNC_NAKED_START()
				push esi

				push arg_1C
				push arg_18
				push flags
				movzx esi, arg_10
				push esi
				movzx esi, arg_C
				push esi
				mov esi, element
				push placement
				call FUNCTION
				add esp, 4 * 6

				pop esi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		API_FUNC_NAKED void PLATFORM_API hud_draw_numbers(
			int16 local_player_index,
			TagGroups::s_hud_absolute_placement* placement,
			TagGroups::s_hud_element_number* element,
			int32 number,
			int16 arg_10,
			long_flags flags,
			game_ticks_t first_render_time,
			real arg_1C)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(HUD_DRAW_NUMBERS);

			API_FUNC_NAKED_START()
				push esi

				push arg_1C
				push first_render_time
				push flags
				movsx esi, arg_10
				push esi
				push number
				push element
				push placement
				movsx esi, local_player_index
				push esi
				call FUNCTION
				add esp, 4 * 8

				pop esi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		API_FUNC_NAKED void PLATFORM_API hud_draw_static_element(
			int16 local_player_index,
			TagGroups::s_hud_absolute_placement* placement,
			TagGroups::s_hud_element_static* element,
			long_flags flags,
			game_ticks_t first_render_time)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(HUD_DRAW_STATIC_ELEMENT);

			API_FUNC_NAKED_START()
				push esi

				push first_render_time
				push flags
				push element
				push placement
				movsx esi, local_player_index
				push esi
				call FUNCTION
				add esp, 4 * 5

				pop esi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		//////////////////////////////////////////////////////////////////////////
		// hud_messaging.c
		API_FUNC_NAKED void PLATFORM_API hud_print_message(
			int16 local_player_index,
			wcstring message)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(HUD_PRINT_MESSAGE);

			API_FUNC_NAKED_START()
				push message
				movsx eax, local_player_index
				call FUNCTION
				add esp, 4 * 1
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		//////////////////////////////////////////////////////////////////////////
		// ui_video_screen.c
		void PLATFORM_API ui_video_screen_add_resolution(
			uint32 width,
			uint32 height,
			uint32 refresh_rate)
		{
			Engine::RasterizerAddResolution(width, height, refresh_rate);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// items
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// weapons.c
		API_FUNC_NAKED bool PLATFORM_API weapon_prevents_melee_attack(
			const datum_index weapon_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(WEAPON_PREVENTS_MELEE_ATTACK);

			API_FUNC_NAKED_START()
				push ecx

				mov ecx, weapon_index
				call FUNCTION

				pop ecx
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED bool PLATFORM_API weapon_prevents_grenade_throwing(
			const datum_index weapon_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(WEAPON_PREVENTS_GRENADE_THROWING);

			API_FUNC_NAKED_START()
				push ecx

				mov ecx, weapon_index
				call FUNCTION

				pop ecx
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API weapon_stop_reload(
			const datum_index weapon_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(WEAPON_STOP_RELOAD);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API first_person_weapon_message_from_unit(
			const datum_index unit_index,
			const int32 weapon_message_type)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(FIRST_PERSON_WEAPON_MESSAGE_FROM_UNIT);

			API_FUNC_NAKED_START()
				push eax
				push ecx
				push edx

				mov eax, unit_index
				push weapon_message_type
				call FUNCTION
				add esp, 4

				pop edx
				pop ecx
				pop eax
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED int16 PLATFORM_API weapon_get_first_person_animation_time(
			const datum_index weapon_index,
			const int16 frame_type,
			Enums::first_person_weapon_animation animation,
			const int32 arg3)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(WEAPON_GET_FIRST_PERSON_ANIMATION_TIME);

			API_FUNC_NAKED_START()
				push ecx

				xor ecx, ecx
				movsx cx, frame_type
				push arg3
				push ecx
				mov eax, weapon_index
				movsx cx, animation
				call FUNCTION
				add esp, 8

				pop ecx
				pop ebp
			API_FUNC_NAKED_END_()
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// main
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// console.c
		API_FUNC_NAKED bool PLATFORM_API console_process_command(
			long_flags access_flags,
			cstring command)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CONSOLE_PROCESS_COMMAND);

			API_FUNC_NAKED_START()
				push edi

				mov edi, command
				push access_flags
				call FUNCTION
				add esp, 4 * 1

				pop edi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		static API_FUNC_NAKED void PLATFORM_API console_printf_impl(
			bool clear_screen,
			cstring format)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CONSOLE_PRINTF);

			API_FUNC_NAKED_START()
				push format
				mov al, clear_screen
				call FUNCTION
				add esp, 4 * 1
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		void PLATFORM_API console_printf(
			bool clear_screen,
			cstring format,
			...)
		{
			char local[k_engine_function_string_buffer_size];
			memset(local, 0, k_engine_function_string_buffer_size);

			va_list args;
			va_start(args, format);
			vsprintf_s(local, format, args);
			va_end(args);

			console_printf_impl(clear_screen, local);
		}

		static API_FUNC_NAKED void PLATFORM_API console_response_printf_impl(
			BOOL clear_screen,
			cstring format)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CONSOLE_RESPONSE_PRINTF);

			API_FUNC_NAKED_START()
				push format
				push clear_screen
				call FUNCTION
				add esp, 4 * 2
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		void PLATFORM_API console_response_printf(
			bool clear_screen,
			cstring format,
			...)
		{
			char local[k_engine_function_string_buffer_size];
			memset(local, 0, k_engine_function_string_buffer_size);

			va_list args;
			va_start(args, format);
			vsprintf_s(local, format, args);
			va_end(args);

			console_response_printf_impl(clear_screen, local);
		}

		static API_FUNC_NAKED void PLATFORM_API console_warning_impl(
			cstring format)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(CONSOLE_WARNING);

			API_FUNC_NAKED_START()
				push format
				call FUNCTION
				add esp, 4 * 1
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		void PLATFORM_API console_warning(
			cstring format,
			...)
		{
			char local[k_engine_function_string_buffer_size];
			memset(local, 0, k_engine_function_string_buffer_size);

			va_list args;
			va_start(args, format);
			vsprintf_s(local, format, args);
			va_end(args);

			console_warning_impl(local);
		}

		bool PLATFORM_API console_process_remote_command(
			cstring command,
			int32 machine_index)
		{
			Console::TerminalGlobals()->rcon_machine_index = machine_index;
			bool result = console_process_command(0, command);
			Console::TerminalGlobals()->rcon_machine_index = NONE;

			return result;
		}

		//////////////////////////////////////////////////////////////////////////
		// main.c
		API_FUNC_NAKED bool PLATFORM_API main_connect(
			cstring address,
			cstring password)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(MAIN_CONNECT);

			API_FUNC_NAKED_START()
				push password
				push address
				call FUNCTION
				add esp, 4 * 2
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		void PLATFORM_API main_menu_load()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(MAINMENU_LOAD);

			GameState::MainGlobals()->map.main_menu_scenario_load = true;

			__asm call FUNCTION
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// math
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// periodic_functions.c
		API_FUNC_NAKED real PLATFORM_API periodic_function_evaluate(
			Enums::periodic_function function_type,
			real input)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(PERIODIC_FUNCTION_EVALUATE);

			API_FUNC_NAKED_START()
				fld input
				sub esp, 4 * 2 // allocate space for the 'input' parameter
				fstp qword ptr [esp] // store the input on the stack
				movzx eax, function_type
				call FUNCTION
				add esp, 4 * 2 // deallocate. double type consumes two DWORDs of stack
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		API_FUNC_NAKED real PLATFORM_API transition_function_evaluate(
			Enums::transition_function function_type,
			real input)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(TRANSITION_FUNCTION_EVALUATE);

			API_FUNC_NAKED_START()
				push input
				movzx ecx, function_type
				call FUNCTION
				add esp, 4 * 1
			API_FUNC_NAKED_END_NO_STACK_POP()
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// memory
	namespace blam
	{
		using namespace Memory;

		//////////////////////////////////////////////////////////////////////////
		// data.c
		API_FUNC_NAKED s_data_array* PLATFORM_API data_new(
			cstring name,
			int32 maximum_count,
			size_t datum_size)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DATA_NEW);

			API_FUNC_NAKED_START()
				push ebx

				mov ebx, datum_size
				push maximum_count
				push name
				call FUNCTION
				add esp, 4 * 2

				pop ebx
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		void PLATFORM_API data_dispose(
			s_data_array* data)
		{
			if (data != nullptr)
			{
				data_verify(data);
				CAST_PTR(Yelo::Memory::s_data_array*, GlobalFree(data));
			}
		}

		API_FUNC_NAKED void PLATFORM_API data_delete_all(
			s_data_array* data)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DATA_DELETE_ALL);

			API_FUNC_NAKED_START()
				push esi

				mov esi, data
				call FUNCTION

				pop esi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		datum_index PLATFORM_API data_next_index(
			s_data_array* data,
			datum_index cursor)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DATA_NEXT_INDEX);

			if (data == nullptr || cursor.IsNull())
				return datum_index::null;

			__asm {
				push esi
				push edi

				mov esi, cursor
				mov edi, data
				call FUNCTION

				pop edi
				pop esi
			}
		}

		/*		void* PLATFORM_API data_iterator_next(s_data_iterator& iterator)
				{
					static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DATA_ITERATOR_NEXT);
		
					API_FUNC_NAKED_START()
						push	edi
		
						mov		edi, iterator
						call	FUNCTION
		
						pop		edi
					API_FUNC_NAKED_END_NO_STACK_POP()
				}*/
		datum_index PLATFORM_API datum_new_at_index(
			s_data_array* data,
			datum_index index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DATUM_NEW_AT_INDEX);

			if (data == nullptr || index.IsNull())
				return datum_index::null;

			__asm {
				mov eax, index
				mov edx, data
				call FUNCTION
			}
		}

		datum_index PLATFORM_API datum_new(
			s_data_array* data)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DATUM_NEW);

			if (data == nullptr)
				return datum_index::null;

			__asm {
				mov edx, data
				call FUNCTION
			}
		}

		void PLATFORM_API datum_delete(
			s_data_array* data,
			datum_index index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DATUM_DELETE);

			if (data == nullptr || index.IsNull())
				return;

			__asm {
				mov edx, index
				mov eax, data
				call FUNCTION
			}
		}

		void* PLATFORM_API datum_try_and_get(
			s_data_array* data,
			datum_index index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(DATUM_TRY_AND_GET);

			if (data == nullptr)
				return nullptr;

			__asm {
				push edx
				push esi

				mov edx, index
				mov esi, data
				call FUNCTION

				pop esi
				pop edx
			}
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// models
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// models.c
		API_FUNC_NAKED int16 PLATFORM_API model_find_marker(
			const datum_index render_model_definition_index,
			cstring marker_name)
		{
			static const uintptr_t CALL_ADDRESS = Engine::GET_FUNC_PTR(MODEL_FIND_MARKER);

			API_FUNC_NAKED_START()
				mov eax, render_model_definition_index
				push render_model_definition_index
				push marker_name
				call CALL_ADDRESS
				add esp, 8

				pop ebp
			API_FUNC_NAKED_END_()
		}

		//////////////////////////////////////////////////////////////////////////
		// model_animations.c
		int16 PLATFORM_API animation_choose_random_permutation_internal(
			long_enum render_or_affects_game_state,
			datum_index animation_graph_index,
			int32 animation_index)
		{
			return Engine::AnimationPickRandomPermutation(render_or_affects_game_state, animation_graph_index, animation_index);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// objects
	namespace blam
	{
		using namespace Objects;

		//////////////////////////////////////////////////////////////////////////
		// damage.c
		bool PLATFORM_API object_restore_body(
			datum_index object_index)
		{
			return Engine::Objects::RestoreBody(object_index);
		}

		void PLATFORM_API object_deplete_body(
			datum_index object_index)
		{
			Engine::Objects::DepleteBody(object_index);
		}

		void PLATFORM_API object_deplete_shield(
			datum_index object_index)
		{
			Engine::Objects::DepleteShield(object_index);
		}

		void PLATFORM_API object_double_charge_shield(
			datum_index object_index)
		{
			Engine::Objects::DoubleChargeShield(object_index);
		}

		void PLATFORM_API area_of_effect_cause_damage(
			s_damage_data& data,
			datum_index)
		{
			// TODO
		}

		void PLATFORM_API object_cause_damage(
			s_damage_data& data,
			datum_index damaged_object_index,
			int16 node_index,
			int16 region_index,
			int16 damage_materials_element_index,
			real_vector3d* normal)
		{
			Engine::Objects::ObjectCauseDamage(
				data,
				damaged_object_index,
				node_index,
				region_index,
				damage_materials_element_index,
				normal);
		}

		//////////////////////////////////////////////////////////////////////////
		// index_resolution.c
		int32 PLATFORM_API index_resolution_table_translate(
			MessageDeltas::s_index_resolution_table& table,
			datum_index local_index)
		{
			// TODO: get rid of these guards
			#ifndef YELO_NO_NETWORK
			return Engine::Networking::TranslateIndex(table, local_index);
			#else
			return 0;
			#endif
		}

		//////////////////////////////////////////////////////////////////////////
		// objects.c
		void PLATFORM_API object_reset(
			datum_index object_index)
		{
			Engine::Objects::Reset(object_index);
		}

		void PLATFORM_API object_set_position(
			datum_index object_index,
			__in_opt real_point3d* new_position,
			__in_opt real_vector3d* new_forward,
			__in_opt real_vector3d* new_up)
		{
			Engine::Objects::SetPosition(object_index, new_position, new_forward, new_up);
		}

		void PLATFORM_API object_set_position_network(
			datum_index object_index,
			real_point3d* new_position)
		{
			Engine::Objects::SetPositionNetwork(object_index, new_position);
		}

		void PLATFORM_API object_translate(
			datum_index object_index,
			const real_point3d& new_position,
			__in_opt const s_scenario_location* new_location)
		{
			// TODO
		}

		void PLATFORM_API object_placement_data_new(
			s_object_placement_data& data,
			datum_index object_definition_index,
			datum_index owner_object_index)
		{
			Engine::Objects::PlacementDataNew(data, object_definition_index, owner_object_index);
		}

		datum_index PLATFORM_API object_new(
			s_object_placement_data& data)
		{
			return Engine::Objects::New(data);
		}

		datum_index PLATFORM_API object_new_with_role(
			s_object_placement_data& data,
			Enums::networked_datum role)
		{
			return Engine::Objects::NewWithRole(data, role);
		}

		void PLATFORM_API object_delete_to_network(
			datum_index object_index)
		{
			Engine::Networking::EncodeObjectDeletionMessage(object_index);
		}

		void PLATFORM_API object_delete(
			datum_index object_index)
		{
			Engine::Objects::Delete(object_index);
		}

		void PLATFORM_API object_reconnect_to_map(
			datum_index object_index,
			__in_opt s_scenario_location* location_reference)
		{
			Engine::Objects::ReconnectToMap(object_index, location_reference);
		}

		void PLATFORM_API object_disconnect_from_map(
			datum_index object_index)
		{
			Engine::Objects::DisconnectFromMap(object_index);
		}

		API_FUNC_NAKED int16 PLATFORM_API object_get_marker_by_name(
			const datum_index object_index,
			cstring marker_name,
			s_object_marker* markers,
			const int16 maximum_marker_count)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(OBJECT_GET_MARKER_BY_NAME);

			_asm jmp FUNCTION;
		}

		void PLATFORM_API object_attach_to_marker(
			datum_index target_object_index,
			cstring target_marker_name,
			datum_index object_index,
			cstring marker_name)
		{
			Engine::Objects::Attach(target_object_index, target_marker_name, object_index, marker_name);
		}

		void PLATFORM_API object_detach(
			datum_index object_index)
		{
			Engine::Objects::Detach(object_index);
		}

		real_point3d& PLATFORM_API object_get_origin(
			datum_index object_index,
			__out real_point3d& return_origin)
		{
			return Engine::Objects::GetOrigin(object_index, return_origin);
		}

		void PLATFORM_API object_get_orientation(
			datum_index object_index,
			__out_opt real_vector3d* return_forward,
			__out_opt real_vector3d* return_up)
		{
			Engine::Objects::GetOrientation(object_index, return_forward, return_up);
		}

		s_scenario_location& PLATFORM_API object_get_location(
			datum_index object_index,
			__out s_scenario_location& return_location)
		{
			return Engine::Objects::GetLocation(object_index, return_location);
		}

		void PLATFORM_API object_start_interpolation(
			datum_index object_index,
			int32 interpolation_frame_count)
		{
			Engine::Objects::StartInterpolation(object_index, interpolation_frame_count);
		}

		s_object_data* PLATFORM_API object_iterator_next(
			s_object_iterator& iter)
		{
			return Engine::Objects::IteratorNext(iter);
		}

		int16 PLATFORM_API objects_in_sphere(
			Flags::objects_find_flags find_flags,
			long_flags object_type_flags,
			const s_scenario_location& location,
			const real_point3d& center,
			real radius,
			datum_index object_indices[],
			int16 maximum_object_indices)
		{
			return Engine::Objects::FindInSphere(
				find_flags,
				object_type_flags,
				location,
				center,
				radius,
				object_indices,
				maximum_object_indices);
		}

		void PLATFORM_API object_definition_predict(
			datum_index object_index)
		{
			Engine::Objects::DefinitionPredict(object_index);
		}

		bool PLATFORM_API object_header_block_allocate(
			datum_index object_index,
			size_t block_reference_offset,
			size_t size)
		{
			return Engine::Objects::HeaderBlockAllocate(object_index, block_reference_offset, size);
		}

		void PLATFORM_API objects_scripting_set_scale(
			datum_index object_index,
			real scale,
			int32 ticks)
		{
			Engine::Objects::SetScale(object_index, scale, ticks);
		}

		real PLATFORM_API object_get_level_of_detail_pixels(
			datum_index object_index)
		{
			return Engine::Objects::GetLevelOfDetailPixels(object_index);
		}

		void PLATFORM_API object_render_state_refresh(
			datum_index object_render_state_index,
			datum_index object_index,
			real level_of_detail_pixels,
			byte arg4)
		{
			Engine::Objects::ObjectRenderStateRefresh(object_render_state_index, object_index, level_of_detail_pixels, arg4);
		}

		API_FUNC_NAKED void PLATFORM_API objects_update()
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(OBJECTS_UPDATE);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API object_update(
			datum_index object_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(OBJECT_UPDATE);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API object_destroy(
			const datum_index object_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(OBJECT_DESTROY);

			API_FUNC_NAKED_START()
				push eax
				push ecx
				push edx
				push edi

				mov eax, object_index
				call FUNCTION

				pop edi
				pop edx
				pop ecx
				pop eax
				pop ebp
			API_FUNC_NAKED_END_()
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// physics
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// collisions.c
		API_FUNC_NAKED bool PLATFORM_API collision_test_vector(
			long_flags flags,
			real_point3d& location,
			real_vector3d& vector,
			datum_index object_index,
			Physics::s_collision_result& collision)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(COLLISION_TEST_VECTOR);

			__asm jmp FUNCTION;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// rasterizer
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// dx9/rasterizer_dx9.c
		bool PLATFORM_API rasterizer_set_texture_bitmap_data(
			_enum stage,
			TagGroups::s_bitmap_data* bitmap)
		{
			return Engine::SetTextureSamplerStage(bitmap, stage);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// render
	namespace blam
	{
		void PLATFORM_API render_window(
			const uint16 local_player_index,
			void* render_camera,
			void* render_frustum,
			void* rasterizer_camera,
			void* rasterizer_frustum,
			void* rasterizer_target,
			const bool is_mirror)
		{
			CAST_PTR(void (PLATFORM_API*)(const uint16, void*, void*, void*, void*, void*, const bool), Engine::GET_FUNC_PTR(RENDER_WINDOW))(local_player_index, render_camera, render_frustum, rasterizer_camera, rasterizer_frustum, rasterizer_target, is_mirror);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// scenario
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// scenario.c
		// After calling this function you should call Yelo::GameState::InitializeForNewBSP
		API_FUNC_NAKED bool PLATFORM_API scenario_switch_structure_bsp(
			int16 bsp_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(SCENARIO_SWITCH_STRUCTURE_BSP);

			API_FUNC_NAKED_START()
				push esi

				movsx esi, bsp_index
				call FUNCTION

				pop esi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}

		bool PLATFORM_API scenario_trigger_volume_test_point(
			int32 trigger_volume_index,
			const real_point3d& point)
		{
			return Engine::Scenario::TriggerVolumeTestPoint(trigger_volume_index, point);
		}

		bool PLATFORM_API scenario_trigger_volume_test_object(
			int32 trigger_volume_index,
			datum_index object_index)
		{
			return Engine::Scenario::TriggerVolumeTestObject(trigger_volume_index, object_index);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// shell
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// shell_windows.c
		API_FUNC_NAKED bool PLATFORM_API shell_get_command_line_argument(
			cstring param,
			_Out_opt_			cstring* value)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(SHELL_GET_COMMAND_LINE_ARGUMENT);

			API_FUNC_NAKED_START()
				push edi

				mov edi, value
				push param
				call FUNCTION
				add esp, 4 * 1

				pop edi
			API_FUNC_NAKED_END_NO_STACK_POP()
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// tag_files
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// tag_groups.c
		datum_index PLATFORM_API tag_loaded(
			tag group_tag,
			cstring name)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(TAG_LOADED);

			#if defined(ENGINE_FUNCTIONS_USE_LOCAL)
			char local[256];
			memset(local, 0, sizeof(local));
			strcpy_s(local, name);
			#else
			cstring local = name;
			#endif

			__asm {
				push edi

				#if defined(ENGINE_FUNCTIONS_USE_LOCAL)
				lea eax, local
				#else
				mov		eax, local
				#endif
				push eax
				mov edi, group_tag
				call FUNCTION
				add esp, 4 * 1

				pop edi
			}
		}

		void PLATFORM_API tag_iterator_new(
			TagGroups::s_tag_iterator& iter,
			const tag group_tag_filter)
		{
			memset(&iter, 0, sizeof(iter));
			iter.group_tag_filter = group_tag_filter;
		}

		datum_index PLATFORM_API tag_iterator_next(
			TagGroups::s_tag_iterator& iter)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(TAG_ITERATOR_NEXT);

			__asm {
				push esi

				mov esi, iter
				call FUNCTION

				pop esi
			}
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// text
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// text_group.c
		wcstring unicode_string_list_get_string(
			datum_index unicode_string_list_definition_index,
			int32 reference_index)
		{
			return Engine::TagGroups::UnicodeStringListGetString(unicode_string_list_definition_index, reference_index);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// units
	namespace blam
	{
		//////////////////////////////////////////////////////////////////////////
		// units.c
		API_FUNC_NAKED void PLATFORM_API unit_update(
			const datum_index unit_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_UPDATE);

			_asm jmp FUNCTION;
		}

		void PLATFORM_API unit_set_animation(
			datum_index unit_index,
			datum_index animation_graph_index,
			int32 animation_index)
		{
			Engine::Objects::UnitSetAnimation(unit_index, animation_graph_index, animation_index);
		}

		API_FUNC_NAKED void PLATFORM_API unit_animation_start_action(
			const datum_index unit_index,
			const Enums::unit_replacement_animation_state action_type)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_ANIMATION_START_ACTION);

			_asm jmp FUNCTION;
		}

		int16 PLATFORM_API unit_find_nearby_seat(
			datum_index unit_index,
			datum_index target_unit_index,
			__out int16& parent_seat_index)
		{
			return Engine::Objects::UnitFindNearbySeat(unit_index, target_unit_index, parent_seat_index);
		}

		bool PLATFORM_API unit_can_enter_seat(
			datum_index unit_index,
			datum_index target_unit_index,
			int16 seat_index,
			__out datum_index* unit_in_seat_index)
		{
			return Engine::Objects::UnitCanEnterSeat(unit_index, target_unit_index, seat_index, unit_in_seat_index);
		}

		bool PLATFORM_API unit_enter_seat(
			datum_index unit_index,
			datum_index target_unit_index,
			int32 seat_index)
		{
			return Engine::Objects::UnitEnterSeat(unit_index, target_unit_index, seat_index);
		}

		void PLATFORM_API unit_get_camera_position(
			datum_index unit_index,
			__out real_point3d* return_position)
		{
			Engine::Objects::UnitGetCameraPosition(unit_index, return_position);
		}

		void PLATFORM_API unit_kill(
			const datum_index unit_index)
		{
			auto* unit_datum = blam::object_try_and_get_and_verify_type<s_unit_datum>(unit_index);
			if (unit_datum)
			{
				SET_FLAG(unit_datum->object.damage.flags, Flags::_object_killed_bit, true);
			}
		}

		void PLATFORM_API unit_open(
			datum_index unit_index)
		{
			Engine::Objects::UnitOpen(unit_index);
		}

		void PLATFORM_API unit_close(
			datum_index unit_index)
		{
			Engine::Objects::UnitClose(unit_index);
		}

		void PLATFORM_API unit_scripting_exit_vehicle(
			datum_index unit_index)
		{
			Engine::Objects::UnitExitVehicle(unit_index);
		}

		API_FUNC_NAKED bool PLATFORM_API unit_try_and_exit_seat(
			const datum_index unit_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_TRY_AND_EXIT_SEAT);

			API_FUNC_NAKED_START()
				push ecx
				push edx
				push edi

				mov edi, unit_index
				call FUNCTION

				pop edi
				pop edx
				pop ecx
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API unit_detach_from_parent(
			const datum_index unit_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_DETACH_FROM_PARENT);

			API_FUNC_NAKED_START()
				push edi

				mov edi, unit_index
				call FUNCTION

				pop edi
				pop ebp
			API_FUNC_NAKED_END_()
		}

		void PLATFORM_API unit_exit_seat_end(
			datum_index unit_index,
			bool no_network_message,
			bool can_run_on_client_side,
			bool unknown)
		{
			Engine::Objects::UnitExitSeatEnd(unit_index, no_network_message, can_run_on_client_side, unknown);
		}

		API_FUNC_NAKED bool PLATFORM_API unit_can_see_point(
			const datum_index unit_index,
			const real_point3d* point,
			const real view_radians)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_CAN_SEE_POINT);

			API_FUNC_NAKED_START()
				push ecx
				push edx
				push edi

				mov ecx, unit_index
				mov edi, point
				push view_radians
				call FUNCTION
				add esp, 4

				pop edi
				pop edx
				pop ecx
				pop ebp
			API_FUNC_NAKED_END_()
		}

		int16 unit_get_custom_animation_time(
			datum_index unit_index)
		{
			return Engine::Objects::UnitGetCustomAnimationTime(unit_index);
		}

		API_FUNC_NAKED bool PLATFORM_API unit_start_user_animation(
			const datum_index unit_index,
			const datum_index animation_definition_index,
			cstring animation_name,
			const bool interpolate)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_START_USER_ANIMATION);

			API_FUNC_NAKED_START()
				push ebx
				push ecx
				push edx
				push edi

				mov eax, animation_name
				mov edi, animation_definition_index
				xor ebx, ebx
				mov bl, interpolate
				push ebx
				push unit_index
				call FUNCTION
				add esp, 8

				pop edi
				pop edx
				pop ecx
				pop ebx
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API unit_animation_set_state(
			const datum_index unit_index,
			const Enums::unit_animation_state state)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_ANIMATION_SET_STATE);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API unit_drop_current_weapon(
			const datum_index unit_index,
			const bool force)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_DROP_CURRENT_WEAPON);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED datum_index PLATFORM_API unit_inventory_get_weapon(
			const datum_index unit_index,
			const int16 index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_INVENTORY_GET_WEAPON);

			API_FUNC_NAKED_START()
				push ecx
				push edx

				mov eax, unit_index
				xor ecx, ecx
				mov cx, index
				call FUNCTION

				pop edx
				pop ecx
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API unit_ready_desired_weapon(
			const datum_index unit_index,
			const bool force)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_READY_DESIRED_WEAPON);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API unit_throw_grenade_release(
			const datum_index unit_index,
			const sbyte keyframe)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_THROW_GRENADE_RELEASE);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API unit_cause_player_melee_damage(
			const datum_index unit_index)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_CAUSE_PLAYER_MELEE_DAMAGE);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API unit_damage_aftermath(
			const datum_index unit_index,
			const s_damage_data* damage_data,
			const _enum damage_flags,
			const real shield_amount,
			const real body_amount,
			void* arg6,
			const int32 damage_part)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_DAMAGE_AFTERMATH);

			_asm jmp FUNCTION;
		}

		API_FUNC_NAKED void PLATFORM_API unit_scripting_set_current_vitality(
			const datum_index unit_index,
			const real health,
			const real shield)
		{
			static const uintptr_t CALL_ADDRESS = Engine::GET_FUNC_PTR(UNIT_SCRIPTING_SET_CURRENT_VITALITY);

			API_FUNC_NAKED_START()
				push eax

				mov eax, unit_index
				push shield
				push health
				call CALL_ADDRESS
				add esp, 8

				pop eax
				pop ebp
			API_FUNC_NAKED_END_()
		}

		API_FUNC_NAKED void PLATFORM_API unit_set_actively_controlled(
			const datum_index unit_index,
			const bool controlled)
		{
			static const uintptr_t FUNCTION = Engine::GET_FUNC_PTR(UNIT_SET_ACTIVELY_CONTROLLED);

			API_FUNC_NAKED_START()
				push ecx

				xor ecx, ecx
				mov cl, controlled
				push unit_index
				call FUNCTION
				add esp, 4

				pop ecx
				pop ebp
			API_FUNC_NAKED_END_()
		}
	};
};
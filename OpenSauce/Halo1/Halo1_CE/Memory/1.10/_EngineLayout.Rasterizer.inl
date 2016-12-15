/*
	Yelo: Open Sauce SDK
		Halo 1 (CE) Edition

	See license\OpenSauce\Halo1_CE for specific license information
*/

#if !PLATFORM_IS_DEDI
#include "Memory/1.10/Pointers/HaloCE_110_Runtime.Rasterizer.inl"
#include "Memory/1.10/Pointers/HaloCE_110_Runtime_Manual.Rasterizer.inl"
#endif

//////////////////////////////////////////////////////////////////////////
// Rasterizer.cpp
#if __EL_INCLUDE_FILE_ID == __EL_RASTERIZER_RASTERIZER
	namespace Rasterizer
	{
		static real_matrix3x4** K_VSH_CONSTANTS__NODEMATRICES_REFERENCES[] = {
			CAST_PTR(real_matrix3x4**,K_VSH_CONSTANTS__NODEMATRICES_REFERENCE_0),
			CAST_PTR(real_matrix3x4**,K_VSH_CONSTANTS__NODEMATRICES_REFERENCE_1),
			CAST_PTR(real_matrix3x4**,K_VSH_CONSTANTS__NODEMATRICES_REFERENCE_2),
			CAST_PTR(real_matrix3x4**,K_VSH_CONSTANTS__NODEMATRICES_REFERENCE_3),
			CAST_PTR(real_matrix3x4**,K_VSH_CONSTANTS__NODEMATRICES_REFERENCE_4),
			CAST_PTR(real_matrix3x4**,K_VSH_CONSTANTS__NODEMATRICES_REFERENCE_5),
		};
		static void** K_VSH_CONSTANTS__NODEMATRICES_REFERENCES_PLUS_8[] = {
			CAST_PTR(void**,K_VSH_CONSTANTS__NODEMATRICES_REFERENCES_PLUS_8_0),
		};

		ENGINE_PTR(s_rasterizer_config, rasterizer_config,					K_RASTERIZER_CONFIG, PTR_NULL);
		ENGINE_PTR(s_rasterizer_globals, rasterizer_globals,				K_RASTERIZER_GLOBALS, PTR_NULL);
		ENGINE_PTR(s_rasterizer_debug_options, rasterizer_debug_data,		K_RASTERIZER_DEBUG_DATA, PTR_NULL);
		ENGINE_PTR(s_rasterizer_frame_parameters, rasterizer_frame_params,	K_RASTERIZER_FRAME_PARAMS, PTR_NULL);

		FUNC_PTR(RASTERIZER_DISPOSE,									K_RASTERIZER_DISPOSE, FUNC_PTR_NULL);
		FUNC_PTR(RASTERIZER_DISPOSE_CALL_FROM_RASTERIZER,				K_RASTERIZER_DISPOSE_CALL_FROM_RASTERIZER, FUNC_PTR_NULL);
		FUNC_PTR(RASTERIZER_DISPOSE_CALL_FROM_SHELL,					K_RASTERIZER_DISPOSE_CALL_FROM_SHELL, FUNC_PTR_NULL);
		

		static void** K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_LIST[] = {
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_0),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_1),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_2),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_3),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_4),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_5),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_6),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_7),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_8),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_9),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_10),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_11),
			CAST_PTR(void**,K_RENDERED_TRIANGLE_ARRAY_UPGRADE_ADDRESS_12),
		};
		static uint16* K_MAXIMUM_RENDERED_TRIANGLES_UPGRADE_ADDRESS_LIST[] = {
			CAST_PTR(uint16*,K_MAXIMUM_RENDERED_TRIANGLES_UPGRADE_ADDRESS_0),
			CAST_PTR(uint16*,K_MAXIMUM_RENDERED_TRIANGLES_UPGRADE_ADDRESS_1),
			CAST_PTR(uint16*,K_MAXIMUM_RENDERED_TRIANGLES_UPGRADE_ADDRESS_2),
			CAST_PTR(uint16*,K_MAXIMUM_RENDERED_TRIANGLES_UPGRADE_ADDRESS_3),
		};

		ENGINE_PTR(DWORD, RASTERIZER_DYNAMIC_GEOMETRY_INITIALIZE__CreateIndexBuffer_Length_ARG, K_RASTERIZER_DYNAMIC_GEOMETRY_INITIALIZE__CREATEINDEXBUFFER_LENGTH_ARG, PTR_NULL);

		ENGINE_PTR(DWORD, RASTERIZER_DEVICE_ADAPTER_INDEX,				K_RASTERIZER_DEVICE_ADAPTER_INDEX, PTR_NULL);

		static char** K_SCREENSHOT_FOLDER_REFERENCES[] = {
			CAST_PTR(char**,K_SCREENSHOT_FOLDER_REFERENCE_0),
			CAST_PTR(char**,K_SCREENSHOT_FOLDER_REFERENCE_1),
		};
	};
	namespace Render
	{
		ENGINE_PTR(s_render_window, render_window,						K_RENDER_WINDOW, PTR_NULL);
		ENGINE_PTR(s_render_globals, render_globals,					K_RENDER_GLOBALS, PTR_NULL);
		ENGINE_PTR(s_render_target, global_render_targets,				K_GLOBAL_RENDER_TARGETS, PTR_NULL);
		ENGINE_PTR(s_structure_render_globals, structure_render_globals,K_STRUCTURE_RENDER_GLOBALS, PTR_NULL);

		FUNC_PTR(RENDER_WINDOW_REFLECTION_CALL,							K_RENDER_WINDOW_REFLECTION_CALL, FUNC_PTR_NULL);
		FUNC_PTR(RENDER_WINDOW_REFLECTION_CALL_RETN,					K_RENDER_WINDOW_REFLECTION_CALL_RETN, FUNC_PTR_NULL);

		FUNC_PTR(RENDER_WINDOW_END_HOOK,								K_RENDER_WINDOW_END_HOOK, FUNC_PTR_NULL);
	};

#elif __EL_INCLUDE_FILE_ID == __EL_RASTERIZER_LIGHTMAPS
	namespace Render { namespace Lightmaps
	{
		FUNC_PTR(BSP_LIGHTMAP_INDEX_HOOK,							K_BSP_LIGHTMAP_INDEX_HOOK, FUNC_PTR_NULL);
		FUNC_PTR(BSP_LIGHTMAP_INDEX_RETN,							K_BSP_LIGHTMAP_INDEX_RETN, FUNC_PTR_NULL);
		FUNC_PTR(SET_LIGHTMAP_SAMPLER_CALL,							K_SET_LIGHTMAP_SAMPLER_CALL, FUNC_PTR_NULL);
		FUNC_PTR(OBJECT_RENDER_CACHE_LIGHTMAP_TAG_INDEX_HOOK,		K_OBJECT_RENDER_CACHE_LIGHTMAP_TAG_INDEX_HOOK, FUNC_PTR_NULL);
		FUNC_PTR(OBJECT_RENDER_CACHE_LIGHTMAP_TAG_INDEX_RETN,		K_OBJECT_RENDER_CACHE_LIGHTMAP_TAG_INDEX_RETN, FUNC_PTR_NULL);
	};};

#elif __EL_INCLUDE_FILE_ID == __EL_RASTERIZER_SKY
	namespace Render { namespace Sky
	{
		FUNC_PTR(FOG_ATMOSPHERIC_SKY_TAG_INDEX_HOOK, K_FOG_ATMOSPHERIC_SKY_TAG_INDEX_HOOK, FUNC_PTR_NULL);
		FUNC_PTR(FOG_ATMOSPHERIC_SKY_TAG_INDEX_RETN, K_FOG_ATMOSPHERIC_SKY_TAG_INDEX_RETN, FUNC_PTR_NULL);
		FUNC_PTR(FOG_PLANAR_SKY_TAG_INDEX_HOOK, K_FOG_PLANAR_SKY_TAG_INDEX_HOOK, FUNC_PTR_NULL);
		FUNC_PTR(FOG_PLANAR_SKY_TAG_INDEX_RETN, K_FOG_PLANAR_SKY_TAG_INDEX_RETN, FUNC_PTR_NULL);
		FUNC_PTR(RENDER_SKY_SKY_TAG_INDEX_HOOK, K_RENDER_SKY_SKY_TAG_INDEX_HOOK, FUNC_PTR_NULL);
		FUNC_PTR(RENDER_SKY_SKY_TAG_INDEX_RETN, K_RENDER_SKY_SKY_TAG_INDEX_RETN, FUNC_PTR_NULL);
		FUNC_PTR(VISIBLE_SKY_ENABLE_SKY_TAG_INDEX_HOOK, K_VISIBLE_SKY_ENABLE_SKY_TAG_INDEX_HOOK, FUNC_PTR_NULL);
		FUNC_PTR(VISIBLE_SKY_ENABLE_SKY_TAG_INDEX_RETN, K_VISIBLE_SKY_ENABLE_SKY_TAG_INDEX_RETN, FUNC_PTR_NULL);
	};};

#else
	#error Undefined engine layout include for: __EL_INCLUDE_RASTERIZER
#endif
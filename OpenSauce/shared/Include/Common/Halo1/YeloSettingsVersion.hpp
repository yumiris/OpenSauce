/*
	Yelo: Open Sauce SDK

	See license\OpenSauce\OpenSauce for specific license information
*/
#pragma once

// Official branch build identifier this codebase is based on
#define K_OPENSAUCE_VERSION_BUILD_MAJ	3
#define K_OPENSAUCE_VERSION_BUILD_MIN	0
#define K_OPENSAUCE_VERSION_BUILD		0
#define K_OPENSAUCE_VERSION				\
	BOOST_PP_CAT(K_OPENSAUCE_VERSION_BUILD_MAJ, BOOST_PP_CAT(., K_OPENSAUCE_VERSION_BUILD_MIN))
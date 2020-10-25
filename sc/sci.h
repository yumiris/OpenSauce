///////////////////////////////////////////////////////////////////////////////
// File:	sci.h
// SDK:		GameSpy ATLAS Competition SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __SCI_H__
#define __SCI_H__

/* Internal interface for S&C SDK -- Developers should use sc.h */


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "sc.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Data types

typedef struct SCQueryParameter
{
	gsi_char *mName;
	gsi_char *mValue;
} SCQueryParameter;

struct SCQueryParameterList
{
	gsi_u32 mCount;
	gsi_u32 mNextSlot;
	SCQueryParameter *mQueryParams;
};

#define GS_SC_LOG(t, l, m, ... ) gsDebugFormat(GSIDebugCat_Atlas, t, l, "{%s:%d} In %s: " m "\n", __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __SCI_H__

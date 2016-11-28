///////////////////////////////////////////////////////////////////////////////
// File:	sciMain.c
// SDK:		GameSpy ATLAS Competition SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#include "sci.h"
#include "sciInterface.h"
#include "sciReport.h"

#include "../md5.h"
#include "../common/gsRC4.h"

/* PUBLIC INTERFACE FUNCTIONS - SCINTERFACE.C CONTAINS PRIVATE INTERFACE */

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scInitialize(int gameId, SCInterfacePtr* theInterfaceOut)
{
	SCInterface* anInterface  = NULL;
	SCResult     anInitResult = SCResult_NO_ERROR;

	// Check parameters
	if (theInterfaceOut == NULL)
	{
		return SCResult_INVALID_PARAMETERS;
	}

	// Clear out parameter
	*theInterfaceOut = NULL;

	// Obtain the internal interface
	anInitResult = sciInterfaceCreate(&anInterface);
	if (anInitResult != SCResult_NO_ERROR)
	{
		return anInitResult;
	}

	// Init the internal interface
	anInitResult = sciInterfaceInit(anInterface);
	if (anInitResult != SCResult_NO_ERROR)
	{
		return anInitResult;
	}

	anInterface->mGameId = (gsi_u32)gameId;
	//anInterface->mOptionsFlags = (gsi_u32)theOptionsFlags;

	// Set the out parameter and return
	*theInterfaceOut = anInterface;
	return SCResult_NO_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scShutdown(SCInterfacePtr theInterface)
{
	SCInterface* anInterface = (SCInterface*)theInterface;

	// Check parameters
	if (anInterface == NULL)
	{
		return SCResult_INVALID_PARAMETERS;
	}

	// Destroy interface if necessary
	if (anInterface->mInit)
	{
		sciInterfaceDestroy(anInterface);
		gsifree(anInterface);
	}

	return SCResult_NO_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scThink(SCInterfacePtr theInterface)
{
	SCInterface* anInterface = (SCInterface*)theInterface;

	// Check parameters
	if (anInterface == NULL)
	{
		return SCResult_INVALID_PARAMETERS;
	}
	if (!anInterface->mInit)
	{
		return SCResult_NOT_INITIALIZED;
	}

	sciWsThink(&anInterface->mWebServices);

	return SCResult_NO_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const char * scGetSessionId(const SCInterfacePtr theInterface)
{
	SCInterface * anInterface = (SCInterface*)theInterface;
	GS_ASSERT(theInterface != NULL);
	GS_ASSERT(gsi_is_true(anInterface->mInit));
	return (char *)anInterface->mSessionId;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const char * scGetConnectionId(const SCInterfacePtr theInterface)
{
	SCInterface * anInterface = (SCInterface*)theInterface;
	GS_ASSERT(theInterface != NULL);
	GS_ASSERT(gsi_is_true(anInterface->mInit));
	return (char *)anInterface->mConnectionId;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scSetSessionId(const SCInterfacePtr theInterface, const gsi_u8 theSessionId[SC_SESSION_GUID_SIZE])
{
	SCInterface * anInterface = (SCInterface*)theInterface;
	GS_ASSERT(theInterface != NULL);
	GS_ASSERT(gsi_is_true(anInterface->mInit));
	
	sciInterfaceSetSessionId(anInterface, (char *)theSessionId);
	return SCResult_NO_ERROR;
}

SCResult SC_CALL scCheckBanList(SCInterfacePtr             theInterface,
							    const GSLoginCertificate * certificate,
							    const GSLoginPrivateData * privateData,
							    gsi_u32                    hostProfileId,
								SCPlatform   			   hostPlatform,
								SCCheckBanListCallback     callback,
							    gsi_time                   timeoutMs,
							    void *                     userData)
{
	SCInterface * anInterface = (SCInterface*)theInterface;
	GS_ASSERT(theInterface != NULL);
	GS_ASSERT(certificate != NULL || (certificate==NULL && privateData==NULL));
	
	if (!wsLoginCertIsValid(certificate))
		return SCResult_INVALID_PARAMETERS;

	return sciWsCheckBanList(hostProfileId, hostPlatform, &anInterface->mWebServices, anInterface->mGameId, certificate, privateData, callback, timeoutMs, userData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// The certificate and private data may be NULL if the local client
// is an unauthenticated dedicated server
SCResult SC_CALL scCreateSession(SCInterfacePtr             theInterface,
							     const GSLoginCertificate * certificate,
							     const GSLoginPrivateData * privateData,
							     SCCreateSessionCallback    callback,
							     gsi_time                   timeoutMs,
								 void *                     userData)
{
	SCInterface * anInterface = (SCInterface*)theInterface;
	gsi_u16 platformId = SCPlatform_Unknown;
	GS_ASSERT(theInterface != NULL);
	GS_ASSERT(certificate != NULL || (certificate==NULL && privateData==NULL));
	
	if (!wsLoginCertIsValid(certificate))
		return SCResult_INVALID_PARAMETERS;

	platformId = sciGetPlatformId();

	return sciWsCreateSession(&anInterface->mWebServices, anInterface->mGameId, platformId, certificate, privateData, callback, timeoutMs, userData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scCreateMatchlessSession(SCInterfacePtr    theInterface,
								 const GSLoginCertificate * certificate,
								 const GSLoginPrivateData * privateData,
								 SCCreateSessionCallback    callback,
								 gsi_time                   timeoutMs,
								 void *                     userData)
{
	SCInterface * anInterface = (SCInterface*)theInterface;
	gsi_u16 platformId = SCPlatform_Unknown;
	GS_ASSERT(theInterface != NULL);
	GS_ASSERT(certificate != NULL || (certificate==NULL && privateData==NULL));

	if (!wsLoginCertIsValid(certificate))
		return SCResult_INVALID_PARAMETERS;

	platformId = sciGetPlatformId();

	return sciWsCreateMatchlessSession(&anInterface->mWebServices, anInterface->mGameId, platformId, certificate, privateData, callback, timeoutMs, userData);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scSetReportIntention(const SCInterfacePtr         theInterface,
									  const gsi_u8                 theConnectionId[SC_CONNECTION_GUID_SIZE],
									  gsi_bool                     isAuthoritative,
									  const GSLoginCertificate *   certificate,
									  const GSLoginPrivateData *   privateData,
									  SCSetReportIntentionCallback callback,
									  gsi_time                     timeoutMs,
									  void *                 userData)
{
	SCInterface* anInterface = (SCInterface*)theInterface;

	// Check parameters
	if (anInterface == NULL)
	{
		return SCResult_INVALID_PARAMETERS;
	}
	if (!certificate)
	{
		return SCResult_INVALID_PARAMETERS;
	}
	if (!privateData)
	{
		return SCResult_INVALID_PARAMETERS;
	}
	if (!anInterface->mInit)
	{
		return SCResult_NOT_INITIALIZED;
	}

	sciInterfaceSetConnectionId(anInterface, (const char *)theConnectionId);
	// Call web service
	return sciWsSetReportIntention(&anInterface->mWebServices, anInterface->mGameId,
		(char *)anInterface->mSessionId, (char *)anInterface->mConnectionId, isAuthoritative,
		certificate, privateData, callback, timeoutMs, userData);
}

									 
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scSubmitReport(const SCInterfacePtr  theInterface,
								const SCReportPtr      theReport,
								gsi_bool              isAuthoritative,
								const GSLoginCertificate * certificate,
								const GSLoginPrivateData * privateData,
								SCSubmitReportCallback callback,
								gsi_time               timeoutMs,
								void *           userData)
{
	SCInterface* anInterface = (SCInterface*)theInterface;
	SCIReport *aReport = (SCIReport *)theReport;
	SCResult result = SCResult_NO_ERROR;

	// Check parameters
	if (anInterface == NULL)
	{
		return SCResult_INVALID_PARAMETERS;
	}
	if (!anInterface->mInit)
	{
		return SCResult_NOT_INITIALIZED;
	}

	// Prepare the report hash
	{
		SCIReportHeader * header = (SCIReportHeader*)aReport->mBuffer.mData;
		GSMD5_CTX md5;
		// Clear out the checksum portion of the header so that the 
		// MD5 hash is calculated on the entire report without a checksum
		memset(header->mChecksum, 0, sizeof(header->mChecksum));
		
		GSMD5Init(&md5);
		GSMD5Update(&md5, (unsigned char *)aReport->mBuffer.mData, aReport->mBuffer.mPos);
		GSMD5Final(header->mChecksum, &md5);	
	}
	
	// Call web service
	result = sciWsSubmitReport(&anInterface->mWebServices, anInterface->mGameId,
		(char *)anInterface->mSessionId, (char *)anInterface->mConnectionId, aReport, isAuthoritative,
		certificate, privateData, callback, timeoutMs, userData);
	if (result == SCResult_NO_ERROR)
	{
		sciInterfaceSetConnectionId(theInterface, "");
	}
	return result;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scCreateReport(const SCInterfacePtr theInterface, 
								gsi_u32 theHeaderVersion,
								gsi_u32 thePlayerCount,
								gsi_u32 theTeamCount,
								SCReportPtr * theReportDataOut)
{
	SCResult aResult = SCResult_NO_ERROR;
	SCIReport * aReport = NULL;
	SCInterface * aInterface = (SCInterface*)theInterface;

	GS_ASSERT(theInterface != NULL);
	GS_ASSERT(theReportDataOut != NULL);
	if (theInterface == NULL)
		return SCResult_INVALID_PARAMETERS;
	if (theReportDataOut == NULL)
		return SCResult_INVALID_PARAMETERS;
	
	aResult = sciCreateReport(aInterface->mSessionId, theHeaderVersion, 
		thePlayerCount, theTeamCount, &aReport);
	if (aResult == SCResult_NO_ERROR)
		*theReportDataOut = (SCReportPtr)aReport;

	return aResult;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportBeginGlobalData(SCReportPtr theReport)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;

	return sciReportBeginGlobalData(aReport);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportBeginPlayerData(SCReportPtr theReport)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;

	return sciReportBeginPlayerData(aReport);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportBeginTeamData(SCReportPtr theReport)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;

	return sciReportBeginTeamData(aReport);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportBeginNewTeam(SCReportPtr theReport)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;
	//return SCResult_NO_ERROR;
	return sciReportBeginNewTeam(aReport);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportBeginNewPlayer(SCReportPtr theReport)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;
	//return SCResult_NO_ERROR;
	return sciReportBeginNewPlayer(aReport);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportEnd(SCReportPtr theReport, gsi_bool isAuth, SCGameStatus theStatus)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;

	return sciReportEnd(aReport, isAuth, theStatus);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportSetPlayerData(SCReportPtr theReport,
								       gsi_u32 thePlayerIndex,
								       const gsi_u8  thePlayerConnectionId[SC_CONNECTION_GUID_SIZE],
								       gsi_u32 thePlayerTeamId,
								       SCGameResult result,
								       gsi_u32 theProfileId,
								       const GSLoginCertificate * certificate,
								       const gsi_u8  theAuthHash[16])
{
	SCIReport * aReport = NULL;
	SCResult aResult = SCResult_NO_ERROR;
	gsi_u32 i;
	gsi_bool isNewTeam = gsi_true;

	GS_ASSERT(theReport != NULL);
	GS_ASSERT(certificate != NULL);
	GSI_UNUSED(theProfileId);
	aReport = (SCIReport*)theReport;
	
	// store the team ID here and use this in order to index the team game results
	for (i=0; i<aReport->mNumTeamsReported; i++)
	{
		// has the team already been reported by another player?
		if (aReport->mTeamIds[i] == thePlayerTeamId)
		{
			isNewTeam = gsi_false;
			break;
		}
	}
	
	if (isNewTeam)
	{	
		aReport->mNumTeamsReported++;

		// have they gone over the limit of teams? If so, this is an error
		GS_ASSERT(aReport->mNumTeamsReported < SC_MAX_NUM_TEAMS);
		if (aReport->mNumTeamsReported > SC_MAX_NUM_TEAMS)
			return SCResult_OUT_OF_MEMORY;	
		
		// if no problems, store the teamId
		aReport->mTeamIds[aReport->mNumTeamsReported-1] = thePlayerTeamId;
	}


	if (aResult == SCResult_NO_ERROR) aResult = sciReportSetPlayerConnectionId(aReport, thePlayerIndex, thePlayerConnectionId);
	if (aResult == SCResult_NO_ERROR) aResult = sciReportSetPlayerTeamIndex   (aReport, thePlayerIndex, thePlayerTeamId);
	if (aResult == SCResult_NO_ERROR) aResult = sciReportSetPlayerGameResult  (aReport, thePlayerIndex, result);
	if (aResult == SCResult_NO_ERROR) aResult = sciReportSetPlayerAuthInfo    (aReport, thePlayerIndex, certificate, theAuthHash);

	return aResult;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportSetTeamData(const SCReportPtr theReport,
									 gsi_u32     theTeamId,
									 SCGameResult result)
{
	SCIReport * aReport = NULL;
	gsi_u32 i;
	gsi_i32 aTeamIndex = -1;
	GS_ASSERT(theReport != NULL);
	GS_ASSERT(result < SCGameResultMax);

	aReport = (SCIReport*)theReport;

	// redundancy check - have they gone over the max limit of teams?
	GS_ASSERT(aReport->mNumTeamsReported < SC_MAX_NUM_TEAMS);
		
	// find the proper team index based on the teamId
	for (i=0; i<aReport->mNumTeamsReported; i++)
	{
		if (aReport->mTeamIds[i] == theTeamId)
		{
			aTeamIndex = (gsi_i32)i;
			break;
		}
	}

	// if no such team exists in our list, an invalid ID was given
	if (aTeamIndex == -1)
		return SCResult_INVALID_PARAMETERS;
	
	// the teamindex reported here needs to be 0 based, which is why we subtract 1
	return sciReportSetTeamGameResult(aReport, (gsi_u32)aTeamIndex, result);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportSetAsMatchless(const SCReportPtr theReport)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;
	return(sciReportSetAsMatchless(aReport));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportAddIntValue(const SCReportPtr theReport, 
									 gsi_u16     theKeyId, 
									 gsi_i32     theValue)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;
	return sciReportAddIntValue(aReport, theKeyId, theValue);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportAddInt64Value(SCReportPtr theReport, 
									   gsi_u16 theKeyId, 
									   gsi_i64 theValue)
{
	SCIReport *aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport *)theReport;
	return sciReportAddInt64Value(aReport, theKeyId, theValue);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportAddShortValue(const SCReportPtr theReport, 
									   gsi_u16     theKeyId, 
									   gsi_i16     theValue)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;
	return sciReportAddShortValue(aReport, theKeyId, theValue);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportAddByteValue(const SCReportPtr theReport, 
									  gsi_u16     theKeyId, 
									  gsi_i8      theValue)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;
	return sciReportAddByteValue(aReport, theKeyId, theValue);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportAddFloatValue(const SCReportPtr  theReport, 
									   gsi_u16      theKeyId, 
									   float		theValue)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;
	return sciReportAddFloatValue(aReport, theKeyId, theValue);

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scReportAddStringValue(const SCReportPtr  theReport, 
									    gsi_u16      theKeyId, 
									    const gsi_char * theValue)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	GS_ASSERT(theValue != NULL);
	aReport = (SCIReport*)theReport;
	return sciReportAddStringValue(aReport, theKeyId, theValue);

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scDestroyReport(SCReportPtr theReport)
{
	SCIReport * aReport = NULL;
	GS_ASSERT(theReport != NULL);
	aReport = (SCIReport*)theReport;

	return sciDestroyReport(aReport);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Create a random key for this cipher
SCResult SC_CALL scPeerCipherInit(const GSLoginCertificate * theLocalCert, SCPeerCipher * theCipher)
{
	// hardcoded to a 256-bit key block, although some ciphers will not use the entire block

	// What to use for a random seed?
	//     1) 16 bytes from our Util_Rand seeded with current time
	//     2) Hashed with 16 bytes from the cert public key (for a little extra randomness)
	//     3) Repeat 1-2 for every 16 bytes of key block needed
	unsigned char randBytes[32];
	int i=0;
	GSMD5_CTX md5;

	GS_ASSERT(theCipher != NULL);
	//CANNOT assert on constant expressions, they are meaningless.
	//GS_ASSERT(GS_CRYPT_RSA_BYTE_SIZE >= 32); // crypt lib must support 128-bit keys.

	#if defined (GS_CRYPT_NO_RANDOM)
		Util_RandSeed(0x2d2d2d2d);
	#else
		Util_RandSeed(current_time());
	#endif

	for (i=0; i < 32; i++)
		randBytes[i] = (unsigned char)Util_RandInt(0x00, 0xFF);

	// Calc the first half, bytes 0-15
	GSMD5Init(&md5);
	GSMD5Update(&md5, randBytes, 16);
	GSMD5Update(&md5, (unsigned char*)theLocalCert->mPeerPublicKey.modulus.mData, 16); // first 16 bytes of the key
	GSMD5Final(&theCipher->mKey[0], &md5);

	// Calc the second half, bytes 16-31
	GSMD5Init(&md5);
	GSMD5Update(&md5, &randBytes[16], 16);
	GSMD5Update(&md5, (unsigned char*)&theLocalCert->mPeerPublicKey.modulus.mData[16], 16); // last 16 bytes of the key
	GSMD5Final(&theCipher->mKey[16], &md5);

	theCipher->mKeyLen = 32;

	RC4Init(&theCipher->mRC4, theCipher->mKey, (int)theCipher->mKeyLen);
	theCipher->mInitialized = gsi_true;

	gsDebugFormat(GSIDebugCat_App, GSIDebugType_Misc, GSIDebugLevel_Notice, "Created PeerCipher key block\r\n");
	gsDebugBinary(GSIDebugCat_App, GSIDebugType_Misc, GSIDebugLevel_Notice, (char *)theCipher->mKey, (gsi_i32)theCipher->mKeyLen);
	
	return SCResult_NO_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scPeerCipherCreateKeyExchangeMsg(const GSLoginCertificate * theRemoteCert, const SCPeerCipher * theCipher, SCPeerKeyExchangeMsg theMsgOut)
{
	// Encrypt the key with the recipients public key, this makes it safe to transmit
	if (0 == gsCryptRSAEncryptBuffer(&theRemoteCert->mPeerPublicKey, theCipher->mKey, theCipher->mKeyLen, (unsigned char *)theMsgOut))
		return SCResult_NO_ERROR;
	else
		return SCResult_UNKNOWN_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scPeerCipherParseKeyExchangeMsg (const GSLoginCertificate * theLocalCert, const GSLoginPrivateData * theCertPrivateData, const SCPeerKeyExchangeMsg theMsg, SCPeerCipher * theCipherOut)
{
	GSI_UNUSED(theLocalCert);
	// Decrypt the key with the local player's private key
	if (0 == gsCryptRSADecryptBuffer(&theCertPrivateData->mPeerPrivateKey, (unsigned char *)theMsg, theCipherOut->mKey, &theCipherOut->mKeyLen))
	{
		gsDebugFormat(GSIDebugCat_App, GSIDebugType_Misc, GSIDebugLevel_Notice, "Decrypted PeerCipher key block\r\n");
		gsDebugBinary(GSIDebugCat_App, GSIDebugType_Misc, GSIDebugLevel_Notice, (char *)theCipherOut->mKey, (gsi_i32)theCipherOut->mKeyLen);
		RC4Init(&theCipherOut->mRC4, theCipherOut->mKey, (int)theCipherOut->mKeyLen);
		theCipherOut->mInitialized = gsi_true;
		return SCResult_NO_ERROR;
	}
	else
		return SCResult_UNKNOWN_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scPeerCipherEncryptBufferIV(SCPeerCipher * theCipher, gsi_u32 theMessageNum, gsi_u8 * theData, gsi_u32 theLen)
{
	RC4Context rc4;
	GSMD5_CTX md5;
	char tempHash[16];

	GS_ASSERT(gsi_is_true(theCipher->mInitialized));

	// Construct a new key for this message
	//    - Using the same key for all messages would be extremely unsafe
	GSMD5Init(&md5);
	GSMD5Update(&md5, theCipher->mKey, theCipher->mKeyLen);
	GSMD5Update(&md5, (unsigned char*)&theMessageNum, sizeof(theMessageNum));
	GSMD5Final((unsigned char *)tempHash, &md5);

	RC4Init(&rc4, (unsigned char *)tempHash, GS_CRYPT_MD5_HASHSIZE);
	RC4Encrypt(&rc4, (const unsigned char*)theData, theData, (int)theLen);
	return SCResult_NO_ERROR; 
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scPeerCipherDecryptBufferIV(SCPeerCipher * theCipher, gsi_u32 theMessageNum, gsi_u8 * theData, gsi_u32 theLen)
{
	return scPeerCipherEncryptBufferIV(theCipher, theMessageNum, theData, theLen);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scPeerCipherEncryptBuffer(SCPeerCipher * theCipher, gsi_u8 * theData, gsi_u32 theLen)
{
	// Encrypt the data using the RC4 context
	GS_ASSERT(gsi_is_true(theCipher->mInitialized));
	RC4Encrypt(&theCipher->mRC4, (const unsigned char*)theData, theData, (int)theLen);
	return SCResult_NO_ERROR; 
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SCResult SC_CALL scPeerCipherDecryptBuffer(SCPeerCipher * theCipher, gsi_u8 * theData, gsi_u32 theLen)
{
	return scPeerCipherEncryptBuffer(theCipher, theData, theLen);
}

SCResult SC_CALL scCreateQueryParameterList(SCQueryParameterListPtr *queryParams, gsi_u32 queryParamsCount)
{
	SCQueryParameterListPtr paramList = NULL;

	// We only accept valid SCQueryParameterList ** pointers,
	// otherwise the allocation will fail!
	GS_ASSERT(queryParams != NULL);
	if (queryParams == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryParams is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	GS_ASSERT(queryParamsCount > 0);
	if (queryParamsCount == 0)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryParamsCount is 0");
		return SCResult_INVALID_PARAMETERS;
	}

	// No caller should ever pass a non-null SCQueryParameterList *
	// We don't want to somehow overwrite the object
	GS_ASSERT(*queryParams == NULL);
	if (*queryParams != NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryParams points to non-NULL SCQueryParameterList");
		return SCResult_INVALID_PARAMETERS;
	}

	paramList = (SCQueryParameterListPtr)gsimalloc(sizeof(struct SCQueryParameterList));
	if (paramList == NULL)
	{
		GS_SC_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, 
			"Failed to allocate memory for: struct SCQueryParameterList");
		return SCResult_OUT_OF_MEMORY;
	}

	paramList->mCount = queryParamsCount;
	paramList->mNextSlot = 0;

	// allocate internal list of params
	paramList->mQueryParams = (SCQueryParameter *)gsimalloc(sizeof(struct SCQueryParameter) * paramList->mCount);
	if (paramList->mQueryParams == NULL)
	{
		GS_SC_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, 
			"Failed to allocate memory for: struct SCQueryParameter");
		return SCResult_OUT_OF_MEMORY;
	}

	memset(paramList->mQueryParams, 0, sizeof(struct SCQueryParameter) * paramList->mCount);
	*queryParams = paramList;
	return SCResult_NO_ERROR;
}

SCResult SC_CALL scAddQueryParameterToList(SCQueryParameterListPtr queryParams, const gsi_char *name, const gsi_char *value)
{
	GS_ASSERT(queryParams != NULL);
	if (queryParams == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryParams is NULL");
		return SCResult_INVALID_PARAMETERS;
	}
	GS_ASSERT(queryParams->mNextSlot < queryParams->mCount);
	if (queryParams->mNextSlot >= queryParams->mCount)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryParams is full, cannot add any more params");
		return SCResult_INVALID_PARAMETERS;
	}
	GS_ASSERT(name != NULL);
	if (name == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "name is NULL");
		return SCResult_INVALID_PARAMETERS;
	}
	GS_ASSERT(value != NULL);
	if (value == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "value is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

#ifdef GSI_UNICODE
	queryParams->mQueryParams[queryParams->mNextSlot].mName = goawstrdup(name);
	queryParams->mQueryParams[queryParams->mNextSlot].mValue = goawstrdup(value);
#else
	queryParams->mQueryParams[queryParams->mNextSlot].mName = goastrdup(name);
	queryParams->mQueryParams[queryParams->mNextSlot].mValue = goastrdup(value);
#endif
	++queryParams->mNextSlot;
	return SCResult_NO_ERROR;
}

SCResult SC_CALL scDestroyQueryParameterList(SCQueryParameterListPtr *queryParams)
{
	gsi_u32 i;
	SCQueryParameterListPtr ptrList = NULL;
	GS_ASSERT(queryParams != NULL);
	if (queryParams == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryParams is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	GS_ASSERT(*queryParams != NULL);
	if (*queryParams == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryParams points to NULL");
		return SCResult_INVALID_PARAMETERS;
	}	

	// dereferencing for readability and clarity
	ptrList = (*queryParams);
	for(i = 0; i < ptrList->mCount; i++)
	{
		gsifree(ptrList->mQueryParams[i].mName);
		gsifree(ptrList->mQueryParams[i].mValue);
	}

	gsifree(ptrList->mQueryParams);
	gsifree(ptrList);

	// this pointer points to the same thing as ptrList
	// Just need to clear the dereferenced queryParams
	*queryParams = NULL;
	return SCResult_NO_ERROR;
}

SCResult SC_CALL scDestroyGameStatsQueryResponse(SCGameStatsQueryResponse **response)
{
	gsi_u32 i, j;
	SCGameStatsQueryResponse *responseToDestroy = NULL;
	GS_ASSERT(response != NULL);
	if (response == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "response is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	GS_ASSERT(*response != NULL);
	if (*response == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "*response is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	responseToDestroy = *response;
	for (i = 0; i < responseToDestroy->mCategoriesCount; i++)
	{
		gsifree(responseToDestroy->mCategories[i].mName);
		for (j = 0; j < responseToDestroy->mCategories[i].mStatsCount; j++)
		{
			gsifree(responseToDestroy->mCategories[i].mStats[j].mName);
			gsifree(responseToDestroy->mCategories[i].mStats[j].mValue);
		}
		gsifree(responseToDestroy->mCategories[i].mStats);
	}

	gsifree(responseToDestroy->mCategories);
	gsifree(responseToDestroy);

	*response = NULL;
	return SCResult_NO_ERROR;
}

SCResult SC_CALL scDestroyPlayerStatsQueryResponse(SCPlayerStatsQueryResponse **response)
{
	gsi_u32 i, j, k;
	SCPlayerStatsQueryResponse *responseToDestroy = NULL;
	GS_ASSERT(response != NULL);
	if (response == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "response is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	GS_ASSERT(*response != NULL);
	if (*response == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "*response is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	responseToDestroy = *response;
	for (i = 0; i < responseToDestroy->mCategoriesCount; i++)
	{
		gsifree(responseToDestroy->mCategories[i].mName);
		for (j = 0; j < responseToDestroy->mCategories[i].mPlayersCount; j++)
		{
			for (k = 0; k < responseToDestroy->mCategories[i].mPlayers[j].mStatsCount; k++)
			{
				gsifree(responseToDestroy->mCategories[i].mPlayers[j].mStats[k].mName);
				gsifree(responseToDestroy->mCategories[i].mPlayers[j].mStats[k].mValue);
			}
			gsifree(responseToDestroy->mCategories[i].mPlayers[j].mStats);
		}
		gsifree(responseToDestroy->mCategories[i].mPlayers);
	}

	gsifree(responseToDestroy->mCategories);
	gsifree(responseToDestroy);

	*response = NULL;
	return SCResult_NO_ERROR;
}

SCResult SC_CALL scDestroyTeamStatsQueryResponse(SCTeamStatsQueryResponse **response)
{
	gsi_u32 i, j, k;
	SCTeamStatsQueryResponse *responseToDestroy = NULL;
	GS_ASSERT(response != NULL);
	if (response == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "response is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	GS_ASSERT(*response != NULL);
	if (*response == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "*response is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	responseToDestroy = *response;
	for (i = 0; i < responseToDestroy->mCategoriesCount; i++)
	{
		gsifree(responseToDestroy->mCategories[i].mName);
		for (j = 0; j < responseToDestroy->mCategories[i].mTeamsCount; j++)
		{
			for (k = 0; k < responseToDestroy->mCategories[i].mTeams[j].mStatsCount; k++)
			{
				gsifree(responseToDestroy->mCategories[i].mTeams[j].mStats[k].mName);
				gsifree(responseToDestroy->mCategories[i].mTeams[j].mStats[k].mValue);
			}
			gsifree(responseToDestroy->mCategories[i].mTeams[j].mStats);
		}
		gsifree(responseToDestroy->mCategories[i].mTeams);
	}

	gsifree(responseToDestroy->mCategories);
	gsifree(responseToDestroy);

	*response = NULL;
	return SCResult_NO_ERROR;
}

SCResult SC_CALL sciValidateParameters(SCInterface * iface,
									   const GSLoginCertificate *certificate,
									   const GSLoginPrivateData *privData,
									   int ruleSetVersion,
									   const char queryId[GS_GUID_SIZE],
									   SCQueryParameterListPtr queryParameters)
{
	GS_ASSERT(iface != NULL);
	if (iface == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "interfacePtr is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	GS_ASSERT(certificate != NULL);
	if (certificate == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "certificate is NULL");
		return SCResult_INVALID_PARAMETERS;
	}
	GS_ASSERT(privData != NULL);
	if (privData == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "privData is NULL");
		return SCResult_INVALID_PARAMETERS;
	}
	GS_ASSERT(ruleSetVersion > 0);
	if (ruleSetVersion <= 0)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "ruleSetVersion must be > 0");
		return SCResult_INVALID_PARAMETERS;
	}
	GS_ASSERT(queryId != NULL);
	if (queryId == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryId is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	if (queryParameters)
	{
		GS_ASSERT((queryParameters->mNextSlot == queryParameters->mCount));
		if (queryParameters->mNextSlot < queryParameters->mCount)
		{
			GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "queryParameters is empty or incomplete");
			return SCResult_INVALID_PARAMETERS;
		}
	}	

	if (!wsLoginCertIsValid(certificate))
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "certificate is invalid; get a new one from AuthService.");
		return SCResult_INVALID_PARAMETERS;
	}
	return SCResult_NO_ERROR;
}

SCResult SC_CALL scRunGameStatsQuery(SCInterfacePtr interfacePtr, 
									 const GSLoginCertificate * certificate, 
									 const GSLoginPrivateData * privData, 
									 int ruleSetVersion, 
									 const char queryId[GS_GUID_SIZE], 
									 SCQueryParameterListPtr queryParameters, 
									 SCGameStatsQueryCallback callback, 
									 void * userData )
{
	SCResult result = SCResult_NO_ERROR;
	SCInterface * intIface = (SCInterface*)interfacePtr;
	result = sciValidateParameters(intIface, certificate, privData, ruleSetVersion, queryId, queryParameters);
	if (result != SCResult_NO_ERROR)
	{
		return result;
	}

	GS_ASSERT(callback != NULL);
	if (callback == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "callback is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	// using the filtered data, serialize the data and call the web service 
	return sciWsRunGameStatsQuery(&intIface->mWebServices,
		intIface->mGameId,
		certificate,
		privData,
		ruleSetVersion,
		(char *)queryId,
		queryParameters,
		callback,
		userData);
}

SCResult SC_CALL scRunPlayerStatsQuery(SCInterfacePtr interfacePtr, 
									   const GSLoginCertificate *certificate, 
									   const GSLoginPrivateData *privData, 
									   int ruleSetVersion, 
									   const char queryId[GS_GUID_SIZE], 
									   const SCQueryParameterListPtr queryParameters, 
									   SCPlayerStatsQueryCallback callback, 
									   void *userData)
{
	SCResult result = SCResult_NO_ERROR;
	SCInterface * intIface = (SCInterface*)interfacePtr;
	result = sciValidateParameters(intIface, certificate, privData, ruleSetVersion, queryId, queryParameters);
	if (result != SCResult_NO_ERROR)
	{
		return result;
	}

	GS_ASSERT(callback != NULL);
	if (callback == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "callback is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	// using the filtered data, serialize the data and call the web service 
	return sciWsRunPlayerStatsQuery(&intIface->mWebServices,
		intIface->mGameId,
		certificate,
		privData,
		ruleSetVersion,
		(char *)queryId,
		queryParameters,
		callback,
		userData);
}

SCResult SC_CALL scRunTeamStatsQuery(SCInterfacePtr					interfacePtr,
									 const GSLoginCertificate *		certificate,
									 const GSLoginPrivateData *		privData,
									 int							ruleSetVersion,
									 const char					queryId[GS_GUID_SIZE],
									 const SCQueryParameterListPtr	queryParameters,
									 SCTeamStatsQueryCallback		callback,
									 void *							userData)
{
	SCResult result = SCResult_NO_ERROR;
	SCInterface * intIface = (SCInterface*)interfacePtr;
	result = sciValidateParameters(intIface, certificate, privData, ruleSetVersion, queryId, queryParameters);
	if (result != SCResult_NO_ERROR)
	{
		return result;
	}

	GS_ASSERT(callback != NULL);
	if (callback == NULL)
	{
		GS_SC_LOG(GSIDebugType_Misc, GSIDebugLevel_WarmError, "callback is NULL");
		return SCResult_INVALID_PARAMETERS;
	}

	// using the filtered data, serialize the data and call the web service 
	return sciWsRunTeamStatsQuery(&intIface->mWebServices,
		intIface->mGameId,
		certificate,
		privData,
		ruleSetVersion,
		(char *)queryId,
		queryParameters,
		callback,
		userData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*char* SC_CALL scGetServiceUrl()
{    
  return sciGetServiceUrl(); 
}*/

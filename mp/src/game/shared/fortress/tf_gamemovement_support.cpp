//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: Auto Repair
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "tf_gamemovement_support.h"
#include "tf_movedata.h"

CTFGameMovementSupport::CTFGameMovementSupport()
{
	m_pSupportData = NULL;

	m_vStandMins = SUPPORTCLASS_HULL_STAND_MIN;
	m_vStandMaxs = SUPPORTCLASS_HULL_STAND_MAX;
	m_vStandViewOffset = SUPPORTCLASS_VIEWOFFSET_STAND;

	m_vDuckMins = SUPPORTCLASS_HULL_DUCK_MIN;
	m_vDuckMaxs = SUPPORTCLASS_HULL_DUCK_MAX;
	m_vDuckViewOffset = SUPPORTCLASS_VIEWOFFSET_DUCK;
}

void CTFGameMovementSupport::ProcessClassMovement( CBaseTFPlayer *pPlayer, CTFMoveData *pTFMoveData )
{
	// Get the class specific data from the TFMoveData structure
	Assert( PlayerClassSupportData_t::PLAYERCLASS_ID == pTFMoveData->m_nClassID );
	m_pSupportData = &pTFMoveData->SupportData();

	// to test pass it through!!
	BaseClass::ProcessMovement( (CBasePlayer *)pPlayer, static_cast<CMoveData*>( pTFMoveData ) );
}

Vector CTFGameMovementSupport::GetPlayerMins( bool bDucked ) const
{
	return bDucked ? m_vDuckMins : m_vStandMins; 
}

Vector CTFGameMovementSupport::GetPlayerMaxs( bool bDucked ) const
{
	return bDucked ? m_vDuckMaxs : m_vStandMaxs;
}

Vector CTFGameMovementSupport::GetPlayerViewOffset( bool bDucked ) const
{
	return bDucked ? m_vDuckViewOffset : m_vStandViewOffset;
}

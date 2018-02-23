//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef TF_CLASS_RECON_H
#define TF_CLASS_RECON_H
#pragma once

#include "TFClassData_Shared.h"
#include "tf_playerclass.h"
#include "basetfcombatweapon_shared.h"//MS11

class CTFJetpackSteam;
class CReconJetpackLevel;


//=====================================================================
// Recon
class CPlayerClassRecon : public CPlayerClass
{
public:
	DECLARE_CLASS( CPlayerClassRecon, CPlayerClass );

	CPlayerClassRecon( CBaseTFPlayer *pPlayer, TFClass iClass  );
	~CPlayerClassRecon();

	virtual void	ClassActivate( void );
	virtual void	ClassDeactivate( void );

	virtual const char*	GetClassModelString( int nTeam );

	// Class Initialization
	virtual void	CreateClass(void);//MS11
	virtual void	RespawnClass(void);//MS11
	virtual bool	ClientCommand(const CCommand &args);//MS11

	virtual bool	ResupplyAmmo( float flPercentage, ResupplyReason_t reason );
	virtual void    SetupMoveData( void );			// Override class specific movement data here.
	virtual void	SetupSizeData( void );			// Override class specific size data here.
	virtual void	ResetViewOffset( void );

	PlayerClassReconData_t *GetClassData( void ) { return &m_ClassData; }

	virtual int		CanBuild( int iObjectType );

	virtual void	ClassThink( void );
	virtual void	GainedNewTechnology( CBaseTechnology *pTechnology );

	virtual void	CreatePersonalOrder();

	// Hooks
	virtual void	SetPlayerHull( void );
	virtual void	GetPlayerHull(bool bDucking, Vector &vecMin, Vector &vecMax);//MS11

	CNetworkVarEmbedded( PlayerClassReconData_t, m_ClassData );

protected:
	bool					m_bHasRadarScanner;
	CHandle<CBaseTFCombatWeapon> m_hWpnPlasma;//MS11
};

EXTERN_SEND_TABLE( DT_PlayerClassReconData )

#endif // TF_CLASS_RECON_H

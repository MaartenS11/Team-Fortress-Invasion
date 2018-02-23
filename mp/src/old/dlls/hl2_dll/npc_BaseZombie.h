//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef NPC_BASEZOMBIE_H
#define NPC_BASEZOMBIE_H
#ifdef _WIN32
#pragma once
#endif

#include "AI_BaseNPC.h"
#include "soundenvelope.h"

#define ZOM_ATTN_FOOTSTEP ATTN_IDLE

#define	ENVELOPE_CONTROLLER		(CSoundEnvelopeController::GetController())

#define ZOMBIE_MELEE_REACH	55

//
// Animation events.
//
#define	ZOMBIE_AE_ATTACK_RIGHT		0x01
#define	ZOMBIE_AE_ATTACK_LEFT		0x02
#define	ZOMBIE_AE_ATTACK_BOTH		0x03
#define ZOMBIE_AE_SWATITEM			10
#define ZOMBIE_AE_STARTSWAT			11
#define ZOMBIE_AE_STEP_LEFT			16 // Footsteps sounds
#define ZOMBIE_AE_STEP_RIGHT		17
#define ZOMBIE_AE_SCUFF_LEFT		18 // foot drags, scuff sound
#define ZOMBIE_AE_SCUFF_RIGHT		19
#define ZOMBIE_AE_ATTACK_SCREAM		20
#define ZOMBIE_AE_GET_UP			21 // for scripted get-ups
#define ZOMBIE_AE_POUND				22 // pounding walls/doors/obstacles


#define ZOMBIE_BODYGROUP_HEADCRAB	1	// The crab on our head


enum HeadcrabRelease_t
{
	RELEASE_NO,
	RELEASE_IMMEDIATE,	// release the headcrab right now!
	RELEASE_SCHEDULED,	// release the headcrab through the AI schedule.
};


//=========================================================
// schedules
//=========================================================
enum
{
	SCHED_ZOMBIE_CHASE_ENEMY = LAST_SHARED_SCHEDULE,
	SCHED_ZOMBIE_MOVE_SWATITEM,
	SCHED_ZOMBIE_SWATITEM,
	SCHED_ZOMBIE_RELEASECRAB,
	SCHED_ZOMBIE_MOVE_TO_AMBUSH,
	SCHED_ZOMBIE_WAIT_AMBUSH,
	SCHED_ZOMBIE_WANDER_MEDIUM,	// medium range wandering behavior.
	SCHED_ZOMBIE_WANDER_FAIL,

	LAST_BASE_ZOMBIE_SCHEDULE,
};

//=========================================================
// tasks
//=========================================================
enum 
{
	TASK_ZOMBIE_DELAY_SWAT = LAST_SHARED_TASK,
	TASK_ZOMBIE_GET_PATH_TO_PHYSOBJ,
	TASK_ZOMBIE_SWAT_ITEM,
	TASK_ZOMBIE_DIE,
	TASK_ZOMBIE_RELEASE_HEADCRAB,

	LAST_BASE_ZOMBIE_TASK,
};


//=========================================================
// Zombie conditions
//=========================================================
enum Zombie_Conds
{
	COND_ZOMBIE_CAN_SWAT_ATTACK = LAST_SHARED_CONDITION,
	COND_ZOMBIE_RELEASECRAB,

	LAST_BASE_ZOMBIE_CONDITION,
};



//=========================================================
//=========================================================
class CNPC_BaseZombie : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_BaseZombie, CAI_BaseNPC );

public:
	CNPC_BaseZombie( void );
	~CNPC_BaseZombie( void );

	void Spawn( void );
	void Precache( void );
	float MaxYawSpeed( void );
	bool OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	Class_T Classify( void );
	void HandleAnimEvent( animevent_t *pEvent );

	void OnStateChange( NPC_STATE OldState, NPC_STATE NewState );

	void KillMe( void )
	{
		m_iHealth = 5;
		OnTakeDamage( CTakeDamageInfo( this, this, m_iHealth * 2, DMG_GENERIC ) );
	}

	int MeleeAttack1Conditions ( float flDot, float flDist );

	// No range attacks
	int RangeAttack1Conditions ( float flDot, float flDist ) { return( 0 ); }
	
	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	int OnTakeDamage_Alive( const CTakeDamageInfo &info );

	virtual int SelectSchedule ( void );
	virtual int	SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	virtual void BuildScheduleTestBits( void );
	bool FValidateHintType( CAI_Hint *pHint);

	virtual int TranslateSchedule( int scheduleType );
	virtual Activity NPC_TranslateActivity( Activity baseAct );

	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	void GatherConditions( void );
	void PrescheduleThink( void );

	virtual void Event_Killed( const CTakeDamageInfo &info );
	void StopLoopingSounds();
	virtual void OnScheduleChange( void );

	virtual void PoundSound();

	// Custom damage/death 
	bool ShouldIgnite( const CTakeDamageInfo &info );
	virtual bool IsChopped( const CTakeDamageInfo &info );
	virtual void DieChopped( const CTakeDamageInfo &info );
	virtual void Ignite( float flFlameLifetime );
	virtual bool ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold );
	virtual HeadcrabRelease_t ShouldReleaseHeadcrab( const CTakeDamageInfo &info, float flDamageThreshold );

	// Headcrab releasing/breaking apart
	void RemoveHead( void );
	virtual void SetZombieModel( void ) { };
	virtual void BecomeTorso( const Vector &vecTorsoForce, const Vector &vecLegsForce );
	void ReleaseHeadcrab( const Vector &vecOrigin, const Vector &vecVelocity, bool fRemoveHead, bool fRagdollBody );

	// Swatting physics objects
	int GetSwatActivity( void );
	virtual bool FindNearestPhysicsObject( int iMaxMass );
	float DistToPhysicsEnt( void );

	// Returns whether we must be very near our enemy to attack them.
	virtual bool MustCloseToAttack(void) { return true; }

	virtual CBaseEntity *ClawAttack( float flDist, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch );

	// Sounds & sound envelope
	virtual bool ShouldPlayFootstepMoan( void );
	virtual void PainSound( void ) = 0;
	virtual void AlertSound( void ) = 0;
	virtual void IdleSound( void ) = 0;
	virtual void AttackSound( void ) = 0;
	virtual void AttackHitSound( void ) = 0;
	virtual void AttackMissSound( void ) = 0;
	virtual void FootstepSound( bool fRightFoot ) = 0;
	virtual void FootscuffSound( bool fRightFoot ) = 0;

	virtual void MoanSound( envelopePoint_t *pEnvelope, int iEnvelopeSize );
	bool ShouldPlayIdleSound( void ) { return false; }

	virtual const char *GetMoanSound( int nSound ) = 0;
	virtual const char *GetHeadcrabClassname( void ) = 0;
	virtual const char *GetLegsModel( void ) = 0;

protected:

	CSoundPatch	*m_pMoanSound;

	bool	m_fIsTorso;			// is this is a half-zombie?
	bool	m_fIsHeadless;		// is this zombie headless

	float	m_flNextFlinch;

	bool m_bHeadShot;			// Used to determine the survival of our crab beyond our death.

	//
	// Zombies catch on fire if they take too much burn damage in a given time period.
	//
	float	m_flBurnDamage;				// Keeps track of how much burn damage we've incurred in the last few seconds.
	float	m_flBurnDamageResetTime;	// Time at which we reset the burn damage.

	EHANDLE m_hPhysicsEnt;

	float m_flNextMoanSound;
	float m_flNextSwat;
	float m_flNextSwatScan;
	float m_crabHealth;
	float m_flMoanPitch;

	static int g_numZombies;	// counts total number of existing zombies.

	int m_iMoanSound; // each zombie picks one of the 4 and keeps it.

	static int ACT_ZOM_SWATLEFTMID;
	static int ACT_ZOM_SWATRIGHTMID;
	static int ACT_ZOM_SWATLEFTLOW;
	static int ACT_ZOM_SWATRIGHTLOW;
	static int ACT_ZOM_RELEASECRAB;
	static int ACT_ZOM_FALL;

	DECLARE_DATADESC();

	DEFINE_CUSTOM_AI;
};

#endif // NPC_BASEZOMBIE_H

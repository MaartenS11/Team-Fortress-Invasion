/*
Copyright (C) Valve Corporation
Copyright (C) 2014-2016 TalonBrave.info
*/

#include "cbase.h"
#include "hud_chat.h"
#include "clientmode_tfnormal.h"
#include "clientmode.h"
#include "hud.h"
#include "iinput.h"
#include "c_basetfplayer.h"
#include "hud_timer.h"
#include "usercmd.h"

#include "in_buttons.h"
#include "in_main.h"

#include "c_tf_playerclass.h"
#include "engine/IEngineSound.h"
#include "ienginevgui.h"

#include <vgui/IInput.h>
#include <vgui/IPanel.h>
#include <vgui_controls/AnimationController.h>

#include "commanderoverlaypanel.h"
#include "vgui_int.h"
#include "vgui_teammenu.h"
#include "vgui_classmenu.h"

#include "viewrender.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar Commander_SlueSpeed("commander_speed", "800.0", 0);
static ConVar Commander_MouseSpeed("commander_mousespeed", "200.0", 0);
static ConVar Commander_RightMoveSpeedScale("commander_rightmovespeedscale", "2.0", 0);
static ConVar Commander_InvertMouse("commander_invertmouse", "1.0", 0);

#define VISIBLE_STATIC_PROP_HEIGHT 7500

// Instance the singleton and expose the interface to it.
IClientMode *GetClientModeNormal( void )
{
	static ClientModeTFNormal g_ClientModeNormal;
	return &g_ClientModeNormal;
}

ClientModeTFNormal::ClientModeTFNormal() 
// hogsy start
	: commander_mode(false)
// hogsy end
{
	// hogsy start
	m_ScaledSlueSpeed = 10;
	m_Log_BaseEto2 = 1.4427f;	// factor to convert from a logarithm of base E to base 2.
	// hogsy end

	m_pViewport = new Viewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );
}

// Viewport

ClientModeTFNormal::Viewport::Viewport() : 
	m_CursorCommander(vgui::dc_arrow),
	m_CursorRightMouseMove(vgui::dc_hand)
{
	// hogsy start
	m_pOverlayPanel = new CCommanderOverlayPanel();
	m_pOverlayPanel->SetParent(this);
	m_pOverlayPanel->SetVisible(false);

#if 0
	SetPaintEnabled(false);
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);
#endif
	// hogsy end

	// use a custom scheme for the hud
	m_bHumanScheme = true;
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/clientschemehuman.res", "HudScheme");
	SetScheme(scheme);

	/*	Try to reload, so element colours are set appropriately to the custom schemes.
		Traditionally, just setting the scheme and then reloading it sometimes isn't enough, which appears to be a bug. 
		~hogsy
	*/
	ReloadScheme();
}

ClientModeTFNormal::Viewport::~Viewport()
{
}

void ClientModeTFNormal::Viewport::OnThink()
{
	BaseClass::OnThink();

	// See if scheme should change
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return;
	int team = pPlayer->GetTeamNumber();
	if ( !team )
		return;

	bool human = ( team == TEAM_HUMANS ) ? true : false;
	if ( human != m_bHumanScheme )
		ReloadScheme();
}

void ClientModeTFNormal::Viewport::ReloadScheme()
{
	// See if scheme should change
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return;

	const char *schemeFile = "resource/clientschemehuman.res";;

	int team = pPlayer->GetTeamNumber();
	if ( team )
	{
		m_bHumanScheme = ( team == TEAM_HUMANS ) ? true : false;
		if ( !m_bHumanScheme )
			schemeFile = "resource/clientschemealien.res";
	}

	BaseClass::ReloadScheme( schemeFile );
}

void ClientModeTFNormal::Viewport::CreateDefaultPanels()
{
	AddNewPanel(CreatePanelByName(PANEL_TEAM),"PANEL_TEAM");
	AddNewPanel(CreatePanelByName(PANEL_CLASS),"PANEL_CLASS");
}

IViewPortPanel *ClientModeTFNormal::Viewport::CreatePanelByName(const char *szPanelName)
{
	IViewPortPanel *newpanel = NULL;

	if(!V_strcmp(PANEL_TEAM,szPanelName))
		newpanel = new CFortressTeamMenu(this);
	else if(!V_strcmp(PANEL_CLASS,szPanelName))
		newpanel = new CFortressClassMenu(this);

	return newpanel;
}

// hogsy start

void ClientModeTFNormal::Viewport::MinimapClicked(const Vector& clickWorldPos)
{
	// Don't use Z... our current z is what we want
	Vector actualOrigin, offset;
	VectorCopy(clickWorldPos, actualOrigin);
	actualOrigin.z = m_pOverlayPanel->TacticalOrigin().z;

	m_pOverlayPanel->ActualToVisibleOffset(offset);
	VectorSubtract(actualOrigin, offset, actualOrigin);
	m_pOverlayPanel->BoundOrigin(actualOrigin);
	m_pOverlayPanel->TacticalOrigin() = actualOrigin;
}

void ClientModeTFNormal::Viewport::EnableCommanderMode()
{
	m_pOverlayPanel->SetVisible(true);

	vgui::VPANEL pRoot = VGui_GetClientDLLRootPanel();

	SetCursor(m_CursorCommander);
	vgui::surface()->SetCursor(m_CursorCommander);

	// Make the viewport fill the root panel.
	if (pRoot)
	{
		int wide, tall;
		vgui::ipanel()->GetSize(pRoot, wide, tall);
		SetBounds(0, 0, wide, tall);
	}

	C_BaseEntity *ent = cl_entitylist->GetEnt(0);
	if (m_pOverlayPanel && ent)
		m_pOverlayPanel->Enable();

	SetVisible(true);
}

void ClientModeTFNormal::Viewport::DisableCommanderMode()
{
	if (m_pOverlayPanel)
		m_pOverlayPanel->Disable();

	m_pOverlayPanel->SetVisible(false);

	SetVisible(false);
}

// hogsy end

vgui::Panel *ClientModeTFNormal::GetMinimapParent( void )
{
#if 0
	return GetCommanderOverlayPanel();
#else
	return GetViewport();
#endif
}

// Everything else

void ClientModeTFNormal::Update()
{
	BaseClass::Update();

	Vector mins, maxs;
	GetNormalViewport()->GetCommanderOverlayPanel()->GetVisibleArea(mins, maxs);
	MapData().SetVisibleArea(mins, maxs);
	
	HudCommanderOverlayMgr()->Tick();
}

#define WINDOWED_KEEPMOVING_PIXELS 300

bool ClientModeTFNormal::CreateMove( float flInputSampleTime, CUserCmd *cmd )
{
	// hogsy start
	if (IsCommanderMode())
	{
		int mx, my;
		int realx, realy;
		//int sidex, sidey;

		cmd->upmove = 0;

		// Figure out the speed scale so their perceptual movement speed stays the same.
		m_ScaledSlueSpeed = Commander_SlueSpeed.GetFloat() * GetNormalViewport()->GetCommanderOverlayPanel()->WorldUnitsPerPixel();
		m_ScaledMouseSpeed = Commander_MouseSpeed.GetFloat() * GetNormalViewport()->GetCommanderOverlayPanel()->WorldUnitsPerPixel();

		// Translate WASD while in commander mode...
		float temp = cmd->forwardmove;
		// Swap forward/right
		cmd->forwardmove = cmd->sidemove;
		// Invert right/left
		cmd->sidemove = -temp;

		// Normalize nonzero inputs to scaled speed
		if (cmd->forwardmove)
		{
			cmd->forwardmove = (cmd->forwardmove > 0) ? GetScaledSlueSpeed() : -GetScaledSlueSpeed();
		}
		if (cmd->sidemove)
		{
			cmd->sidemove = (cmd->sidemove > 0) ? GetScaledSlueSpeed() : -GetScaledSlueSpeed();
		}

		// Sample mouse
		::input->GetFullscreenMousePos(&mx, &my, &realx, &realy);

		if (GetNormalViewport()->GetCommanderOverlayPanel()->IsRightMouseMapMoving() || (in_commandermousemove.state & 1))
		{
			cmd->forwardmove = m_ScaledMouseSpeed * (mx - m_LastMouseX);
			cmd->sidemove = m_ScaledMouseSpeed * (my - m_LastMouseY);

			if (Commander_InvertMouse.GetInt())
			{
				cmd->forwardmove *= -1.0f;
				cmd->sidemove *= -1.0f;
			}

			//input->SetFullscreenMousePos( m_LastMouseX, m_LastMouseY );
			mx = m_LastMouseX;
			my = m_LastMouseY;
		}
		/*
		else if ( input->IsFullscreenMouse() )
		{
		if ( abs( realx - mx ) < WINDOWED_KEEPMOVING_PIXELS &&
		abs( realy - my ) < WINDOWED_KEEPMOVING_PIXELS )
		{
		sidex = 2;
		sidey = 2;

		// Check Size of viewport
		if ( mx < sidex )
		{
		cmd->forwardmove = -GetScaledSlueSpeed();
		}
		else if ( mx > ( ScreenWidth() - sidex ))
		{
		cmd->forwardmove = GetScaledSlueSpeed();
		}

		if ( my < sidey )
		{
		cmd->sidemove = -GetScaledSlueSpeed();
		}
		else if ( my > ( ScreenHeight() - sidey ) )
		{
		cmd->sidemove = GetScaledSlueSpeed();
		}
		}
		}
		*/

		m_LastMouseX = mx;
		m_LastMouseY = my;

		// Look straight down
		cmd->viewangles.x = 90; // 45;
		cmd->viewangles.y = 90; //45fmod( 3.0* 360 * (gpGlobals->curtime * 0.01), 360 );
		cmd->viewangles.z = 0;

		GetNormalViewport()->GetCommanderOverlayPanel()->TacticalAngles() = cmd->viewangles;

		IsometricMove(cmd);

		// Reset command
		ResetCommand(cmd);
	}
	else
	// hogsy end
	{
		// Let the player override the view.
		C_BaseTFPlayer *pPlayer = C_BaseTFPlayer::GetLocalPlayer();
		if (!pPlayer)
			return true;

		// Let the player at it
		pPlayer->CreateMove(flInputSampleTime, cmd);

		// Handle knockdowns
		if (pPlayer->CheckKnockdownAngleOverride())
		{
			QAngle ang;
			pPlayer->GetKnockdownAngles(ang);

			cmd->viewangles = ang;
			engine->SetViewAngles(ang);

			cmd->forwardmove = cmd->sidemove = cmd->upmove = 0;
			// Only keep score if it's down
			cmd->buttons &= (IN_SCORE);
		}

		if (pPlayer->GetPlayerClass())
		{
			C_PlayerClass *pPlayerClass = pPlayer->GetPlayerClass();
			pPlayerClass->CreateMove(flInputSampleTime, cmd);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ClientModeTFNormal::ShouldDrawViewModel( void )
{
	if (IsCommanderMode())
		return false;

	C_BaseTFPlayer *pPlayer = C_BaseTFPlayer::GetLocalPlayer();
	if(!pPlayer)
		return false;

	return pPlayer->ShouldDrawViewModel();
}

// hogsy start

bool ClientModeTFNormal::ShouldDrawEntity(C_BaseEntity *pEnt)
{
	if (IsCommanderMode())
		return MapData().IsEntityVisibleToTactical(pEnt);

	return BaseClass::ShouldDrawEntity(pEnt);
}

bool ClientModeTFNormal::ShouldDrawDetailObjects()
{
	if (IsCommanderMode())
		return false;

	return BaseClass::ShouldDrawDetailObjects();
}

bool ClientModeTFNormal::ShouldDrawParticles()
{
	if (IsCommanderMode())
	{
		Vector vCenter;
		float xSize, ySize;
		GetNormalViewport()->GetCommanderOverlayPanel()->GetOrthoRenderBox(vCenter, xSize, ySize);

		// Activate/deactivate particles rendering based on zoom level
		float maxSize = max(xSize, ySize);
		return (maxSize < VISIBLE_STATIC_PROP_HEIGHT);
	}

	return BaseClass::ShouldDrawParticles();
}

bool ClientModeTFNormal::ShouldDrawCrosshair(void)
{
	if (IsCommanderMode())
		return false;

	return BaseClass::ShouldDrawCrosshair();
}

// Always draw the local player while in commander mode
bool ClientModeTFNormal::ShouldDrawLocalPlayer(C_BasePlayer *pPlayer)
{
	if (IsCommanderMode())
		return true;

	return BaseClass::ShouldDrawLocalPlayer(pPlayer);
}

// The mode can choose to not draw fog
bool ClientModeTFNormal::ShouldDrawFog()
{
	if (IsCommanderMode())
		return false;

	return BaseClass::ShouldDrawFog();
}

// Zero out any movement in the command
void ClientModeTFNormal::ResetCommand(CUserCmd *cmd)
{
	if (IsCommanderMode())
	{
		cmd->buttons = 0;
		cmd->forwardmove = 0;
		cmd->sidemove = 0;
		cmd->upmove = 0;
		cmd->viewangles.Init();
	}
}

void ClientModeTFNormal::Layout()
{
	// https://github.com/hogsy/Fortress/issues/14
	BaseClass::Layout();

	// Force it to recompute it's boundaries
	if (IsCommanderMode())
	{
		GetNormalViewport()->GetCommanderOverlayPanel()->Disable();
		GetNormalViewport()->GetCommanderOverlayPanel()->Enable();
	}
}

ClientModeTFNormal::Viewport *ClientModeTFNormal::GetNormalViewport()
{
	Assert(m_pViewport);
	return static_cast< ClientModeTFNormal::Viewport * >(m_pViewport);
}

bool ClientModeTFNormal::GetOrthoParameters(CViewSetup *pSetup)
{
	Vector vCenter;
	float xSize, ySize;
	GetNormalViewport()->GetCommanderOverlayPanel()->GetOrthoRenderBox(vCenter, xSize, ySize);

	pSetup->m_bOrtho		= true;
	pSetup->m_OrthoLeft		= -xSize;
	pSetup->m_OrthoTop		= -ySize;
	pSetup->m_OrthoRight	= xSize;
	pSetup->m_OrthoBottom	= ySize;

	return true;
}

// TF2 commander mode movement logic
void ClientModeTFNormal::IsometricMove(CUserCmd *cmd)
{
	if (!IsCommanderMode())
		return;

	Vector forward, right, up;
	AngleVectors(cmd->viewangles, &forward, &right, &up);  // Determine movement angles

	// Copy movement amounts
	float fmove = cmd->forwardmove;
	float smove = cmd->sidemove;

	// No up / down movement
	forward.Init(1, 0, 0);
	right.Init(0, -1, 0);

	Vector wishvel;
	wishvel.Init();

	// Determine x and y parts of velocity
	for (int i = 0; i < 3; i++)
	{
		wishvel[i] = forward[i] * fmove + right[i] * smove;
	}

	GetNormalViewport()->GetCommanderOverlayPanel()->TacticalOrigin() += TICK_INTERVAL * wishvel;
	GetNormalViewport()->GetCommanderOverlayPanel()->BoundOrigin(GetNormalViewport()->GetCommanderOverlayPanel()->TacticalOrigin());
}

// Convert move to scaled move
float ClientModeTFNormal::Commander_ResampleMove(float in)
{
	float sign;
	float move;

	if (!in)
		return 0.0;

	sign = in > 0.0 ? 1.0 : -1.0;

	move = GetScaledSlueSpeed();

	return move * sign;
}

// Makes sure the mouse is over the same world position as it started
void ClientModeTFNormal::MoveMouse(Vector& worldPos)
{
	if (!IsCommanderMode())
		return;

	Vector worldCenter;
	float wworld, hworld;
	GetNormalViewport()->GetCommanderOverlayPanel()->GetOrthoRenderBox(worldCenter, wworld, hworld);
	wworld *= 2; hworld *= 2;

	Vector worldDelta;
	VectorSubtract(worldPos, worldCenter, worldDelta);

	int w, h;
	GetNormalViewport()->GetSize(w, h);

	int mx, my;
	mx = (worldDelta.x / wworld + 0.5f) * w;
	my = (0.5f - worldDelta.y / hworld) * h;

	// Clamp
	if (mx < 0)	mx = 0; else if (mx > w) mx = w;
	if (my < 0)	my = 0; else if (my > h) my = h;

	//input->SetFullscreenMousePos( mx, my );

	m_LastMouseX = mx;
	m_LastMouseY = my;
}

// Checks map bounds and determines ideal height for tactical view
float ClientModeTFNormal::GetHeightForMap(float zoom)
{
	Vector mins, maxs;
	MapData().GetMapBounds(mins, maxs);
	return maxs.z + TACTICAL_ZOFFSET;
}

// When in commander mode, force gl_clear and don't draw the skybox
void ClientModeTFNormal::PreRender(CViewSetup *pSetup)
{
	if (IsCommanderMode())
	{
		materials->ClearBuffers(true, false);

		GetOrthoParameters(pSetup);
		render->DrawTopView(true);
		Vector2D mins = pSetup->origin.AsVector2D();
		Vector2D maxs = pSetup->origin.AsVector2D();
		mins.x += pSetup->m_OrthoLeft;
		maxs.x += pSetup->m_OrthoRight;
		mins.y += pSetup->m_OrthoTop;
		maxs.y += pSetup->m_OrthoBottom;
		render->TopViewBounds(mins, maxs);

		// Activate/deactivate static prop + particles rendering based on zoom level
		Vector2D size;
		Vector2DSubtract(maxs, mins, size);
		float maxSize = max(size.x, size.y);
		bool showStaticProps = (maxSize < VISIBLE_STATIC_PROP_HEIGHT);
		ClientLeafSystem()->DrawStaticProps(showStaticProps);
#if 0	// Source doesn't support this anymore ~hogsy
		ClientLeafSystem()->DrawSmallEntities(showStaticProps);
#endif
	}

	BaseClass::PreRender(pSetup);
}

void ClientModeTFNormal::PostRender(void)
{
	if (IsCommanderMode())
	{
		render->DrawTopView(false);

		ClientLeafSystem()->DrawStaticProps(true);
#if 0	// Source doesn't support this anymore ~hogsy
		ClientLeafSystem()->DrawSmallEntities(true);
#endif
		return;
	}

	BaseClass::PostRender();
}

int ClientModeTFNormal::KeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding)
{
	if (IsCommanderMode())
	{
		switch (keynum)
		{
		case MOUSE_WHEEL_UP:
		case MOUSE_WHEEL_DOWN:
			// Swallow
			return 0;
		}
	}

	// Allow engine to process
	return BaseClass::KeyInput(down, keynum, pszCurrentBinding);
}

// Inherited from IMinimapClient
void ClientModeTFNormal::MinimapClicked(const Vector& clickWorldPos)
{
	if (GetNormalViewport())
		GetNormalViewport()->MinimapClicked(clickWorldPos);
}

CCommanderOverlayPanel *ClientModeTFNormal::GetCommanderOverlayPanel(void)
{
	if (GetNormalViewport())
		return GetNormalViewport()->GetCommanderOverlayPanel();

	return nullptr;
}

void ClientModeTFNormal::OverrideView(CViewSetup *pSetup)
{
	// Turn off vis when in commander mode
	if(IsCommanderMode())
	{
		// hogsy start
		CViewRender::GetMainView()->DisableVis();
		// hogsy end

		VectorCopy(GetNormalViewport()->GetCommanderOverlayPanel()->TacticalAngles(), pSetup->angles);
		VectorCopy(GetNormalViewport()->GetCommanderOverlayPanel()->TacticalOrigin(), pSetup->origin);
		return;
	}

	BaseClass::OverrideView(pSetup);
}

void ClientModeTFNormal::Enable()
{
	BaseClass::Enable();
}

void ClientModeTFNormal::Disable()
{
	BaseClass::Disable();
}

void ClientModeTFNormal::EnableCommanderMode()
{
	if (IsCommanderMode())
		return;

	commander_mode = true;

	GetNormalViewport()->EnableCommanderMode();

	HudCommanderOverlayMgr()->Enable(true);

	Layout();
}

void ClientModeTFNormal::DisableCommanderMode()
{
	if (!IsCommanderMode())
		return;

	commander_mode = false;

	::input->ResetMouse();

	GetNormalViewport()->DisableCommanderMode();

	HudCommanderOverlayMgr()->Enable(false);
}

// Called once at dll load time
void ClientModeTFNormal::Init()
{
	BaseClass::Init();
	//GetViewport()->RequestFocus();
}

void ClientModeTFNormal::LevelInit(const char *newmap)
{
	MapData().LevelInit(newmap);

	BaseClass::LevelInit(newmap);

	HudCommanderOverlayMgr()->LevelShutdown();

	GetNormalViewport()->GetCommanderOverlayPanel()->LevelInit(newmap);
	HudCommanderOverlayMgr()->LevelInit();

	GetNormalViewport()->Enable();
}

void ClientModeTFNormal::LevelShutdown(void)
{
	GetNormalViewport()->Disable();

	MapData().LevelShutdown();
	HudCommanderOverlayMgr()->LevelShutdown();
	GetNormalViewport()->GetCommanderOverlayPanel()->LevelShutdown();

	BaseClass::LevelShutdown();
}

// hogsy end

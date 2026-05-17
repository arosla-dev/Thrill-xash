/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "extdll.h"
#include "basemenu.h"
#include "utils.h"
#include "keydefs.h"
#include "menu_btnsbmp_table.h"
#include "menu_strings.h"

#define ART_BANNER		"gfx/shell/head_customize"

#define ID_BACKGROUND	0
#define ID_BANNER		1

#define ID_CANCEL		5

#define ID_SHADOWS		10
#define ID_NEAREST		11
#define ID_ROLLANGLE	12
#define ID_OVERBRIGHT	13
#define ID_SKILL		14
#define ID_PATHFIND		15
#define ID_NPCCUSTOM	16
#define ID_MISCCUSTOM	17


typedef struct
{
	menuFramework_s	menu;

	menuBitmap_s	background;
	menuBitmap_s	banner;

	menuCheckBox_s	Shadows;
	menuCheckBox_s	Nearest;
	menuSlider_s	Rollangle;
	menuSlider_s	Skill;
	menuCheckBox_s	Overbright;
	menuCheckBox_s	Goodpathfinding;
	menuPicButton_s	NpcCustomize;
	menuPicButton_s MiscCustomize;

	menuPicButton_s	cancel;

} uiPlayerSetup_t;

static uiPlayerSetup_t	uiPlayerSetup;

/*
=================
UI_PlayerSetup_GetConfig
=================
*/
static void UI_PlayerSetup_GetConfig( void )
{
	if( CVAR_GET_FLOAT( "r_shadows" ))
		uiPlayerSetup.Shadows.enabled = 1;

	if( CVAR_GET_FLOAT( "gl_texture_nearest" ))
		uiPlayerSetup.Nearest.enabled = 1;

	if( CVAR_GET_FLOAT( "gl_overbright" ))
		uiPlayerSetup.Overbright.enabled = 1;

	if( CVAR_GET_FLOAT( "cheat_goodpathfinding" ))
		uiPlayerSetup.Goodpathfinding.enabled = 1;

	uiPlayerSetup.Rollangle.curValue = RemapVal( CVAR_GET_FLOAT( "cl_rollangle" ), 0.0f, 1.0f, 0.0f, 1.0f );

	uiPlayerSetup.Skill.curValue = RemapVal( CVAR_GET_FLOAT( "skill" ), 1.0f, 3.0f, 0.0f, 1.0f );
}

/*
=================
UI_PlayerSetup_UpdateConfig
=================
*/
static void UI_PlayerSetup_UpdateConfig( void )
{
	CVAR_SET_FLOAT( "r_shadows", uiPlayerSetup.Shadows.enabled );
	//CVAR_SET_FLOAT( "gl_texture_nearest", uiPlayerSetup.Nearest.enabled );
	CVAR_SET_FLOAT( "gl_overbright", uiPlayerSetup.Overbright.enabled );
	CVAR_SET_FLOAT( "cl_rollangle", RemapVal( uiPlayerSetup.Rollangle.curValue, 0.0f, 1.0f, 0.0f, 1.0f ));
	CVAR_SET_FLOAT( "skill", RemapVal( uiPlayerSetup.Skill.curValue, 0.0f, 1.0f, 1.0f, 3.0f ));
	CVAR_SET_FLOAT( "cheat_goodpathfinding", uiPlayerSetup.Goodpathfinding.enabled );
}

static void UI_PlayerSetup_SetConfig( void )
{
	CVAR_SET_FLOAT( "r_shadows", uiPlayerSetup.Shadows.enabled );
	CVAR_SET_FLOAT( "gl_texture_nearest", uiPlayerSetup.Nearest.enabled );
	CVAR_SET_FLOAT( "gl_overbright", uiPlayerSetup.Overbright.enabled );
	CVAR_SET_FLOAT( "cl_rollangle", RemapVal( uiPlayerSetup.Rollangle.curValue, 0.0f, 1.0f, 0.0f, 1.0f ));
	CVAR_SET_FLOAT( "skill", RemapVal( uiPlayerSetup.Skill.curValue, 0.0f, 1.0f, 1.0f, 3.0f ));
	CVAR_SET_FLOAT( "cheat_goodpathfinding", uiPlayerSetup.Goodpathfinding.enabled );
}

/*
=================
UI_Background_Ownerdraw
=================
*/
static void UI_Background_Ownerdraw( void *self )
{
	menuCommon_s	*item = (menuCommon_s *)self;

	UI_DrawBackground_Callback( self );
}

/*
=================
UI_PlayerSetup_Callback
=================
*/
static void UI_PlayerSetup_Callback( void *self, int event )
{
	menuCommon_s	*item = (menuCommon_s *)self;

	if( event == QM_CHANGED )
	{
		switch( item->id )
		{
		case ID_NEAREST:
			UI_PlayerSetup_SetConfig();
			break;
		}
		UI_PlayerSetup_UpdateConfig();
		return;
	}

	if( event != QM_ACTIVATED )
		return;

	switch( item->id )
	{
	case ID_CANCEL:
		UI_PlayerSetup_SetConfig();
		UI_PopMenu();
		break;
	case ID_NPCCUSTOM:
		UI_NpcModel_Menu();
		break;
	case ID_MISCCUSTOM:
		UI_MiscCustom_Menu();
		break;
	}
}

/*
=================
UI_MsgBox_Ownerdraw
=================
*/
static void UI_MsgBox_Ownerdraw( void *self )
{
	menuCommon_s	*item = (menuCommon_s *)self;

	UI_FillRect( item->x, item->y, item->width, item->height, uiPromptBgColor );
}

/*
=================
UI_PlayerSetup_Init
=================
*/
static void UI_PlayerSetup_Init( void )
{
	memset( &uiPlayerSetup, 0, sizeof( uiPlayerSetup_t ));

	uiPlayerSetup.menu.vidInitFunc = UI_PlayerSetup_Init;
	
	uiPlayerSetup.background.generic.id = ID_BACKGROUND;
	uiPlayerSetup.background.generic.type = QMTYPE_BITMAP;
	uiPlayerSetup.background.generic.flags = QMF_INACTIVE;
	uiPlayerSetup.background.generic.x = 0;
	uiPlayerSetup.background.generic.y = 0;
	uiPlayerSetup.background.generic.width = 1024;
	uiPlayerSetup.background.generic.height = 768;
	uiPlayerSetup.background.pic = ART_BACKGROUND;
	uiPlayerSetup.background.generic.ownerdraw = UI_Background_Ownerdraw;

	uiPlayerSetup.banner.generic.id = ID_BANNER;
	uiPlayerSetup.banner.generic.type = QMTYPE_BITMAP;
	uiPlayerSetup.banner.generic.flags = QMF_INACTIVE|QMF_DRAW_ADDITIVE;
	uiPlayerSetup.banner.generic.x = UI_BANNER_POSX;
	uiPlayerSetup.banner.generic.y = UI_BANNER_POSY;
	uiPlayerSetup.banner.generic.width = UI_BANNER_WIDTH;
	uiPlayerSetup.banner.generic.height = UI_BANNER_HEIGHT;
	uiPlayerSetup.banner.pic = ART_BANNER;


	uiPlayerSetup.Shadows.generic.id = ID_SHADOWS;
	uiPlayerSetup.Shadows.generic.type = QMTYPE_CHECKBOX;
	uiPlayerSetup.Shadows.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_ACT_ONRELEASE|QMF_MOUSEONLY|QMF_DROPSHADOW;
	uiPlayerSetup.Shadows.generic.name = "Enable shadows";
	uiPlayerSetup.Shadows.generic.x = 72;
	uiPlayerSetup.Shadows.generic.y = 200;
	uiPlayerSetup.Shadows.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.Shadows.generic.statusText = "Enable drop shadows on models (off on software)";

	uiPlayerSetup.Overbright.generic.id = ID_OVERBRIGHT;
	uiPlayerSetup.Overbright.generic.type = QMTYPE_CHECKBOX;
	uiPlayerSetup.Overbright.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_ACT_ONRELEASE|QMF_MOUSEONLY|QMF_DROPSHADOW;
	uiPlayerSetup.Overbright.generic.name = "Enable Overbrights";
	uiPlayerSetup.Overbright.generic.x = 72;
	uiPlayerSetup.Overbright.generic.y = 250;
	uiPlayerSetup.Overbright.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.Overbright.generic.statusText = "Overbrights lighting of the world";

	uiPlayerSetup.Nearest.generic.id = ID_NEAREST;
	uiPlayerSetup.Nearest.generic.type = QMTYPE_CHECKBOX;
	uiPlayerSetup.Nearest.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_ACT_ONRELEASE|QMF_MOUSEONLY|QMF_DROPSHADOW;
	uiPlayerSetup.Nearest.generic.name = "Disable texture filtering";
	uiPlayerSetup.Nearest.generic.x = 72;
	uiPlayerSetup.Nearest.generic.y = 300;
	uiPlayerSetup.Nearest.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.Nearest.generic.statusText = "Disables opengl texture filtering";

	uiPlayerSetup.Goodpathfinding.generic.id = ID_PATHFIND;
	uiPlayerSetup.Goodpathfinding.generic.type = QMTYPE_CHECKBOX;
	uiPlayerSetup.Goodpathfinding.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_ACT_ONRELEASE|QMF_MOUSEONLY|QMF_DROPSHADOW;
	uiPlayerSetup.Goodpathfinding.generic.name = "Cheat - Good path finding";
	uiPlayerSetup.Goodpathfinding.generic.x = 72;
	uiPlayerSetup.Goodpathfinding.generic.y = 350;
	uiPlayerSetup.Goodpathfinding.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.Goodpathfinding.generic.statusText = "Monsters explode when they fail";

	uiPlayerSetup.MiscCustomize.generic.id = ID_MISCCUSTOM;
	uiPlayerSetup.MiscCustomize.generic.type = QMTYPE_BM_BUTTON;
	uiPlayerSetup.MiscCustomize.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW | QMF_NOTIFY;
	uiPlayerSetup.MiscCustomize.generic.name = "Misc Features";
	uiPlayerSetup.MiscCustomize.generic.statusText = "Change miscellaneous options.";
	uiPlayerSetup.MiscCustomize.generic.x = 72;
	uiPlayerSetup.MiscCustomize.generic.y = 395;
	uiPlayerSetup.MiscCustomize.generic.callback = UI_PlayerSetup_Callback;

	uiPlayerSetup.NpcCustomize.generic.id = ID_NPCCUSTOM;
	uiPlayerSetup.NpcCustomize.generic.type = QMTYPE_BM_BUTTON;
	uiPlayerSetup.NpcCustomize.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW | QMF_NOTIFY;
	uiPlayerSetup.NpcCustomize.generic.name = "Npc Models";
	uiPlayerSetup.NpcCustomize.generic.statusText = "Change the models used for npcs.";
	uiPlayerSetup.NpcCustomize.generic.x = 72;
	uiPlayerSetup.NpcCustomize.generic.y = 435;
	uiPlayerSetup.NpcCustomize.generic.callback = UI_PlayerSetup_Callback;

	//UI_UtilSetupPicButton(&uiVideo.vidOptions, PC_VID_OPT);

	uiPlayerSetup.Rollangle.generic.id = ID_ROLLANGLE;
	uiPlayerSetup.Rollangle.generic.type = QMTYPE_SLIDER;
	uiPlayerSetup.Rollangle.generic.flags = QMF_PULSEIFFOCUS|QMF_DROPSHADOW;
	uiPlayerSetup.Rollangle.generic.name = "View Roll Angle";
	//uiPlayerSetup.Rollangle.generic.x = 400;
	uiPlayerSetup.Rollangle.generic.x = 72;
	uiPlayerSetup.Rollangle.generic.y = 505;
	uiPlayerSetup.Rollangle.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.Rollangle.generic.statusText = "Adjust tilt angle when strafing";
	uiPlayerSetup.Rollangle.minValue = 0.0;
	uiPlayerSetup.Rollangle.maxValue = 1.0;
	uiPlayerSetup.Rollangle.range = 0.05f;

	uiPlayerSetup.Skill.generic.id = ID_SKILL;
	uiPlayerSetup.Skill.generic.type = QMTYPE_SLIDER;
	uiPlayerSetup.Skill.generic.flags = QMF_PULSEIFFOCUS|QMF_DROPSHADOW;
	uiPlayerSetup.Skill.generic.name = "Skill Easy|Medium|Hard";
	uiPlayerSetup.Skill.generic.x = 72;
	uiPlayerSetup.Skill.generic.y = 575;
	uiPlayerSetup.Skill.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.Skill.generic.statusText = "Change skill level";
	uiPlayerSetup.Skill.minValue = 0.0;
	uiPlayerSetup.Skill.maxValue = 1.0;
	uiPlayerSetup.Skill.range = 0.5f;


	uiPlayerSetup.cancel.generic.id = ID_CANCEL;
	uiPlayerSetup.cancel.generic.type = QMTYPE_BM_BUTTON;
	uiPlayerSetup.cancel.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW|QMF_NOTIFY;
	uiPlayerSetup.cancel.generic.name = "done";
	uiPlayerSetup.cancel.generic.statusText = "Go back to the previous menu";
	uiPlayerSetup.cancel.generic.x = 72;
	uiPlayerSetup.cancel.generic.y = 600;
	uiPlayerSetup.cancel.generic.callback = UI_PlayerSetup_Callback;

	UI_UtilSetupPicButton( &uiPlayerSetup.cancel, PC_DONE );

	UI_PlayerSetup_GetConfig();


	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.background );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.banner );
	
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.Shadows );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.Nearest );

	if( CVAR_GET_FLOAT( "sv_cheats" ))
		UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.Goodpathfinding );

	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.Overbright );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.Rollangle );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.Skill );
	UI_AddItem(&uiPlayerSetup.menu, (void*)&uiPlayerSetup.MiscCustomize);
	UI_AddItem( &uiPlayerSetup.menu, (void*)&uiPlayerSetup.NpcCustomize);

	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.cancel );
}

/*
=================
UI_PlayerSetup_Precache
=================
*/
void UI_PlayerSetup_Precache( void )
{
	PIC_Load( ART_BACKGROUND );
	PIC_Load( ART_BANNER );
}

/*
=================
UI_PlayerSetup_Menu
=================
*/
void UI_PlayerSetup_Menu( void )
{
	if( !CheckGameDll( )) return;

	UI_PlayerSetup_Precache();
	UI_PlayerSetup_Init();

	UI_PushMenu( &uiPlayerSetup.menu );
}
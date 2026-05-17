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

#include "mathlib.h"
#include "extdll.h"
#include "const.h"
#include "basemenu.h"
#include "utils.h"
#include "keydefs.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "menu_btnsbmp_table.h"

#pragma optimize("", off)
#define ART_BANNER		"gfx/shell/head_customize"

#define ID_BACKGROUND	0
#define ID_BANNER		1
#define ID_DONE		2
#define ID_ADVOPTIONS	3
#define ID_VIEW		4
#define ID_NAME		5
#define ID_MODEL		6
#define ID_TOPCOLOR		7
#define ID_BOTTOMCOLOR	8
#define ID_HIMODELS		9
#define ID_SHOWMODELS	10

#define MAX_PLAYERMODELS	100

typedef struct
{
	char		models[MAX_PLAYERMODELS][CS_SIZE];
	int		num_models;
	menuSlider_s		barney_modelnum;
	char		currentModel[CS_SIZE];

	ref_viewpass_t	rvp;
	cl_entity_t* ent;

	menuFramework_s	menu;

	menuBitmap_s	background;
	menuBitmap_s	banner;

	menuPicButton_s	done;
	menuPicButton_s	AdvOptions;
	menuBitmap_s	view;

	menuCheckBox_s	showModels;
	menuCheckBox_s	hiModels;
	menuSlider_s	topColor;
	menuSlider_s	bottomColor;

	menuField_s	name;
	menuSpinControl_s	model;
} uiBarneyModel_t;

static uiBarneyModel_t	uiBarneyModel;
static HIMAGE		playerImage = 0;	// keep actual
static char		lastImage[256];

/*
=================
UI_BarneyModel_CalcFov

assume refdef is valid
=================
*/
static void UI_BarneyModel_CalcFov(ref_viewpass_t* rvp)
{
	rvp->fov_x = 40.0f;
	float x = rvp->viewport[2] / tan(DEG2RAD(rvp->fov_x) * 0.5f);
	float half_fov_y = atan(rvp->viewport[3] / x);
	rvp->fov_y = RAD2DEG(half_fov_y) * 2;
}

/*
=================
UI_BarneyModel_FindModels
=================
*/
static void UI_BarneyModel_FindModels(void)
{
	char	name[256], path[256];
	char** filenames;
	int	i, numFiles;

	uiBarneyModel.num_models = 0;

	// Get file list
	filenames = FS_SEARCH("models/players*", &numFiles, TRUE);
	//if (!numFiles) filenames = FS_SEARCH("models/*", &numFiles, FALSE);
#if 1
	// add default singleplayer model
	strcpy(uiBarneyModel.models[uiBarneyModel.num_models], "Barney");
	uiBarneyModel.num_models++;
	strcpy(uiBarneyModel.models[uiBarneyModel.num_models], "indeep/barney");
	uiBarneyModel.num_models++;
	strcpy(uiBarneyModel.models[uiBarneyModel.num_models], "alpha/barney");
	uiBarneyModel.num_models++;
#endif
	// build the model list
	for (i = 0; i < numFiles; i++)
	{
		COM_FileBase(filenames[i], name);
		sprintf(path, "models/%s.mdl", name);
		//printf("[%s] is a string %d chars long\n", buffer, n);
		//ALERT(at_console, "strafe right GRUNT\n");
		if (!FILE_EXISTS(path)) continue;

		strcpy(uiBarneyModel.models[uiBarneyModel.num_models], name);
		uiBarneyModel.num_models++;
	}
}

/*
=================
UI_BarneyModel_GetConfig
=================
*/
static void UI_BarneyModel_GetConfig(void)
{
	int	i;

	strncpy(uiBarneyModel.name.buffer, CVAR_GET_STRING("name"), sizeof(uiBarneyModel.name.buffer));

	// find models
	UI_BarneyModel_FindModels();

	// select current model
	for (i = 0; i < uiBarneyModel.num_models; i++)
	{
		if (!_stricmp(uiBarneyModel.models[i], CVAR_GET_STRING("barney_model")))
		{
			uiBarneyModel.model.curValue = (float)i;
			break;
		}
	}

	if (gMenu.m_gameinfo.flags & GFL_NOMODELS)
		uiBarneyModel.model.curValue = 0.0f; // force to default


	strcpy(uiBarneyModel.currentModel, uiBarneyModel.models[(int)uiBarneyModel.model.curValue]);
	uiBarneyModel.model.maxValue = (float)(uiBarneyModel.num_models - 1);

	uiBarneyModel.topColor.curValue = CVAR_GET_FLOAT("topcolor") / 255;
	uiBarneyModel.bottomColor.curValue = CVAR_GET_FLOAT("bottomcolor") / 255;

	if (CVAR_GET_FLOAT("cl_himodels"))
		uiBarneyModel.hiModels.enabled = 1;

	if (CVAR_GET_FLOAT("ui_showmodels"))
		uiBarneyModel.showModels.enabled = 1;
}

/*
=================
UI_BarneyModel_SetConfig
=================
*/
static void UI_BarneyModel_SetConfig(void)
{

	CVAR_SET_STRING("name", uiBarneyModel.name.buffer);
	CVAR_SET_STRING("barney_model", uiBarneyModel.currentModel);
	CVAR_SET_FLOAT("topcolor", (int)(uiBarneyModel.topColor.curValue * 255));
	CVAR_SET_FLOAT("bottomcolor", (int)(uiBarneyModel.bottomColor.curValue * 255));
	CVAR_SET_FLOAT("cl_himodels", uiBarneyModel.hiModels.enabled);
	CVAR_SET_FLOAT("ui_showmodels", uiBarneyModel.showModels.enabled);
}

/*
=================
UI_BarneyModel_UpdateConfig
=================
*/
static void UI_BarneyModel_UpdateConfig(void)
{
	char	path[256], name[256];
	//char	newImage[256];
	int	topColor, bottomColor;

	// see if the model has changed
	if (_stricmp(uiBarneyModel.currentModel, uiBarneyModel.models[(int)uiBarneyModel.model.curValue]))
	{
		strcpy(uiBarneyModel.currentModel, uiBarneyModel.models[(int)uiBarneyModel.model.curValue]);
	}

	uiBarneyModel.model.generic.name = uiBarneyModel.models[(int)uiBarneyModel.model.curValue];
	strcpy(name, uiBarneyModel.models[(int)uiBarneyModel.model.curValue]);


		sprintf(path, "models/%s.mdl", name);
		//sprintf(newImage, "models/player/%s/%s.bmp", name, name);

	topColor = (int)(uiBarneyModel.topColor.curValue * 255);
	bottomColor = (int)(uiBarneyModel.bottomColor.curValue * 255);

	CVAR_SET_STRING("barney_model", uiBarneyModel.currentModel);
	CVAR_SET_FLOAT("cl_himodels", uiBarneyModel.hiModels.enabled);
	CVAR_SET_FLOAT("ui_showmodels", uiBarneyModel.showModels.enabled);
	CVAR_SET_FLOAT("topcolor", topColor);
	CVAR_SET_FLOAT("bottomcolor", bottomColor);


	// IMPORTANT: always set default model becuase we need to have something valid here
	// if you wish draw your playermodel as normal studiomodel please change "models/player.mdl" to path
	if (uiBarneyModel.ent)
		g_engfuncs.pfnSetModel(uiBarneyModel.ent, path);

}

/*
=================
UI_BarneyModel_Callback
=================
*/
static void UI_BarneyModel_Callback(void* self, int event)
{
	menuCommon_s* item = (menuCommon_s*)self;

	switch (item->id)
	{
	case ID_HIMODELS:
	case ID_SHOWMODELS:
		if (event == QM_PRESSED)
			((menuCheckBox_s*)self)->focusPic = UI_CHECKBOX_PRESSED;
		else ((menuCheckBox_s*)self)->focusPic = UI_CHECKBOX_FOCUS;
		break;
	}

	if (event == QM_CHANGED)
	{
		UI_BarneyModel_UpdateConfig();
		return;
	}

	if (event != QM_ACTIVATED)
		return;

	switch (item->id)
	{
	case ID_DONE:
		UI_BarneyModel_SetConfig();
		UI_PopMenu();
		break;
	case ID_ADVOPTIONS:
		UI_BarneyModel_SetConfig();
		UI_GameOptions_Menu();
		break;
	}
}
/*
=================
UI_BarneyModel_Ownerdraw
=================
*/
static void UI_BarneyModel_Ownerdraw(void* self)
{
	menuCommon_s* item = (menuCommon_s*)self;

	// draw the background
	UI_FillRect(item->x, item->y, item->width, item->height, uiPromptBgColor);

	// draw the rectangle
	UI_DrawRectangle(item->x, item->y, item->width, item->height, uiInputFgColor);


	if (!ui_showmodels->value && playerImage != 0)
	{
		PIC_Set(playerImage, 255, 255, 255, 255);
		PIC_Draw(item->x, item->y, item->width, item->height);
	}
	else
	{
		R_ClearScene();

		// clearing body for each frame
		uiBarneyModel.ent->curstate.body = 0;

		uiBarneyModel.ent->angles[1] += 0.5f;

		// draw the player model
		R_AddEntity(ET_NORMAL, uiBarneyModel.ent);
		R_RenderFrame(&uiBarneyModel.rvp);
	}
}

/*
=================
UI_BarneyModel_Init
=================
*/
static void UI_BarneyModel_Init(void)
{
	int addFlags = 0;

	memset(&uiBarneyModel, 0, sizeof(uiBarneyModel_t));

	if (gMenu.m_gameinfo.flags & GFL_NOMODELS)
		addFlags |= QMF_INACTIVE;

	uiBarneyModel.menu.vidInitFunc = UI_BarneyModel_Init;

	uiBarneyModel.background.generic.id = ID_BACKGROUND;
	uiBarneyModel.background.generic.type = QMTYPE_BITMAP;
	uiBarneyModel.background.generic.flags = QMF_INACTIVE;
	uiBarneyModel.background.generic.x = 0;
	uiBarneyModel.background.generic.y = 0;
	uiBarneyModel.background.generic.width = 1024;
	uiBarneyModel.background.generic.height = 768;
	uiBarneyModel.background.pic = ART_BACKGROUND;

	uiBarneyModel.banner.generic.id = ID_BANNER;
	uiBarneyModel.banner.generic.type = QMTYPE_BITMAP;
	uiBarneyModel.banner.generic.flags = QMF_INACTIVE | QMF_DRAW_ADDITIVE;
	uiBarneyModel.banner.generic.x = UI_BANNER_POSX;
	uiBarneyModel.banner.generic.y = UI_BANNER_POSY;
	uiBarneyModel.banner.generic.width = UI_BANNER_WIDTH;
	uiBarneyModel.banner.generic.height = UI_BANNER_HEIGHT;
	uiBarneyModel.banner.pic = ART_BANNER;

	uiBarneyModel.done.generic.id = ID_DONE;
	uiBarneyModel.done.generic.type = QMTYPE_BM_BUTTON;
	uiBarneyModel.done.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW;
	uiBarneyModel.done.generic.x = 72;
	uiBarneyModel.done.generic.y = 230;
	uiBarneyModel.done.generic.name = "Done";
	uiBarneyModel.done.generic.statusText = "Go back to the Multiplayer Menu";
	uiBarneyModel.done.generic.callback = UI_BarneyModel_Callback;

	UI_UtilSetupPicButton(&uiBarneyModel.done, PC_DONE);

	uiBarneyModel.AdvOptions.generic.id = ID_ADVOPTIONS;
	uiBarneyModel.AdvOptions.generic.type = QMTYPE_BM_BUTTON;
	uiBarneyModel.AdvOptions.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW;
	uiBarneyModel.AdvOptions.generic.x = 72;
	uiBarneyModel.AdvOptions.generic.y = 280;
	uiBarneyModel.AdvOptions.generic.name = "Adv. Options";
	uiBarneyModel.AdvOptions.generic.statusText = "Configure handness, fov and other advanced options";
	uiBarneyModel.AdvOptions.generic.callback = UI_BarneyModel_Callback;

	UI_UtilSetupPicButton(&uiBarneyModel.AdvOptions, PC_ADV_OPT);

	uiBarneyModel.view.generic.id = ID_VIEW;
	uiBarneyModel.view.generic.type = QMTYPE_BITMAP;
	uiBarneyModel.view.generic.flags = QMF_INACTIVE;
	uiBarneyModel.view.generic.x = 660;
	uiBarneyModel.view.generic.y = 260;
	uiBarneyModel.view.generic.width = 260;
	uiBarneyModel.view.generic.height = 320;
	uiBarneyModel.view.generic.ownerdraw = UI_BarneyModel_Ownerdraw;

	uiBarneyModel.name.generic.id = ID_NAME;
	uiBarneyModel.name.generic.type = QMTYPE_FIELD;
	uiBarneyModel.name.generic.flags = QMF_CENTER_JUSTIFY | QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW;
	uiBarneyModel.name.generic.x = 320;
	uiBarneyModel.name.generic.y = 260;
	uiBarneyModel.name.generic.width = 256;
	uiBarneyModel.name.generic.height = 36;
	uiBarneyModel.name.generic.callback = UI_BarneyModel_Callback;
	uiBarneyModel.name.generic.statusText = "Enter your multiplayer display name";
	uiBarneyModel.name.maxLength = 32;

	uiBarneyModel.model.generic.id = ID_MODEL;
	uiBarneyModel.model.generic.type = QMTYPE_SPINCONTROL;
	uiBarneyModel.model.generic.flags = QMF_CENTER_JUSTIFY | QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW | addFlags;
	uiBarneyModel.model.generic.x = FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS) ? 320 : 702;
	uiBarneyModel.model.generic.y = FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS) ? 320 : 590;
	uiBarneyModel.model.generic.width = FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS) ? 256 : 176;
	uiBarneyModel.model.generic.height = FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS) ? 36 : 32;
	uiBarneyModel.model.generic.callback = UI_BarneyModel_Callback;
	uiBarneyModel.model.generic.statusText = "Select a model for security guards.";
	uiBarneyModel.model.minValue = 0;
	uiBarneyModel.model.maxValue = 1;
	uiBarneyModel.model.range = 1;

	uiBarneyModel.topColor.generic.id = ID_TOPCOLOR;
	uiBarneyModel.topColor.generic.type = QMTYPE_SLIDER;
	uiBarneyModel.topColor.generic.flags = QMF_PULSEIFFOCUS | QMF_DROPSHADOW | addFlags;
	uiBarneyModel.topColor.generic.name = "Top color";
	uiBarneyModel.topColor.generic.x = 250;
	uiBarneyModel.topColor.generic.y = 550;
	uiBarneyModel.topColor.generic.width = 300;
	uiBarneyModel.topColor.generic.callback = UI_BarneyModel_Callback;
	uiBarneyModel.topColor.generic.statusText = "Set a player model top color";
	uiBarneyModel.topColor.minValue = 0.0;
	uiBarneyModel.topColor.maxValue = 1.0;
	uiBarneyModel.topColor.range = 0.05f;

	uiBarneyModel.bottomColor.generic.id = ID_BOTTOMCOLOR;
	uiBarneyModel.bottomColor.generic.type = QMTYPE_SLIDER;
	uiBarneyModel.bottomColor.generic.flags = QMF_PULSEIFFOCUS | QMF_DROPSHADOW | addFlags;
	uiBarneyModel.bottomColor.generic.name = "Bottom color";
	uiBarneyModel.bottomColor.generic.x = 250;
	uiBarneyModel.bottomColor.generic.y = 620;
	uiBarneyModel.bottomColor.generic.width = 300;
	uiBarneyModel.bottomColor.generic.callback = UI_BarneyModel_Callback;
	uiBarneyModel.bottomColor.generic.statusText = "Set a player model bottom color";
	uiBarneyModel.bottomColor.minValue = 0.0;
	uiBarneyModel.bottomColor.maxValue = 1.0;
	uiBarneyModel.bottomColor.range = 0.05f;

	uiBarneyModel.showModels.generic.id = ID_SHOWMODELS;
	uiBarneyModel.showModels.generic.type = QMTYPE_CHECKBOX;
	uiBarneyModel.showModels.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_ACT_ONRELEASE | QMF_MOUSEONLY | QMF_DROPSHADOW | addFlags;
	uiBarneyModel.showModels.generic.name = "Show 3D Preview";
	uiBarneyModel.showModels.generic.x = 72;
	uiBarneyModel.showModels.generic.y = 380;
	uiBarneyModel.showModels.generic.callback = UI_BarneyModel_Callback;
	uiBarneyModel.showModels.generic.statusText = "show 3D player models instead of preview thumbnails";

	uiBarneyModel.hiModels.generic.id = ID_HIMODELS;
	uiBarneyModel.hiModels.generic.type = QMTYPE_CHECKBOX;
	uiBarneyModel.hiModels.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_ACT_ONRELEASE | QMF_MOUSEONLY | QMF_DROPSHADOW | addFlags;
	uiBarneyModel.hiModels.generic.name = "High quality models";
	uiBarneyModel.hiModels.generic.x = 72;
	uiBarneyModel.hiModels.generic.y = 430;
	uiBarneyModel.hiModels.generic.callback = UI_BarneyModel_Callback;
	uiBarneyModel.hiModels.generic.statusText = "show hi-res models in multiplayer";

	UI_BarneyModel_GetConfig();

	UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.background);
	UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.banner);
	UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.done);
	//UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.AdvOptions);
	// disable playermodel preview for HLRally to prevent crash
	if (!FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS))
		UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.view);
	//UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.name);

	if (!FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS))
	{
		UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.model);
		//UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.topColor);
		//UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.bottomColor);
		//UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.showModels);
		//UI_AddItem(&uiBarneyModel.menu, (void*)&uiBarneyModel.hiModels);
	}

	// NOTE: must be called after UI_AddItem whan we sure what UI_ScaleCoords is done
	uiBarneyModel.rvp.viewport[0] = uiBarneyModel.view.generic.x;
	uiBarneyModel.rvp.viewport[1] = uiBarneyModel.view.generic.y;
	uiBarneyModel.rvp.viewport[2] = uiBarneyModel.view.generic.width;
	uiBarneyModel.rvp.viewport[3] = uiBarneyModel.view.generic.height;

	UI_BarneyModel_CalcFov(&uiBarneyModel.rvp);
	uiBarneyModel.ent = GET_MENU_EDICT();

	if (!uiBarneyModel.ent)
		return;

	// adjust entity params
	uiBarneyModel.ent->curstate.number = 1;	// IMPORTANT: always set playerindex to 1
	uiBarneyModel.ent->curstate.animtime = gpGlobals->time;	// start animation
	uiBarneyModel.ent->curstate.sequence = 0;
	uiBarneyModel.ent->curstate.scale = 1.0f;
	uiBarneyModel.ent->curstate.frame = 0.0f;
	uiBarneyModel.ent->curstate.framerate = 1.0f;
	uiBarneyModel.ent->curstate.effects |= EF_NOSHADOW;
	uiBarneyModel.ent->curstate.controller[0] = 127;
	uiBarneyModel.ent->curstate.controller[1] = 127;
	uiBarneyModel.ent->curstate.controller[2] = 127;
	uiBarneyModel.ent->curstate.controller[3] = 127;
	uiBarneyModel.ent->latched.prevcontroller[0] = 127;
	uiBarneyModel.ent->latched.prevcontroller[1] = 127;
	uiBarneyModel.ent->latched.prevcontroller[2] = 127;
	uiBarneyModel.ent->latched.prevcontroller[3] = 127;
	uiBarneyModel.ent->origin[0] = uiBarneyModel.ent->curstate.origin[0] = 45.0f / tan(DEG2RAD(uiBarneyModel.rvp.fov_y / 2.0f));
	uiBarneyModel.ent->origin[2] = uiBarneyModel.ent->curstate.origin[2] = -40.0f;
	uiBarneyModel.ent->angles[1] = uiBarneyModel.ent->curstate.angles[1] = 180.0f;
	//uiBarneyModel.ent->player = true; // yes, draw me as playermodel
}

/*
=================
UI_BarneyModel_Precache
=================
*/
void UI_BarneyModel_Precache(void)
{
	PIC_Load(ART_BACKGROUND);
	PIC_Load(ART_BANNER);
}

/*
=================
UI_BarneyModel_Menu
=================
*/
void UI_BarneyModel_Menu(void)
{
	//if (gMenu.m_gameinfo.gamemode == GAME_SINGLEPLAYER_ONLY)
		//return;

	UI_BarneyModel_Precache();
	UI_BarneyModel_Init();

	UI_BarneyModel_UpdateConfig();
	UI_PushMenu(&uiBarneyModel.menu);
}
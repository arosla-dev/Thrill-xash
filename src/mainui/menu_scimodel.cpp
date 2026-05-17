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
#define ID_VIEWSCI		4
#define ID_NAME		5
#define ID_MODELSCI		6
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
} uiSciModel_t;

static uiSciModel_t	uiSciModel;
static HIMAGE		playerImage = 0;	// keep actual
static char		lastImage[256];

/*
=================
UI_SciModel_CalcFov

assume refdef is valid
=================
*/
static void UI_SciModel_CalcFov(ref_viewpass_t* rvp)
{
	rvp->fov_x = 40.0f;
	float x = rvp->viewport[2] / tan(DEG2RAD(rvp->fov_x) * 0.5f);
	float half_fov_y = atan(rvp->viewport[3] / x);
	rvp->fov_y = RAD2DEG(half_fov_y) * 2;
}

/*
=================
UI_SciModel_FindModels
=================
*/
static void UI_SciModel_FindModels(void)
{
	char	name[256], path[256];
	char** filenames;
	int	i, numFiles;

	uiSciModel.num_models = 0;

	// Get file list
	filenames = FS_SEARCH("models/players*", &numFiles, TRUE);
	//if (!numFiles) filenames = FS_SEARCH("models/*", &numFiles, FALSE);
#if 1
	// add default singleplayer model
	strcpy(uiSciModel.models[uiSciModel.num_models], "scientist");
	uiSciModel.num_models++;
	strcpy(uiSciModel.models[uiSciModel.num_models], "indeep/scientist");
	uiSciModel.num_models++;
	strcpy(uiSciModel.models[uiSciModel.num_models], "alpha/scientist");
	uiSciModel.num_models++;
#endif
	// build the model list
	for (i = 0; i < numFiles; i++)
	{
		COM_FileBase(filenames[i], name);
		sprintf(path, "models/%s.mdl", name);
		//printf("[%s] is a string %d chars long\n", buffer, n);
		//ALERT(at_console, "strafe right GRUNT\n");
		if (!FILE_EXISTS(path)) continue;

		strcpy(uiSciModel.models[uiSciModel.num_models], name);
		uiSciModel.num_models++;
	}
}

/*
=================
UI_SciModel_GetConfig
=================
*/
static void UI_SciModel_GetConfig(void)
{
	int	i;

	strncpy(uiSciModel.name.buffer, CVAR_GET_STRING("name"), sizeof(uiSciModel.name.buffer));

	// find models
	UI_SciModel_FindModels();

	// select current model
	for (i = 0; i < uiSciModel.num_models; i++)
	{
		if (!_stricmp(uiSciModel.models[i], CVAR_GET_STRING("sci_model")))
		{
			uiSciModel.model.curValue = (float)i;
			break;
		}
	}

	if (gMenu.m_gameinfo.flags & GFL_NOMODELS)
		uiSciModel.model.curValue = 0.0f; // force to default


	strcpy(uiSciModel.currentModel, uiSciModel.models[(int)uiSciModel.model.curValue]);
	uiSciModel.model.maxValue = (float)(uiSciModel.num_models - 1);

	uiSciModel.topColor.curValue = CVAR_GET_FLOAT("topcolor") / 255;
	uiSciModel.bottomColor.curValue = CVAR_GET_FLOAT("bottomcolor") / 255;

	if (CVAR_GET_FLOAT("cl_himodels"))
		uiSciModel.hiModels.enabled = 1;

	if (CVAR_GET_FLOAT("ui_showmodels"))
		uiSciModel.showModels.enabled = 1;
}

/*
=================
UI_SciModel_SetConfig
=================
*/
static void UI_SciModel_SetConfig(void)
{

	CVAR_SET_STRING("name", uiSciModel.name.buffer);
	CVAR_SET_STRING("sci_model", uiSciModel.currentModel);
	CVAR_SET_FLOAT("topcolor", (int)(uiSciModel.topColor.curValue * 255));
	CVAR_SET_FLOAT("bottomcolor", (int)(uiSciModel.bottomColor.curValue * 255));
	CVAR_SET_FLOAT("cl_himodels", uiSciModel.hiModels.enabled);
	CVAR_SET_FLOAT("ui_showmodels", uiSciModel.showModels.enabled);
}

/*
=================
UI_SciModel_UpdateConfig
=================
*/
static void UI_SciModel_UpdateConfig(void)
{
	char	path[256], name[256];
	char	newImage[256];
	int	topColor, bottomColor;

	// see if the model has changed
	if (_stricmp(uiSciModel.currentModel, uiSciModel.models[(int)uiSciModel.model.curValue]))
	{
		strcpy(uiSciModel.currentModel, uiSciModel.models[(int)uiSciModel.model.curValue]);
	}

	uiSciModel.model.generic.name = uiSciModel.models[(int)uiSciModel.model.curValue];
	strcpy(name, uiSciModel.models[(int)uiSciModel.model.curValue]);


		sprintf(path, "models/%s.mdl", name);
		//sprintf(newImage, "models/player/%s/%s.bmp", name, name);

		Con_Printf("FUCK\n");
		Con_Printf("%s\n",path);

	topColor = (int)(uiSciModel.topColor.curValue * 255);
	bottomColor = (int)(uiSciModel.bottomColor.curValue * 255);

	CVAR_SET_STRING("sci_model", uiSciModel.currentModel);
	CVAR_SET_FLOAT("cl_himodels", uiSciModel.hiModels.enabled);
	CVAR_SET_FLOAT("ui_showmodels", uiSciModel.showModels.enabled);
	CVAR_SET_FLOAT("topcolor", topColor);
	CVAR_SET_FLOAT("bottomcolor", bottomColor);


	// IMPORTANT: always set default model becuase we need to have something valid here
	// if you wish draw your playermodel as normal studiomodel please change "models/player.mdl" to path
	if (uiSciModel.ent)
		ENGINE_SET_MODEL(uiSciModel.ent, path);

	if (!ui_showmodels->value)
	{
		if (_stricmp(lastImage, newImage))
		{
			if (lastImage[0] && playerImage)
			{
				// release old image
				PIC_Free(lastImage);
				lastImage[0] = '\0';
				playerImage = 0;
			}

			if (_stricmp(name, "player"))
			{
				sprintf(lastImage, "models/player/%s/%s.bmp", name, name);
				playerImage = PIC_Load(lastImage, PIC_KEEP_SOURCE); // if present of course
			}
			else if (lastImage[0] && playerImage)
			{
				// release old image
				PIC_Free(lastImage);
				lastImage[0] = '\0';
				playerImage = 0;
			}
		}

		if (playerImage != 0) // update remap colors
			PIC_Remap(playerImage, topColor, bottomColor);
	}
}

/*
=================
UI_SciModel_Callback
=================
*/
static void UI_SciModel_Callback(void* self, int event)
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
		UI_SciModel_UpdateConfig();
		return;
	}

	if (event != QM_ACTIVATED)
		return;

	switch (item->id)
	{
	case ID_DONE:
		UI_SciModel_SetConfig();
		UI_PopMenu();
		break;
	case ID_ADVOPTIONS:
		UI_SciModel_SetConfig();
		UI_GameOptions_Menu();
		break;
	}
}
/*
=================
UI_SciModel_Ownerdraw
=================
*/
static void UI_SciModel_Ownerdraw(void* self)
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
		uiSciModel.ent->curstate.body = 0;

		uiSciModel.ent->angles[1] += 0.5f;

		// draw the player model
		R_AddEntity(ET_NORMAL, uiSciModel.ent);
		R_RenderFrame(&uiSciModel.rvp);
	}
}

/*
=================
UI_SciModel_Init
=================
*/
static void UI_SciModel_Init(void)
{
	int addFlags = 0;

	memset(&uiSciModel, 0, sizeof(uiSciModel_t));

	if (gMenu.m_gameinfo.flags & GFL_NOMODELS)
		addFlags |= QMF_INACTIVE;

	uiSciModel.menu.vidInitFunc = UI_SciModel_Init;

	uiSciModel.background.generic.id = ID_BACKGROUND;
	uiSciModel.background.generic.type = QMTYPE_BITMAP;
	uiSciModel.background.generic.flags = QMF_INACTIVE;
	uiSciModel.background.generic.x = 0;
	uiSciModel.background.generic.y = 0;
	uiSciModel.background.generic.width = 1024;
	uiSciModel.background.generic.height = 768;
	uiSciModel.background.pic = ART_BACKGROUND;

	uiSciModel.banner.generic.id = ID_BANNER;
	uiSciModel.banner.generic.type = QMTYPE_BITMAP;
	uiSciModel.banner.generic.flags = QMF_INACTIVE | QMF_DRAW_ADDITIVE;
	uiSciModel.banner.generic.x = UI_BANNER_POSX;
	uiSciModel.banner.generic.y = UI_BANNER_POSY;
	uiSciModel.banner.generic.width = UI_BANNER_WIDTH;
	uiSciModel.banner.generic.height = UI_BANNER_HEIGHT;
	uiSciModel.banner.pic = ART_BANNER;

	uiSciModel.done.generic.id = ID_DONE;
	uiSciModel.done.generic.type = QMTYPE_BM_BUTTON;
	uiSciModel.done.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW;
	uiSciModel.done.generic.x = 72;
	uiSciModel.done.generic.y = 230;
	uiSciModel.done.generic.name = "Done";
	uiSciModel.done.generic.statusText = "Go back to the Multiplayer Menu";
	uiSciModel.done.generic.callback = UI_SciModel_Callback;

	UI_UtilSetupPicButton(&uiSciModel.done, PC_DONE);

	uiSciModel.AdvOptions.generic.id = ID_ADVOPTIONS;
	uiSciModel.AdvOptions.generic.type = QMTYPE_BM_BUTTON;
	uiSciModel.AdvOptions.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW;
	uiSciModel.AdvOptions.generic.x = 72;
	uiSciModel.AdvOptions.generic.y = 280;
	uiSciModel.AdvOptions.generic.name = "Adv. Options";
	uiSciModel.AdvOptions.generic.statusText = "Configure handness, fov and other advanced options";
	uiSciModel.AdvOptions.generic.callback = UI_SciModel_Callback;

	UI_UtilSetupPicButton(&uiSciModel.AdvOptions, PC_ADV_OPT);

	uiSciModel.view.generic.id = ID_VIEWSCI;
	uiSciModel.view.generic.type = QMTYPE_BITMAP;
	uiSciModel.view.generic.flags = QMF_INACTIVE;
	uiSciModel.view.generic.x = 660;
	uiSciModel.view.generic.y = 260;
	uiSciModel.view.generic.width = 260;
	uiSciModel.view.generic.height = 320;
	uiSciModel.view.generic.ownerdraw = UI_SciModel_Ownerdraw;

	uiSciModel.name.generic.id = ID_NAME;
	uiSciModel.name.generic.type = QMTYPE_FIELD;
	uiSciModel.name.generic.flags = QMF_CENTER_JUSTIFY | QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW;
	uiSciModel.name.generic.x = 320;
	uiSciModel.name.generic.y = 260;
	uiSciModel.name.generic.width = 256;
	uiSciModel.name.generic.height = 36;
	uiSciModel.name.generic.callback = UI_SciModel_Callback;
	uiSciModel.name.generic.statusText = "Enter your multiplayer display name";
	uiSciModel.name.maxLength = 32;

	uiSciModel.model.generic.id = ID_MODELSCI;
	uiSciModel.model.generic.type = QMTYPE_SPINCONTROL;
	uiSciModel.model.generic.flags = QMF_CENTER_JUSTIFY | QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW | addFlags;
	uiSciModel.model.generic.x = FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS) ? 320 : 702;
	uiSciModel.model.generic.y = FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS) ? 320 : 590;
	uiSciModel.model.generic.width = FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS) ? 256 : 176;
	uiSciModel.model.generic.height = FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS) ? 36 : 32;
	uiSciModel.model.generic.callback = UI_SciModel_Callback;
	uiSciModel.model.generic.statusText = "Select a model for scientists.";
	uiSciModel.model.minValue = 0;
	uiSciModel.model.maxValue = 1;
	uiSciModel.model.range = 1;

	uiSciModel.topColor.generic.id = ID_TOPCOLOR;
	uiSciModel.topColor.generic.type = QMTYPE_SLIDER;
	uiSciModel.topColor.generic.flags = QMF_PULSEIFFOCUS | QMF_DROPSHADOW | addFlags;
	uiSciModel.topColor.generic.name = "Top color";
	uiSciModel.topColor.generic.x = 250;
	uiSciModel.topColor.generic.y = 550;
	uiSciModel.topColor.generic.width = 300;
	uiSciModel.topColor.generic.callback = UI_SciModel_Callback;
	uiSciModel.topColor.generic.statusText = "Set a player model top color";
	uiSciModel.topColor.minValue = 0.0;
	uiSciModel.topColor.maxValue = 1.0;
	uiSciModel.topColor.range = 0.05f;

	uiSciModel.bottomColor.generic.id = ID_BOTTOMCOLOR;
	uiSciModel.bottomColor.generic.type = QMTYPE_SLIDER;
	uiSciModel.bottomColor.generic.flags = QMF_PULSEIFFOCUS | QMF_DROPSHADOW | addFlags;
	uiSciModel.bottomColor.generic.name = "Bottom color";
	uiSciModel.bottomColor.generic.x = 250;
	uiSciModel.bottomColor.generic.y = 620;
	uiSciModel.bottomColor.generic.width = 300;
	uiSciModel.bottomColor.generic.callback = UI_SciModel_Callback;
	uiSciModel.bottomColor.generic.statusText = "Set a player model bottom color";
	uiSciModel.bottomColor.minValue = 0.0;
	uiSciModel.bottomColor.maxValue = 1.0;
	uiSciModel.bottomColor.range = 0.05f;

	uiSciModel.showModels.generic.id = ID_SHOWMODELS;
	uiSciModel.showModels.generic.type = QMTYPE_CHECKBOX;
	uiSciModel.showModels.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_ACT_ONRELEASE | QMF_MOUSEONLY | QMF_DROPSHADOW | addFlags;
	uiSciModel.showModels.generic.name = "Show 3D Preview";
	uiSciModel.showModels.generic.x = 72;
	uiSciModel.showModels.generic.y = 380;
	uiSciModel.showModels.generic.callback = UI_SciModel_Callback;
	uiSciModel.showModels.generic.statusText = "show 3D player models instead of preview thumbnails";

	uiSciModel.hiModels.generic.id = ID_HIMODELS;
	uiSciModel.hiModels.generic.type = QMTYPE_CHECKBOX;
	uiSciModel.hiModels.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_ACT_ONRELEASE | QMF_MOUSEONLY | QMF_DROPSHADOW | addFlags;
	uiSciModel.hiModels.generic.name = "High quality models";
	uiSciModel.hiModels.generic.x = 72;
	uiSciModel.hiModels.generic.y = 430;
	uiSciModel.hiModels.generic.callback = UI_SciModel_Callback;
	uiSciModel.hiModels.generic.statusText = "show hi-res models in multiplayer";

	UI_SciModel_GetConfig();

	UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.background);
	UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.banner);
	UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.done);
	//UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.AdvOptions);
	// disable playermodel preview for HLRally to prevent crash
	if (!FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS))
		UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.view);
	//UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.name);

	if (!FBitSet(gMenu.m_gameinfo.flags, GFL_NOMODELS))
	{
		UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.model);
		//UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.topColor);
		//UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.bottomColor);
		//UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.showModels);
		//UI_AddItem(&uiSciModel.menu, (void*)&uiSciModel.hiModels);
	}

	// NOTE: must be called after UI_AddItem whan we sure what UI_ScaleCoords is done
	uiSciModel.rvp.viewport[0] = uiSciModel.view.generic.x;
	uiSciModel.rvp.viewport[1] = uiSciModel.view.generic.y;
	uiSciModel.rvp.viewport[2] = uiSciModel.view.generic.width;
	uiSciModel.rvp.viewport[3] = uiSciModel.view.generic.height;

	UI_SciModel_CalcFov(&uiSciModel.rvp);
	uiSciModel.ent = GET_MENU_EDICT();

	if (!uiSciModel.ent)
		return;

	// adjust entity params
	uiSciModel.ent->curstate.number = 1;	// IMPORTANT: always set playerindex to 1
	uiSciModel.ent->curstate.animtime = gpGlobals->time;	// start animation
	uiSciModel.ent->curstate.sequence = 0;
	uiSciModel.ent->curstate.scale = 1.0f;
	uiSciModel.ent->curstate.frame = 0.0f;
	uiSciModel.ent->curstate.framerate = 1.0f;
	uiSciModel.ent->curstate.effects |= EF_NOSHADOW;
	uiSciModel.ent->curstate.controller[0] = 127;
	uiSciModel.ent->curstate.controller[1] = 127;
	uiSciModel.ent->curstate.controller[2] = 127;
	uiSciModel.ent->curstate.controller[3] = 127;
	uiSciModel.ent->latched.prevcontroller[0] = 127;
	uiSciModel.ent->latched.prevcontroller[1] = 127;
	uiSciModel.ent->latched.prevcontroller[2] = 127;
	uiSciModel.ent->latched.prevcontroller[3] = 127;
	uiSciModel.ent->origin[0] = uiSciModel.ent->curstate.origin[0] = 45.0f / tan(DEG2RAD(uiSciModel.rvp.fov_y / 2.0f));
	uiSciModel.ent->origin[2] = uiSciModel.ent->curstate.origin[2] = -40.0f;
	uiSciModel.ent->angles[1] = uiSciModel.ent->curstate.angles[1] = 180.0f;
	//uiSciModel.ent->player = true; // yes, draw me as playermodel
}

/*
=================
UI_SciModel_Precache
=================
*/
void UI_SciModel_Precache(void)
{
	PIC_Load(ART_BACKGROUND);
	PIC_Load(ART_BANNER);
}

/*
=================
UI_SciModel_Menu
=================
*/
void UI_SciModel_Menu(void)
{
	//if (gMenu.m_gameinfo.gamemode == GAME_SINGLEPLAYER_ONLY)
		//return;

	UI_SciModel_Precache();
	UI_SciModel_Init();

	UI_SciModel_UpdateConfig();
	UI_PushMenu(&uiSciModel.menu);
}
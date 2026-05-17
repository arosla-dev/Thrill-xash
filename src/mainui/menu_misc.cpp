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

#define ID_VIEW		10
#define ID_MDL		11
#define ID_BHOP		12	// jay - bhop cvar

typedef struct
{
	menuFramework_s	menu;

	menuBitmap_s	background;
	menuBitmap_s	banner;

	menuCheckBox_s	Viewstretch;
	menuCheckBox_s	Mdlgamma;
	menuCheckBox_s	AllowBhop;	// jay - bhop cvar

	menuPicButton_s	cancel;

} uiMiscCustom_t;

static uiMiscCustom_t	uiMiscCustom;

/*
=================
UI_MiscCustom_GetConfig
=================
*/
static void UI_MiscCustom_GetConfig(void)
{
	if (CVAR_GET_FLOAT("view_stretch"))
		uiMiscCustom.Viewstretch.enabled = 1;

	if (CVAR_GET_FLOAT("mdl_gamma"))
		uiMiscCustom.Mdlgamma.enabled = 1;

	// jay - bhop cvar
	if (CVAR_GET_FLOAT("sv_allowbhop"))
		uiMiscCustom.AllowBhop.enabled = 1;
}

/*
=================
UI_MiscCustom_UpdateConfig
=================
*/
static void UI_MiscCustom_UpdateConfig(void)
{
	CVAR_SET_FLOAT("view_stretch", uiMiscCustom.Viewstretch.enabled);
	CVAR_SET_FLOAT("mdl_gamma", uiMiscCustom.Mdlgamma.enabled);
	CVAR_SET_FLOAT("sv_allowbhop", uiMiscCustom.AllowBhop.enabled);	// jay - bhop cvar
}

static void UI_MiscCustom_SetConfig(void)
{
	CVAR_SET_FLOAT("view_stretch", uiMiscCustom.Viewstretch.enabled);
	CVAR_SET_FLOAT("mdl_gamma", uiMiscCustom.Mdlgamma.enabled);
	CVAR_SET_FLOAT("sv_allowbhop", uiMiscCustom.AllowBhop.enabled);	// jay - bhop cvar
}

/*
=================
UI_Background_Ownerdraw
=================
*/
static void UI_Background_Ownerdraw(void* self)
{
	menuCommon_s* item = (menuCommon_s*)self;

	UI_DrawBackground_Callback(self);
}

/*
=================
UI_MiscCustom_Callback
=================
*/
static void UI_MiscCustom_Callback(void* self, int event)
{
	menuCommon_s* item = (menuCommon_s*)self;

	if (event == QM_CHANGED)
	{
		switch (item->id)
		{
		case ID_VIEW:
			UI_MiscCustom_SetConfig();
			break;
		}
		UI_MiscCustom_UpdateConfig();
		return;
	}

	if (event != QM_ACTIVATED)
		return;

	switch (item->id)
	{
	case ID_CANCEL:
		UI_MiscCustom_SetConfig();
		UI_PopMenu();
		break;
	}
}

/*
=================
UI_MsgBox_Ownerdraw
=================
*/
static void UI_MsgBox_Ownerdraw(void* self)
{
	menuCommon_s* item = (menuCommon_s*)self;

	UI_FillRect(item->x, item->y, item->width, item->height, uiPromptBgColor);
}

/*
=================
UI_MiscCustom_Init
=================
*/
static void UI_MiscCustom_Init(void)
{
	memset(&uiMiscCustom, 0, sizeof(uiMiscCustom_t));

	uiMiscCustom.menu.vidInitFunc = UI_MiscCustom_Init;

	uiMiscCustom.background.generic.id = ID_BACKGROUND;
	uiMiscCustom.background.generic.type = QMTYPE_BITMAP;
	uiMiscCustom.background.generic.flags = QMF_INACTIVE;
	uiMiscCustom.background.generic.x = 0;
	uiMiscCustom.background.generic.y = 0;
	uiMiscCustom.background.generic.width = 1024;
	uiMiscCustom.background.generic.height = 768;
	uiMiscCustom.background.pic = ART_BACKGROUND;
	uiMiscCustom.background.generic.ownerdraw = UI_Background_Ownerdraw;

	uiMiscCustom.banner.generic.id = ID_BANNER;
	uiMiscCustom.banner.generic.type = QMTYPE_BITMAP;
	uiMiscCustom.banner.generic.flags = QMF_INACTIVE | QMF_DRAW_ADDITIVE;
	uiMiscCustom.banner.generic.x = UI_BANNER_POSX;
	uiMiscCustom.banner.generic.y = UI_BANNER_POSY;
	uiMiscCustom.banner.generic.width = UI_BANNER_WIDTH;
	uiMiscCustom.banner.generic.height = UI_BANNER_HEIGHT;
	uiMiscCustom.banner.pic = ART_BANNER;


	uiMiscCustom.Viewstretch.generic.id = ID_VIEW;
	uiMiscCustom.Viewstretch.generic.type = QMTYPE_CHECKBOX;
	uiMiscCustom.Viewstretch.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_ACT_ONRELEASE | QMF_MOUSEONLY | QMF_DROPSHADOW;
	uiMiscCustom.Viewstretch.generic.name = "Enable Viewstretching";
	uiMiscCustom.Viewstretch.generic.x = 72;
	uiMiscCustom.Viewstretch.generic.y = 200;
	uiMiscCustom.Viewstretch.generic.callback = UI_MiscCustom_Callback;
	uiMiscCustom.Viewstretch.generic.statusText = "Enables the view stretch from the 0.52 alpha";

	uiMiscCustom.Mdlgamma.generic.id = ID_MDL;
	uiMiscCustom.Mdlgamma.generic.type = QMTYPE_CHECKBOX;
	uiMiscCustom.Mdlgamma.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_ACT_ONRELEASE | QMF_MOUSEONLY | QMF_DROPSHADOW;
	uiMiscCustom.Mdlgamma.generic.name = "Enable Alpha Model Gamma";
	uiMiscCustom.Mdlgamma.generic.x = 72;
	uiMiscCustom.Mdlgamma.generic.y = 250;
	uiMiscCustom.Mdlgamma.generic.callback = UI_MiscCustom_Callback;
	uiMiscCustom.Mdlgamma.generic.statusText = "Enables the darker gamma from the 0.52 alpha";

	// jay - bhop cvar
	uiMiscCustom.AllowBhop.generic.id = ID_BHOP;
	uiMiscCustom.AllowBhop.generic.type = QMTYPE_CHECKBOX;
	uiMiscCustom.AllowBhop.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_ACT_ONRELEASE | QMF_MOUSEONLY | QMF_DROPSHADOW;
	uiMiscCustom.AllowBhop.generic.name = "Allow Bunnyhopping";
	uiMiscCustom.AllowBhop.generic.x = 72;
	uiMiscCustom.AllowBhop.generic.y = 300;
	uiMiscCustom.AllowBhop.generic.callback = UI_MiscCustom_Callback;
	uiMiscCustom.AllowBhop.generic.statusText = "Allows the player to bunnyhop";


	uiMiscCustom.cancel.generic.id = ID_CANCEL;
	uiMiscCustom.cancel.generic.type = QMTYPE_BM_BUTTON;
	uiMiscCustom.cancel.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW | QMF_NOTIFY;
	uiMiscCustom.cancel.generic.name = "done";
	uiMiscCustom.cancel.generic.statusText = "Go back to the previous menu";
	uiMiscCustom.cancel.generic.x = 72;
	uiMiscCustom.cancel.generic.y = 600;
	uiMiscCustom.cancel.generic.callback = UI_MiscCustom_Callback;

	UI_UtilSetupPicButton(&uiMiscCustom.cancel, PC_DONE);

	UI_MiscCustom_GetConfig();


	UI_AddItem(&uiMiscCustom.menu, (void*)&uiMiscCustom.background);
	UI_AddItem(&uiMiscCustom.menu, (void*)&uiMiscCustom.banner);

	UI_AddItem(&uiMiscCustom.menu, (void*)&uiMiscCustom.Viewstretch);
	UI_AddItem(&uiMiscCustom.menu, (void*)&uiMiscCustom.Mdlgamma);
	UI_AddItem(&uiMiscCustom.menu, (void*)&uiMiscCustom.AllowBhop);	// jay - bhop cvar

	UI_AddItem(&uiMiscCustom.menu, (void*)&uiMiscCustom.cancel);
}

/*
=================
UI_MiscCustom_Precache
=================
*/
void UI_MiscCustom_Precache(void)
{
	PIC_Load(ART_BACKGROUND);
	PIC_Load(ART_BANNER);
}

/*
=================
UI_MiscCustom_Menu
=================
*/
void UI_MiscCustom_Menu(void)
{
	if (!CheckGameDll()) return;

	UI_MiscCustom_Precache();
	UI_MiscCustom_Init();

	UI_PushMenu(&uiMiscCustom.menu);
}
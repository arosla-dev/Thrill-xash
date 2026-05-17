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
#define ID_BARNEYCUSTOM	16
#define ID_SCICUSTOM	17

typedef struct
{
	menuFramework_s	menu;

	menuBitmap_s	background;
	menuBitmap_s	banner;

	menuPicButton_s	BarneyCustomize;
	menuPicButton_s	SciCustomize;

	menuPicButton_s	cancel;

} uiNpcModel_t;

static uiNpcModel_t	uiNpcModel;

/*
=================
UI_NpcModel_GetConfig
=================
*/
static void UI_NpcModel_GetConfig(void)
{

}

/*
=================
UI_NpcModel_UpdateConfig
=================
*/
static void UI_NpcModel_UpdateConfig(void)
{

}

static void UI_NpcModel_SetConfig(void)
{

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
UI_NpcModel_Callback
=================
*/
static void UI_NpcModel_Callback(void* self, int event)
{
	menuCommon_s* item = (menuCommon_s*)self;

	if (event != QM_ACTIVATED)
		return;

	switch (item->id)
	{
	case ID_CANCEL:
		UI_NpcModel_SetConfig();
		UI_PopMenu();
		break;
	case ID_BARNEYCUSTOM:
		UI_BarneyModel_Menu();
		break;
	case ID_SCICUSTOM:
		UI_SciModel_Menu();
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
UI_NpcModel_Init
=================
*/
static void UI_NpcModel_Init(void)
{
	memset(&uiNpcModel, 0, sizeof(uiNpcModel_t));

	uiNpcModel.menu.vidInitFunc = UI_NpcModel_Init;

	uiNpcModel.background.generic.id = ID_BACKGROUND;
	uiNpcModel.background.generic.type = QMTYPE_BITMAP;
	uiNpcModel.background.generic.flags = QMF_INACTIVE;
	uiNpcModel.background.generic.x = 0;
	uiNpcModel.background.generic.y = 0;
	uiNpcModel.background.generic.width = 1024;
	uiNpcModel.background.generic.height = 768;
	uiNpcModel.background.pic = ART_BACKGROUND;
	uiNpcModel.background.generic.ownerdraw = UI_Background_Ownerdraw;

	uiNpcModel.banner.generic.id = ID_BANNER;
	uiNpcModel.banner.generic.type = QMTYPE_BITMAP;
	uiNpcModel.banner.generic.flags = QMF_INACTIVE | QMF_DRAW_ADDITIVE;
	uiNpcModel.banner.generic.x = UI_BANNER_POSX;
	uiNpcModel.banner.generic.y = UI_BANNER_POSY;
	uiNpcModel.banner.generic.width = UI_BANNER_WIDTH;
	uiNpcModel.banner.generic.height = UI_BANNER_HEIGHT;
	uiNpcModel.banner.pic = ART_BANNER;


	uiNpcModel.BarneyCustomize.generic.id = ID_BARNEYCUSTOM;
	uiNpcModel.BarneyCustomize.generic.type = QMTYPE_BM_BUTTON;
	uiNpcModel.BarneyCustomize.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW | QMF_NOTIFY;
	uiNpcModel.BarneyCustomize.generic.name = "Barney";
	uiNpcModel.BarneyCustomize.generic.statusText = "Change the models used for security guards.";
	uiNpcModel.BarneyCustomize.generic.x = 72;
	uiNpcModel.BarneyCustomize.generic.y = 250;
	uiNpcModel.BarneyCustomize.generic.callback = UI_NpcModel_Callback;

	uiNpcModel.SciCustomize.generic.id = ID_SCICUSTOM;
	uiNpcModel.SciCustomize.generic.type = QMTYPE_BM_BUTTON;
	uiNpcModel.SciCustomize.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW | QMF_NOTIFY;
	uiNpcModel.SciCustomize.generic.name = "Scientist";
	uiNpcModel.SciCustomize.generic.statusText = "Change the models used for scientists.";
	uiNpcModel.SciCustomize.generic.x = 72;
	uiNpcModel.SciCustomize.generic.y = 200;
	uiNpcModel.SciCustomize.generic.callback = UI_NpcModel_Callback;

	uiNpcModel.cancel.generic.id = ID_CANCEL;
	uiNpcModel.cancel.generic.type = QMTYPE_BM_BUTTON;
	uiNpcModel.cancel.generic.flags = QMF_HIGHLIGHTIFFOCUS | QMF_DROPSHADOW | QMF_NOTIFY;
	uiNpcModel.cancel.generic.name = "done";
	uiNpcModel.cancel.generic.statusText = "Go back to the previous menu";
	uiNpcModel.cancel.generic.x = 72;
	uiNpcModel.cancel.generic.y = 600;
	uiNpcModel.cancel.generic.callback = UI_NpcModel_Callback;

	UI_UtilSetupPicButton(&uiNpcModel.cancel, PC_DONE);

	UI_NpcModel_GetConfig();


	UI_AddItem(&uiNpcModel.menu, (void*)&uiNpcModel.background);
	UI_AddItem(&uiNpcModel.menu, (void*)&uiNpcModel.banner);

	UI_AddItem(&uiNpcModel.menu, (void*)&uiNpcModel.BarneyCustomize);
	UI_AddItem(&uiNpcModel.menu, (void*)&uiNpcModel.SciCustomize);

	UI_AddItem(&uiNpcModel.menu, (void*)&uiNpcModel.cancel);
}

/*
=================
UI_NpcModel_Precache
=================
*/
void UI_NpcModel_Precache(void)
{
	PIC_Load(ART_BACKGROUND);
	PIC_Load(ART_BANNER);
}

/*
=================
UI_NpcModel_Menu
=================
*/
void UI_NpcModel_Menu(void)
{
	if (!CheckGameDll()) return;

	UI_NpcModel_Precache();
	UI_NpcModel_Init();

	UI_PushMenu(&uiNpcModel.menu);
}
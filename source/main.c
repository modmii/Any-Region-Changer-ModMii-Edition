/*-------------------------------------------------------------

regionchange.c -- Region Changing application

Copyright (C) 2008 tona

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <ogcsys.h>
#include <gccore.h>
#include <ogc/system.h>
#include <wiiuse/wpad.h>

#include "wiibasics.h"
#include "runtimeiospatch.h"
#include "sysconf.h"
#include "detect_settings.h"
#include "gecko.h"

#define ITEMS 11
#define SADR_LENGTH 0x1007 + 1
#define WARNING_SIGN "\x1b[30;1m\x1b[43;1m/!\\\x1b[37;1m\x1b[40m"
#define maxdata 256

// Why was this unsigned? Lol
int selected = 8;
char page_contents[ITEMS][64];

int lang, area, game, video, region, country, countrystr, eula;
u8 sadr[SADR_LENGTH];

static const char
	*languages[] = {"Japanese", "English", "German", "French", "Spanish", "Italian", "Dutch"}, // 9
	*areas[] = {"Japan", "USA", "Europe", "Australia", "Brazil", "Taiwan", "China", "Korea", "Hong Kong", "Asia", "Latin Am.", "S. Africa"}, // ?
	*regions[] = {"Japan", "USA", "Europe", "Korea"}, // 8
	*vmodes[] = {"NTSC", "PAL", "MPAL"}, // 8
	*eulas[] = {"Unread", "Read"}; // 8

void draw_credits()
{
	ClearScreen();
	PrintBanner();
	Console_SetPosition(3, 0);
	printf("\t\t\t\tCREDITS:\n\n\n");
	printf("\tOriginal Program:\n");
	printf("\t- Tona\n");
	printf("\n\tOthers in Original Credits: \n");
	printf("\t- Bushing, svpe, Marcan, Waninkoko, Crediar, \n");
	printf("\t- ChipD, Sorak05, serlex, pcg, callmebob, and dieforit .\n");
	printf("\n\tARC ModMii Edition: \n");
	printf("\t- Scooby74029(FakeCoder) - Coding \n");
	printf("\t- Xflak - testing, overall help \n");
	printf("\t- TheShadowEevee - KR region support \n");
	printf("\t- To Many Coders To Thank For The Great Code I Used ..... \n");
	printf("\n");
	Console_SetPosition(26, 0);
	ClearLine();
	Console_SetPosition(26, 20);
	printf("Press Any Button To Return ..... ");
}

void Draw_Disclaimer()
{
	ClearScreen();
	PrintBanner();
	printf("\n\t  This software comes supplied with absolutely no warranty.\n");
	printf("\t\t\t\tUse this software at your own risk.\n");

	printf("\n\n\n\t\t\t\t" WARNING_SIGN " IMPORTANT BRICK INFORMATION " WARNING_SIGN "\n\n");
	printf("\n\tSemi Bricks are caused by the Console Area setting not matching\n");
	printf("\tyour System Menu region. A semi-brick in itself is not terribly\n");
	printf("\tharmful, but it can easily deteriorate into a full brick--there\n");
	printf("\tare multiple simple triggers to do so.\n");
	printf("\n\n\n\tIn order to practice proper safety when using this application, \n");
	printf("\tplease make sure that your Console Area and System Menu region \n");
	printf("\tare congruent before rebooting your system. A warning will be\n");
	printf("\tdisplayed if they are not in agreement.\n");

	sleep(5);
	printf("\n\n\n\n\t\t\t\t[A] Continue             [Home] Exit\n");
	wait_key(WPAD_BUTTON_A);
}

void handleError(const char *string, int errorval)
{
	printf("Unexpected Error: %s Value: %d\n", string, errorval);
	printf("Press any key to quit\n");
	wait_anyKey();
	exit(0);
}

void getSettings(void)
{
	int ret;
	lang = SYSCONF_GetLanguage();
	area = SYSCONF_GetArea();
	game = SYSCONF_GetRegion();
	video = SYSCONF_GetVideo();
	eula = SYSCONF_GetEULA();
	if (lang < 0 || area < 0 || game < 0 || video < 0 || (eula != SYSCONF_ENOENT && eula < 0))
	{
		printf("Error getting settings!\n"
			"lang=%d\t"
			"area=%d\t"
			"game=%d\n"
			"video=%d\t"
			"eula=%d\t\n" , lang, area, game, video, eula);
		wait_anyKey();
		exit(1);
	}

	if (SYSCONF_GetLength("IPL.SADR") != SADR_LENGTH)
		handleError("IPL.SADR Length Incorrect", SYSCONF_GetLength("IPL.SADR"));

	ret = SYSCONF_Get("IPL.SADR", sadr, SADR_LENGTH);
	if (ret < 0)
		handleError("SYSCONF_Get IPL.SADR", ret);

	country = sadr[0];
	gprintf("\n\ncountry[%i] \n\n", country);
}

void saveSettings(void)
{
	Console_SetPosition(24, 0);
	int ret = 0;
	if (lang != SYSCONF_GetLanguage())
		ret = SYSCONF_SetLanguage(lang);
	if (ret)
		handleError("SYSCONF_SetLanguage", ret);

	if (area != SYSCONF_GetArea())
		ret = SYSCONF_SetArea(area);
	if (ret)
		handleError("SYSCONF_SetArea", ret);

	if (game != SYSCONF_GetRegion())
		ret = SYSCONF_SetRegion(game);
	if (ret)
		handleError("SYSCONF_SetRegion", ret);

	if (video != SYSCONF_GetVideo())
		ret = SYSCONF_SetVideo(video);
	if (ret)
		handleError("SYSCONF_SetVideo", ret);

	if (eula != SYSCONF_GetEULA())
		ret = SYSCONF_SetEULA(eula);
	if (ret)
		handleError("SYSCONF_SetEULA", ret);

	if (country != sadr[0])
	{
		memset(sadr, 0, SADR_LENGTH);
		sadr[0] = country;
		ret = SYSCONF_Set("IPL.SADR", sadr, SADR_LENGTH);
		if (ret)
			handleError("SYSCONF_Set IPL.SADR", ret);
	}
	// wait_anyKey();
	printf("Saving...");
	ret = SYSCONF_SaveChanges();
	if (ret < 0)
		handleError("SYSCONF_SaveChanges", ret);
	else
		printf("OK!\n");
	printf("Press any key to continue .....");
	wait_anyKey();
}

static char* itos(int i) {
	static char buffer[12];
	sprintf(buffer, "%d", i);

	return buffer;
}

typedef struct
{
	const char* name;
	const char* value;
} page_item;

void updatePage(void)
{
	const page_item page_items[] =
	{
		{ "Language Setting",			languages[lang]	},
		{ "Console Area Setting",		areas[area]		},
		{ "Game Region setting",		regions[game]	},
		{ "Console Video Mode",			vmodes[video]	},
		{ "Shop Country Code",			itos(country)	},
		{ "End-User License Agreement", (eula == SYSCONF_ENOENT) ? "Disabled" : eulas[eula]},

		{ "Revert Settings",	"Revert"	},
		{ "Save Settings",		"Save"		},
		{ "Auto Fix Settings",	"Fix"		},

		{ "Return to the Homebrew Channel", "Exit" },
		{}
	};
	const char* s_selected =   "	%-40s < %s >";
	const char* s_deselected = "	%-40s   %s  ";

	for (int i = 0; page_items[i].name != NULL; i++)
	{
		sprintf(page_contents[i], (i == selected) ? s_selected : s_deselected,
				page_items[i].name, page_items[i].value);
	}
}
char AREAtoSysMenuRegion(int area)
{
	// Data based on my own tests with AREA/Sysmenu
	switch (area)
	{
	case 0:
	case 5:
	case 6:
		return 'J';
	case 1:
	case 4:
	case 8:
	case 9:
	case 10:
	case 11:
		return 'U';
	case 2:
	case 3:
		return 'E';
	case 7:
		return 'K';
	default:
		return 0;
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//---------------------------------------------------------------------------------
	int ret = 0, i;
	u16 sysmenu_version;
	char sysmenu_region;
	bool needbreak = false;
	u32 buttons;
	int Current_Ios = 0;

	videoInit();
	ret = IosPatch_RUNTIME(true, false, false, false);
	if (ret < 0)
	{
		ret = IOS_ReloadIOS(249);
		if (ret < 0)
		{
			ret = IOS_ReloadIOS(236);
			if (ret < 0)
			{
				printf("\n\n\nUnable to find a suitable IOS to use. Consider updating the Homebrew Channel!\n");
				printf("Exiting in 5 seconds...\n");
				sleep(5);
				return -1;
			}
		}
	}

	ISFS_Initialize();
	WPAD_Init();
	PAD_Init();

	if (InitGecko())
		USBGeckoOutput();

	Current_Ios = IOS_GetVersion();
	gprintf("\n\ncurrent_ios [%i] \n\n", Current_Ios);

	sysmenu_version = get_installed_title_version(TITLE_ID(1, 2));
	sysmenu_region = get_sysmenu_region();
	gprintf("\n\nsysmenu_version[%u] sysmenu_region[%c] \n\n", sysmenu_version, sysmenu_region);
	gprintf("Init SYSCONF...");
	ret = SYSCONF_Init();
	if (ret < 0)
		handleError("SYSCONF_Init", ret);
	else
		gprintf("OK!\n");

	getSettings();
	region = game;
	Draw_Disclaimer();
	updatePage();

	while (1)
	{
		PrintBanner();
		printf("\n------------------------------------------------------------------------");
		printf("Edit Region Settings");
		if (sysmenu_region != 0 && sysmenu_region != AREAtoSysMenuRegion(area))
			printf("    " WARNING_SIGN " \x1b[41;1mWARNING: AREA/SysMenu MISMATCH!\x1b[40m " WARNING_SIGN);
		printf("\n------------------------------------------------------------------------");

		for (i = 0; i < 8; i++)
			printf("%s\n", page_contents[i]);

		printf("\n\t    Country Codes: \t1[JPN] 49[USA] 110[UK] 136[KOR]\n");

		printf("------------------------------------------------------------------------");
		printf("Auto Fix - SysMenu Region: %c (v%u)\n", sysmenu_region, sysmenu_version);
		printf("------------------------------------------------------------------------");
		for (i = i; i < 9; i++)
			printf("%s\n", page_contents[i]);
		printf("------------------------------------------------------------------------");
		printf("Exiting Options\n");
		printf("------------------------------------------------------------------------");
		for (i = i; i < ITEMS; i++)
			printf("%s\n", page_contents[i]);
		printf("------------------------------------------------------------------------");
		Console_SetPosition(26, 0);
		printf("Change Selection: [%s%s %s%s]\t\t\t\t\t\tSelect: [A]\tCredits: [1]", LEFT_ARROW, RIGHT_ARROW, UP_ARROW, DOWN_ARROW);
		buttons = wait_anyKey();
		gprintf("\n\nselected [%i] \n\n", selected);
		if (buttons & WPAD_BUTTON_DOWN)
			if (++selected > 9)
				selected = 0;

		if (buttons & WPAD_BUTTON_UP)
			if (--selected < 0)
				selected = 9;

		if (buttons & WPAD_BUTTON_LEFT)
		{
			switch (selected)
			{
			case 0:
				if (--lang < 0)
					lang = 6;
				break;
			case 1:
				if (--area < 0)
					area = 11;
				break;
			case 2:
				if (--game < 0)
					game = 3;
				break;
			case 3:
				if (--video < 0)
					video = 2;
				break;
			case 4:
				if (country == 1)
					country = 136;
				else if (country == 49)
					country = 1;
				else if (country == 110)
					country = 49;
				else if (country == 136)
					country = 110;
				if (eula >= 0)
					eula = 0;
				break;
			case 5:
				if (eula >= 0)
					eula = !eula;
				break;
			}
		}

		if (buttons & WPAD_BUTTON_RIGHT)
		{
			switch (selected)
			{
			case 0:
				if (++lang == 7)
					lang = 0;
				break;
			case 1:
				if (++area == 12)
					area = 0;
				break;
			case 2:
				if (++game == 4)
					game = 0;
				break;
			case 3:
				if (++video == 3)
					video = 0;
				break;
			case 4:
				if (country == 1)
					country = 49;
				else if (country == 49)
					country = 110;
				else if (country == 110)
					country = 136;
				else if (country == 136)
					country = 1;
				if (eula >= 0)
					eula = 0;
				break;
			case 5:
				if (eula >= 0)
					eula = !eula;
				break;
			}
		}

		if (buttons & WPAD_BUTTON_A)
		{
			switch (selected)
			{
			case 6:
				getSettings();
				break;
			case 7:
				saveSettings();
				break;
			case 8:
				if (sysmenu_region == 'J')
				{ // jpn
					lang = 0;
					area = 0;
					game = 0;
					video = 0;
					country = 1;
				}
				else if (sysmenu_region == 'U')
				{ // usa
					lang = 1;
					area = 1;
					game = 1;
					video = 0;
					country = 49;
				}
				else if (sysmenu_region == 'E')
				{ // EUR/PAL
					lang = 1;
					area = 2;
					game = 2;
					video = 1;
					country = 110;
				}
				else if (sysmenu_region == 'K')
				{ // KOR
					lang = 1;
					area = 7;
					game = 3;
					video = 0;
					country = 136;
				}
				else
				{
					printf("\nUnknown System Menu region '%c' (v%hu).\n", sysmenu_region ?: ' ', sysmenu_version);
					printf("Press any key to quit\n");
					wait_anyKey();
					exit(0);
				}
				selected--;
				// saveSettings();
				break;
			case 9:
				needbreak = true;
				break;
			case 10:
				STM_RebootSystem();
				break;
			}
		}

		if (buttons & WPAD_BUTTON_B)
		{
			selected = 9;
		}
		if (buttons & WPAD_BUTTON_1)
		{
			draw_credits();
			wait_anyKey();
		}
		updatePage();
		if (needbreak)
			break;
	}

	Console_SetPosition(26, 30);
	Console_SetColors(BLACK, 0, GREEN, 0);
	printf("Exiting");
	ISFS_Deinitialize();
	WPAD_Shutdown();

	return 0;
}

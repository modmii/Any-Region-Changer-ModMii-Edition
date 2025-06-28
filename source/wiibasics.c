/*-------------------------------------------------------------

wiibasics.c -- basic Wii initialization and functions

Copyright (C) 2008 tona
Unless other credit specified

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

i modified it a ton sorry tona
- thepikachugamer

3.This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#include <string.h>

#include "wiibasics.h"
#include "gecko.h"

#define MAX_WIIMOTES 4

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
int ConsoleRows;
int ConsoleCols;

/* Basic init taken pretty directly from the libOGC examples */
void videoInit(void)
{
	// Initialise the video system
	VIDEO_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// rmode->viWidth = 678;
	// rmode->viXOrigin = (VI_MAX_WIDTH_PAL - 678)/2;

	GX_AdjustForOverscan(rmode, rmode, 32, 24);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb, 0, 0, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	// printf("\x1b[2;0H");
	CON_GetMetrics(&ConsoleCols, &ConsoleRows);
}

void ClearScreen()
{
	/* Clear console */
	printf("\x1b[2J");
	fflush(stdout);
}
void ClearLine()
{
	printf("\r\x1b[2K\r");
	fflush(stdout);
}
void PrintCenter(char *text, int width)
{
	int textLen = strlen(text);
	int leftPad = (width - textLen) / 2;
	int rightPad = (width - textLen) - leftPad;
	printf("%*s%s%*s", leftPad, " ", text, rightPad, " ");
}
void Console_SetFgColor(u8 color, u8 bold)
{
	printf("\x1b[%u;%dm", color + 30, bold);
}
void Console_SetBgColor(u8 color, u8 bold)
{
	printf("\x1b[%u;%dm", color + 40, bold);
}
void Console_SetColors(u8 bgColor, u8 bgBold, u8 fgColor, u8 fgBold)
{
	Console_SetBgColor(bgColor, bgBold);
	Console_SetFgColor(fgColor, fgBold);
}
void Console_SetPosition(u8 row, u8 column)
{
	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[%u;%uH", row, column);
}
void PrintBanner()
{
	ClearScreen();
	Console_SetColors(GREEN, 0, WHITE, 2);
	char text[ConsoleCols];
	snprintf(text, sizeof(text), "Any Region Changer   ModMii Edition   %.1lf.%i            IOS: %i", ARCME_VERSION, ARCME_REV, IOS_GetVersion());
	PrintCenter(text, ConsoleCols);
	Console_SetColors(BLACK, 0, WHITE, 2);
}
/*
void miscInit(void)
{
	int ret;

	// This function initialises the attached controllers
	WPAD_Init();

	// NO
//	Identify_SU();

	gprintf("Initializing Filesystem driver...");
	fflush(stdout);

	ret = ISFS_Initialize();
	if (ret < 0)
	{
		gprintf("\nError! ISFS_Initialize (ret = %d)\n", ret);
	//	wait_anyKey();
		exit(0);
	}
	else
	{
		gprintf("OK!\n");
	}

	// IdentSysMenu();
}

void IdentSysMenu(void)
{
	int ret;
	Identify_SysMenu();

	ret = ES_SetUID(TITLE_ID(1, 2));
	if (ret < 0)
	{
		printf("SetUID fail %d", ret);
		wait_anyKey();
		exit(1);
	}

	printf("Initializing Filesystem driver...");
	fflush(stdout);

	ISFS_Deinitialize();
	ret = ISFS_Initialize();
	if (ret < 0)
	{
		printf("\nError! ISFS_Initialize (ret = %d)\n", ret);
		wait_anyKey();
		exit(1);
	}
	else
	{
		printf("OK!\n");
	}
}

void miscDeInit(void)
{
	fflush(stdout);
	ISFS_Deinitialize();
}
*/

u32 getButtons(void)
{
	WPAD_ScanPads();
	PAD_ScanPads();

	u32 buttons = WPAD_ButtonsDown(0);
	u32 buttons_gc = PAD_ButtonsDown(0);
	if (buttons_gc & PAD_BUTTON_A) buttons |= WPAD_BUTTON_A;
	if (buttons_gc & PAD_BUTTON_B) buttons |= WPAD_BUTTON_B;
	if (buttons_gc & PAD_BUTTON_X) buttons |= WPAD_BUTTON_1;
	if (buttons_gc & PAD_BUTTON_Y) buttons |= WPAD_BUTTON_2;
	if (buttons_gc & PAD_BUTTON_START) buttons |= WPAD_BUTTON_HOME;
	if (buttons_gc & PAD_BUTTON_UP) buttons |= WPAD_BUTTON_UP;
	if (buttons_gc & PAD_BUTTON_DOWN) buttons |= WPAD_BUTTON_DOWN;
	if (buttons_gc & PAD_BUTTON_LEFT) buttons |= WPAD_BUTTON_LEFT;
	if (buttons_gc & PAD_BUTTON_RIGHT) buttons |= WPAD_BUTTON_RIGHT;

	return buttons;
}

u32 wait_anyKey(void)
{
	u32 pressed;
	while (!(pressed = getButtons()))
	{
		VIDEO_WaitVSync();
	}
	if (pressed & WPAD_BUTTON_HOME)
	{
		Console_SetPosition(26, 0);
		ClearLine();
		Console_SetPosition(26, 30);
		Console_SetColors(BLACK, 0, GREEN, 0);
		printf("Exiting");
		exit(0);
	}
	return pressed;
}

u32 wait_key(u32 button)
{
	u32 pressed;
	do
	{
		pressed = wait_anyKey();
	} while (!(pressed & button));

	return pressed;
}

char charASCII(u8 c)
{
	if (c < 0x20 || c > 0x7E)
		return '.';
	else
		return (char)c;
}

void hex_print_array16(const u8 *array, u32 size)
{
	u32 offset = 0;
	u32 page_size = 0x100;
	char line[17];
	line[16] = 0;
	if (size > page_size)
		printf("Page 1 of %u", (size / page_size) + 1);
	while (offset < size)
	{
		if (!(offset % 16))
			printf("\n0x%08X: ", offset);

		printf("%02X", array[offset]);

		line[offset % 16] = charASCII(array[offset]);

		if (!(++offset % 2))
			printf(" ");

		if (!(offset % 16))
			printf(line);

		if (!(offset % page_size) && offset < size)
		{
			u32 pressed;
			printf("\n\tPress a key for next page or B for finish\n");
			pressed = wait_anyKey();
			if (pressed & WPAD_BUTTON_HOME)
				exit(1);
			else if (pressed & WPAD_BUTTON_B)
				return;
		}
	}
}

bool yes_or_no(void)
{
	bool yes = 0;
	u32 buttons = 0;

	do
	{
		yes = buttons & WPAD_BUTTON_LEFT;
		if (yes)
			printf("\r\x1b[K  <\x1b[30m\x1b[47;1m Yes \x1b[37;1m\x1b[40m>    No    ");
		else
			printf("\r\x1b[K    Yes    <\x1b[30m\x1b[47;1m No \x1b[37;1m\x1b[40m>   ");
	} while ((buttons = wait_key(WPAD_BUTTON_A | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT)) && (!(buttons & WPAD_BUTTON_A)));
	printf("\n");
	return yes;
}

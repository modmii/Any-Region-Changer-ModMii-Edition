/*-------------------------------------------------------------

detect_settings.c -- detects various system settings

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

Hi i made some changes
- thepikachugamer

3.This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#define _GNU_SOURCE /* for memmem */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gccore.h>
#include <sys/param.h>
#include "detect_settings.h"
#include "wiibasics.h"

/* (gdb) print (unsigned short)-1
 * $1 = 65535
 * This is a valid version number */
// u16 get_installed_title_version(u64 title)
s32 get_installed_title_version(u64 title)
{
	s32 ret, fd;
	static char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);

	// Check to see if title exists
	if (ES_GetDataDir(title, filepath) >= 0)
	{
		u32 tmd_size;
		static u8 tmd_buf[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);

		ret = ES_GetStoredTMDSize(title, &tmd_size);
		if (ret < 0)
		{
			// If we fail to use the ES function, try reading manually
			// This is a workaround added since some IOS (like 21) don't like our
			// call to ES_GetStoredTMDSize

			// printf("Error! ES_GetStoredTMDSize: %d\n", ret);

			sprintf(filepath, "/title/%08x/%08x/content/title.tmd", TITLE_UPPER(title), TITLE_LOWER(title));

			fd = ret = ISFS_Open(filepath, ISFS_OPEN_READ);
			if (ret < 0)
			{
				printf("Error! ISFS_Open (ret = %d)\n", ret);
				return ret;
			}

			ret = ISFS_Seek(fd, 0x1dc, 0);
			if (ret < 0)
			{
				printf("Error! ISFS_Seek (ret = %d)\n", ret);
				return ret;
			}

			ret = ISFS_Read(fd, tmd_buf, 2);
			ISFS_Close(fd);
			if (ret < 0)
			{
				printf("Error! ISFS_Read (ret = %d)\n", ret);
				return ret;
			}

			return *(u16*)tmd_buf;
		}
		else
		{
			// Normal versions of IOS won't have a problem, so we do things the "right" way.

			// Some of this code adapted from bushing's title_lister.c
			signed_blob *s_tmd = (signed_blob *)tmd_buf;
			ret = ES_GetStoredTMD(title, s_tmd, tmd_size);
			if (ret < 0)
			{
				printf("Error! ES_GetStoredTMD: %d\n", ret);
				// return -1;
				return ret;
			}
			tmd *t = SIGNATURE_PAYLOAD(s_tmd);
			return t->title_version;
		}
	}
	return -106;
}

u64 get_title_ios(u64 title)
{
	s32 ret, fd;
	static char filepath[256] ATTRIBUTE_ALIGN(32);

	// Check to see if title exists
	if (ES_GetDataDir(title, filepath) >= 0)
	{
		u32 tmd_size;
		static u8 tmd_buf[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);

		ret = ES_GetStoredTMDSize(title, &tmd_size);
		if (ret < 0)
		{
			// If we fail to use the ES function, try reading manually
			// This is a workaround added since some IOS (like 21) don't like our
			// call to ES_GetStoredTMDSize

			// printf("Error! ES_GetStoredTMDSize: %d\n", ret);

			sprintf(filepath, "/title/%08x/%08x/content/title.tmd", TITLE_UPPER(title), TITLE_LOWER(title));

			ret = ISFS_Open(filepath, ISFS_OPEN_READ);
			if (ret <= 0)
			{
				printf("Error! ISFS_Open (ret = %d)\n", ret);
				return 0;
			}

			fd = ret;

			ret = ISFS_Seek(fd, 0x184, 0);
			if (ret < 0)
			{
				printf("Error! ISFS_Seek (ret = %d)\n", ret);
				return 0;
			}

			ret = ISFS_Read(fd, tmd_buf, 8);
			if (ret < 0)
			{
				printf("Error! ISFS_Read (ret = %d)\n", ret);
				return 0;
			}

			ret = ISFS_Close(fd);
			if (ret < 0)
			{
				printf("Error! ISFS_Close (ret = %d)\n", ret);
				return 0;
			}

			// it's very nicely aligned i don't think the broadway will crash with this one
			return *(u64*)tmd_buf;
		}
		else
		{
			// Normal versions of IOS won't have a problem, so we do things the "right" way.

			// Some of this code adapted from bushing's title_lister.c
			signed_blob *s_tmd = (signed_blob *)tmd_buf;
			ret = ES_GetStoredTMD(title, s_tmd, tmd_size);
			if (ret < 0)
			{
				printf("Error! ES_GetStoredTMD: %d\n", ret);
				return -1;
			}
			tmd *t = SIGNATURE_PAYLOAD(s_tmd);
			return t->sys_version;
		}
	}
	return 0;
}

/* Get Sysmenu Region identifies the region of the system menu (not your Wii)
  by looking into it's resource content file for region information. */ // <-- Semibricked Wiis:
char get_sysmenu_region(void)
{
	s32 ret;
	u16 version;

	version = get_installed_title_version(TITLE_ID(1, 2));
	if (version <= 0)
		return 0;

	/* "mauifrog's 4.1 mod"(?) */
	if ((version / 1000) == 54)
		version %= 1000;

	switch (version & 0b0000000000011111)
	{
		case 0:
			return 'J';
		case 1:
			return 'U';
		case 2:
			return 'E';
		case 6:
			return 'K';

		default:
			printf("Infected system menu (version number is %hu)\n", version);
			printf("Press HOME to exit, any other button to try plan B.\n");
			wait_anyKey();
			break;
	}

	// Plan B
	tikview view ATTRIBUTE_ALIGN(0x20) = {};
	s32 cfd;
	char region = 0;
	const char search[] = "ipl\\bin\\RVL\\Final_";
	unsigned char* buffer = NULL;

	ret = ES_GetTicketViews(TITLE_ID(1, 2), &view, 1);
	if (ret < 0)
	{
		printf("Error! ES_GetTicketViews (ret = %i)\n", ret);
		wait_anyKey();
		return '?';
	}

	// .......right, this isn't a vWii with Priiloader installed lol
	cfd = ret = ES_OpenTitleContent(TITLE_ID(1, 2), &view, 1);
	if (ret < 0)
	{
		printf("Error! ES_OpenTitleContent (ret = %i)\n", ret);
		wait_anyKey();
		return '?';
	}

	size_t size = ret = ES_SeekContent(cfd, 0, SEEK_END);
	ES_SeekContent(cfd, 0, SEEK_SET);

	buffer = aligned_alloc(0x20, size);
	if (!buffer)
	{
		printf("Out of memory!\n");
		wait_anyKey();
		exit(1);
	}

	ret = ES_ReadContent(cfd, buffer, size);
	ES_CloseContent(cfd);
	if (ret < 0)
	{
		printf("Error! ES_ReadContent (ret = %i)\n", ret);
		wait_anyKey();
	}

	char* thestring = memmem(buffer, size, search, strlen(search));
	if (thestring)
	{
		printf("Found you!!! %s\n", thestring);
		region = thestring[strlen(thestring)];
	}

	free(buffer);
	if (!region)
	{
		printf("Unable to identify system menu region!!\n");
		sleep(2);
	}

	return region ? region : '?';

}

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

3.This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <gccore.h>

#include "id.h"
#include "wiibasics.h"
#include "detect_settings.h"

u16 get_installed_title_version(u64 title)
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

			ret = ISFS_Seek(fd, 0x1dc, 0);
			if (ret < 0)
			{
				printf("Error! ISFS_Seek (ret = %d)\n", ret);
				return 0;
			}

			ret = ISFS_Read(fd, tmd_buf, 2);
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

			return be16(tmd_buf);
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
			return t->title_version;
		}
	}
	return 0;
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

			return be64(tmd_buf);
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
  by looking into it's resource content file for region information. */
char get_sysmenu_region(void)
{
	s32 ret, cfd;
	static u8 fbuffer[0x500] ATTRIBUTE_ALIGN(32);
	static tikview viewdata[0x10] ATTRIBUTE_ALIGN(32);
	u32 views;
	static u64 tid ATTRIBUTE_ALIGN(32) = TITLE_ID(1, 2);
	u8 region, match[] = "FINAL";

	/*ret = ES_SetUID(TITLE_ID(1,2));
	if (ret){
		printf("Error! ES_GetSetUID %d\n", ret);
		wait_anyKey();
		return 0;
	}

	ret = ES_GetTitleID(&tid);
	if (ret){
		printf("Error! ES_GetTitleID %d\n", ret);
		wait_anyKey();
		return 0;
	}
	if (tid != TITLE_ID(1,2)){
		printf("Error! Not System Menu! %016llx\n", tid);
		wait_anyKey();
		return 0;
	}
	ret = ES_OpenContent(1);
	if (ret < 0)
	{
		printf("Error! ES_OpenContent (ret = %d)\n", ret);
		wait_anyKey();
		return 0;
	}*/

	ret = ES_GetNumTicketViews(tid, &views);
	if (ret < 0)
	{
		printf(" Error! ES_GetNumTickets (ret = %d)\n", ret);
		wait_anyKey();
		return ret;
	}

	if (!views)
	{
		printf(" No tickets found!\n");
		wait_anyKey();
		return 0;
	}
	else if (views > 16)
	{
		printf(" Too many ticket views! (views = %d)\n", views);
		wait_anyKey();
		return 0;
	}

	ret = ES_GetTicketViews(tid, viewdata, 1);
	if (ret < 0)
	{
		printf("Error! ES_OpenTitleContent (ret = %d)\n", ret);
		wait_anyKey();
		return 0;
	}

	ret = ES_OpenTitleContent(tid, viewdata, 1);
	if (ret < 0)
	{
		printf("Error! ES_OpenTitleContent (ret = %d)\n", ret);
		wait_anyKey();
		return 0;
	}

	cfd = ret;
	region = 0;
	while (!region)
	{
		int i;
		ret = ES_ReadContent(cfd, fbuffer, 0x500);
		if (ret < 0)
		{
			printf("Error! ES_ReadContent (ret = %d)\n", ret);
			wait_anyKey();
			return 0;
		}

		for (i = 0; i < 0x500; i++)
		{
			if (fbuffer[i] == 'F')
			{
				if (!memcmp(&fbuffer[i], match, 6))
				{
					region = fbuffer[i + 6];
					break;
				}
			}
		}
	}
	ret = ES_CloseContent(cfd);
	if (ret < 0)
	{
		printf("Error! ES_CloseContent (ret = %d)\n", ret);
		wait_anyKey();
		return 0;
	}

	switch (region)
	{
	case 'U':
	case 'E':
	case 'J':
	case 'K':
		return region;
		break;
	default:
		return -1;
		break;
	}
}
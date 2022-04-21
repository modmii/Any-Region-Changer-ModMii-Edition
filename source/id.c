/*-------------------------------------------------------------

id.c -- ES Identification code

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
#include <stdlib.h>
#include <string.h>
#include <gccore.h>

#include "wiibasics.h"
#include "id.h"
#include "gecko.h"
#include "sha1.h"
#include "certs_dat.h"

/* Debug functions adapted from libogc's es.c */
//#define DEBUG_ES
//#define DEBUG_IDENT
#define ISALIGNED(x) ((((u32)x) & 0x1F) == 0)

static u8 su_tmd[0x208] ATTRIBUTE_ALIGN(32);
static u8 su_tik[STD_SIGNED_TIK_SIZE] ATTRIBUTE_ALIGN(32);
int su_id_filled = 0;

#ifdef DEBUG_IDENT
s32 __sanity_check_certlist(const signed_blob *certs, u32 certsize)
{
	int count = 0;
	signed_blob *end;

	if (!certs || !certsize)
		return 0;

	end = (signed_blob *)(((u8 *)certs) + certsize);
	while (certs != end)
	{
#ifdef DEBUG_ES
		printf("Checking certificate at %p\n", certs);
#endif
		certs = ES_NextCert(certs);
		if (!certs)
			return 0;
		count++;
	}
#ifdef DEBUG_ES
	printf("Num of certificates: %d\n", count);
#endif
	return count;
}
#endif
void zero_sig(signed_blob *sig)
{
	u8 *sig_ptr = (u8 *)sig;
	memset(sig_ptr + 4, 0, SIGNATURE_SIZE(sig) - 4);
}

void brute_tmd(tmd *p_tmd)
{
	u16 fill;
	for (fill = 0; fill < 65535; fill++)
	{
		p_tmd->fill2 = fill;
		sha1 hash;
		//    debug_printf("SHA1(%p, %x, %p)\n", p_tmd, TMD_SIZE(p_tmd), hash);
		SHA1((u8 *)p_tmd, TMD_SIZE(p_tmd), hash);
		;

		if (hash[0] == 0)
		{
			//      debug_printf("setting fill3 to %04hx\n", fill);
			return;
		}
	}
	printf("Unable to fix tmd :(\n");
	exit(4);
}

void brute_tik(tik *p_tik)
{
	u16 fill;
	for (fill = 0; fill < 65535; fill++)
	{
		p_tik->padding = fill;
		sha1 hash;
		//    debug_printf("SHA1(%p, %x, %p)\n", p_tmd, TMD_SIZE(p_tmd), hash);
		SHA1((u8 *)p_tik, sizeof(tik), hash);

		if (hash[0] == 0)
			return;
	}
	printf("Unable to fix tik :(\n");
	exit(5);
}

void forge_tmd(signed_blob *s_tmd)
{
	//  debug_printf("forging tmd sig");
	zero_sig(s_tmd);
	brute_tmd(SIGNATURE_PAYLOAD(s_tmd));
}

void forge_tik(signed_blob *s_tik)
{
	//  debug_printf("forging tik sig");
	zero_sig(s_tik);
	brute_tik(SIGNATURE_PAYLOAD(s_tik));
}

void Make_SUID(void)
{
	signed_blob *s_tmd, *s_tik;
	tmd *p_tmd;
	tik *p_tik;

	memset(su_tmd, 0, sizeof su_tmd);
	memset(su_tik, 0, sizeof su_tik);
	s_tmd = (signed_blob *)&su_tmd[0];
	s_tik = (signed_blob *)&su_tik[0];
	*s_tmd = *s_tik = 0x10001;
	p_tmd = (tmd *)SIGNATURE_PAYLOAD(s_tmd);
	p_tik = (tik *)SIGNATURE_PAYLOAD(s_tik);

	strcpy(p_tmd->issuer, "Root-CA00000001-CP00000004");
	p_tmd->title_id = TITLE_ID(1, 2);

	p_tmd->num_contents = 1;

	forge_tmd(s_tmd);

	strcpy(p_tik->issuer, "Root-CA00000001-XS00000003");
	p_tik->ticketid = 0x000038A45236EE5FLL;
	p_tik->titleid = TITLE_ID(1, 2);

	memset(p_tik->cidx_mask, 0xFF, 0x20);
	forge_tik(s_tik);

	su_id_filled = 1;
}

s32 Identify(const u8 *certs, u32 certs_size, const u8 *idtmd, u32 idtmd_size, const u8 *idticket, u32 idticket_size)
{
	s32 ret;
	u32 keyid = 0;
	ret = ES_Identify((signed_blob *)certs, certs_size, (signed_blob *)idtmd, idtmd_size, (signed_blob *)idticket, idticket_size, &keyid);
	if (ret < 0)
	{
		switch (ret)
		{
		case ES_EINVAL:
			printf("Error! ES_Identify (ret = %d;) Data invalid!\n", ret);
			break;
		case ES_EALIGN:
			printf("Error! ES_Identify (ret = %d;) Data not aligned!\n", ret);
			break;
		case ES_ENOTINIT:
			printf("Error! ES_Identify (ret = %d;) ES not initialized!\n", ret);
			break;
		case ES_ENOMEM:
			printf("Error! ES_Identify (ret = %d;) No memory!\n", ret);
			break;
		default:
			printf("Error! ES_Identify (ret = %d)\n", ret);
			break;
		}
#ifdef DEBUG_IDENT
		printf("\tTicket: %u Std: %u Max: %u\n", idticket_size, STD_SIGNED_TIK_SIZE, MAX_SIGNED_TMD_SIZE);
		printf("\tTMD invalid? %d %d %d Tik invalid? %d %d\n", !(signed_blob *)idtmd, !idtmd_size, !IS_VALID_SIGNATURE((signed_blob *)idtmd), !(signed_blob *)idticket, !IS_VALID_SIGNATURE((signed_blob *)idticket));
		printf("\tCerts: Sane? %d\n", __sanity_check_certlist((signed_blob *)certs, certs_size));
		if (!ISALIGNED(certs))
			printf("\tCertificate data is not aligned!\n");
		if (!ISALIGNED(idtmd))
			printf("\tTMD data is not aligned!\n");
		if (!ISALIGNED(idticket))
			printf("\tTicket data is not aligned!\n");
#endif
	}
	else
		printf("OK!\n");
	return ret;
}

s32 Identify_SU(void)
{
	if (!su_id_filled)
		Make_SUID();

	gprintf("\nIdentifying as SU...");
	fflush(stdout);
	return Identify(certs_dat, certs_dat_size, su_tmd, sizeof su_tmd, su_tik, sizeof su_tik);
}

s32 Identify_SysMenu(void)
{
	s32 ret;
	u32 sysmenu_tmd_size, sysmenu_ticket_size;
	// static u8 certs[0xA00] ATTRIBUTE_ALIGN(32);
	static u8 sysmenu_tmd[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);
	static u8 sysmenu_ticket[STD_SIGNED_TIK_SIZE] ATTRIBUTE_ALIGN(32);

	/*printf("\nPulling Certs...");
	ret = ISFS_ReadFileToArray ("/sys/certs.sys", certs, 0xA00, &certs_size);
	if (ret < 0) {
		printf("\tReading Certs failed!\n");
		return -1;
	}*/

	printf("\nPulling Sysmenu TMD...");
	ret = ISFS_ReadFileToArray("/title/00000001/00000002/content/title.tmd", sysmenu_tmd, MAX_SIGNED_TMD_SIZE, &sysmenu_tmd_size);
	if (ret < 0)
	{
		printf("\tReading TMD failed!\n");
		return -1;
	}

	printf("\nPulling Sysmenu Ticket...");
	ret = ISFS_ReadFileToArray("/ticket/00000001/00000002.tik", sysmenu_ticket, STD_SIGNED_TIK_SIZE, &sysmenu_ticket_size);
	if (ret < 0)
	{
		printf("\tReading TMD failed!\n");
		return -1;
	}

	printf("\nIdentifying as SysMenu...");
	fflush(stdout);
	return Identify(certs_dat, certs_dat_size, sysmenu_tmd, sysmenu_tmd_size, sysmenu_ticket, sysmenu_ticket_size);
}

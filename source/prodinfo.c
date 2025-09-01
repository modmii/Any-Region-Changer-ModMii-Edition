/* (c) 2025 thepikachugamer */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <ogc/isfs.h>
#include <ogc/es.h>

#include "prodinfo.h"

#define SETTING_TXT_PATH "/title/00000001/00000002/data/setting.txt"
#define SETTING_TXT_SIZE 0x100

static int InfoReady = 0;
static int BufferDirty = 0;
static char SettingBuffer[SETTING_TXT_SIZE] [[gnu::aligned(0x20)]]; // ISFS demands.
static const uint32_t key_unrolled[32 / 4] = { 0xfaf4e9d3, 0xa74e9c39, 0x73e7ce9d, 0x3b76edda, 0xb56bd7ae, 0x5dbb76ed, 0xdbb76fdf, 0xbf7ffefd };

static void ProductInfo_DecryptBuffer(const char* in, char* out)
{
	const uint32_t* in32 = (const uint32_t *)in;
	uint32_t* out32 = (uint32_t *)out;

	for (int i = 0; i < (32 / 4); i++) {
		for (int j = i; j < (SETTING_TXT_SIZE / 4); j += (32 / 4))
			out32[j] = in32[j] ^ key_unrolled[i];
	}
}

int ProductInfo_Init(void) {
	int ret, ret2, fd;

	if (InfoReady)
		return 0;

	ret = fd = ISFS_Open(SETTING_TXT_PATH, ISFS_OPEN_READ);
	if (ret < 0)
		return ret;

	ret = ISFS_Read(fd, SettingBuffer, SETTING_TXT_SIZE);
	ret2 = ISFS_Close(fd);
	if (ret != SETTING_TXT_SIZE)
		return ret2 ?: -1;

	ProductInfo_DecryptBuffer(SettingBuffer, SettingBuffer);

	InfoReady = 1;
	return 0;
}

// The way official software does this is, interesting. The file is kept encrypted in memory (ofc) and then they use a state-machine-whatever to step through the bytes of the buffer. We don't have to do allat though! Or, we won't.
// I will note that it seems to like
// <https://github.com/koopthekoopa/wii-ipl/blob/main/libs/RVL_SDK/src/sc/scapi_prdinfo.c#L77C1-L77C57>
//             if (((ptext ^ type[typeOfs]) & 0xDF) == 0) {
// have case insensitivity? interesting. (0xDF is ~0x20)
int ProductInfo_Find(int len; const char* item, char out[len], int len) {
	if (!InfoReady || !item || !out || !len)
		return 0;

	for (int i = 0; i < SETTING_TXT_SIZE && SettingBuffer[i] != '\0'; ) {
		const char* ptr = SettingBuffer + i;


		int llen = strcspn(ptr, "\r\n");
		int nlen = strcspn(ptr, "=");
		int vlen = llen - (nlen + 1);
		if (nlen >= llen)
			break;

		if (nlen == strlen(item) && memcmp(ptr, item, nlen) == 0) {
			const char* value = ptr + nlen + 1;

			if (vlen >= len) {
				printf("Item %s is too large (=%.*s)\n", item, vlen, value);
				return 0;
			}

			memcpy(out, value, vlen);
			out[vlen] = '\0';
			return vlen;
		}

		i += llen + strspn(ptr + llen, "\r\n");
	}

	printf("Could not find item %s\n", item);
	return 0;
}

int ProductInfo_Set(const char* item, const char* val) {
	if (!InfoReady)
		return -1;

	char tmpbuffer[SETTING_TXT_SIZE];
	int i, x;
	bool found = false;

	// Copy each setting one by one to the temporary buffer; if we find the setting we want to change, change it
	for (i = 0, x = 0; i < SETTING_TXT_SIZE && SettingBuffer[i] != '\0'; ) {
		const char* ptr = SettingBuffer + i;

		int llen = strcspn(ptr, "\r\n");
		int nlen = strcspn(ptr, "=");
		if (nlen >= llen)
			break;

		if (nlen == strlen(item) && memcmp(ptr, item, nlen) == 0) {
			// Found! Replace it.
			x += snprintf(tmpbuffer + x, SETTING_TXT_SIZE - x, "%.*s=%s\r\n", nlen, ptr, val);
			found = true;
		}
		else {
			// Nah.
			x += snprintf(tmpbuffer + x, SETTING_TXT_SIZE - x, "%.*s\r\n", llen, ptr);
		}

		i += llen + strspn(ptr + llen, "\r\n");
	}

	if (!found) {
		printf("Could not find item %s\n", item);
		return -2;
	}

	if (x == SETTING_TXT_SIZE) {
		printf("Buffer overflow...?\n");
		return -3;
	}

	memcpy(SettingBuffer, tmpbuffer, x);
	memset(SettingBuffer + x, '\0', SETTING_TXT_SIZE - x);
	BufferDirty++;
	return 0;
}

int ProductInfo_Save(void) {
	int fd, ret, ret2;

	if (!InfoReady)
		return -1;

	if (!BufferDirty)
		return 0;

	printf("%s", SettingBuffer);

	char encrypted[SETTING_TXT_SIZE] [[gnu::aligned(0x20)]];
	ProductInfo_DecryptBuffer(SettingBuffer, encrypted);

	// Get ready!
	printf("Change count=%i\n", BufferDirty);
	ret = fd = ISFS_Open(SETTING_TXT_PATH, ISFS_OPEN_WRITE);
	if (ret < 0) {
		return ret;
	}

	ret = ISFS_Write(fd, encrypted, SETTING_TXT_SIZE);
	ret2 = ISFS_Close(fd);

	if (ret != SETTING_TXT_SIZE)
		return ret2 ?: -1;

	BufferDirty = 0;
	return 0;
}

#if 0
char* ProductInfo_GetSerial(char serial[24]) {
	char code[4], serno[12];

	if (!serial)
		return NULL;

	if (!InfoReady)
		return strcpy(serial, "Info not ready!");

	if (!ProductInfo_Find("CODE", code, sizeof code) || !ProductInfo_Find("SERNO", serno, sizeof serno))
		return strcpy(serial, "UNKNOWN");

	snprintf(serial, 24, "%s%s", code, serno);

	/* https://3dbrew.org/wiki/Serials */
	int i, check = 0;
	for (i = 0; i < 8; i++) {
		unsigned digit = serno[i] - '0';
		if (digit >= 10) {
			errorf("Invalid serial 'number' %s\n", serial);
			break;
		}

		if (i & 1) // odd position
			digit += (digit << 1); // * 3

		check += digit;
	}

	if (i == 8) {
		check = (10 - (check % 10)) % 10;
		if (serno[i] - '0' != check)
			errorf("Invalid serial number %s\n", serial);
	}


	return serial;
}

#define CASE_CONSOLE_MODEL(model, len, v, name) \
	if (memcmp(model, v, strlen(v)) == 0) \
		return name;

static bool ProductInfo_IsRVA(void) {
	int fd = ISFS_Open("/title/00000001/00000002/data/RVA.txt", 0);
	ISFS_Close(fd);

	return (fd >= 0);
}

const char* ProductInfo_GetConsoleType(char model[16]) {
	char _model[16] = {};

	if (!model)
		model = _model;

	if (!InfoReady) {
		strcpy(model, "Info not ready!");
		return "Info not ready!";
	}

	int len = ProductInfo_Find("MODEL", model, 16); // pointer, cannot use sizeof. [16] is really just a hint
	if (!len) {
		strcpy(model, "UNKNOWN");
		return "UNKNOWN";
	}

	// Status update!! Wii System Transfer checks the device ID. I was right lol. Not too invasive to put it in here.
	{
		uint32_t device_id = 0;
		if (ES_GetDeviceID(&device_id) == 0 && (device_id >> 28) == 0x2)
			return "vWii (Wii U)";
	}
	CASE_CONSOLE_MODEL(model, ret, "RVL-001", "Wii");
	CASE_CONSOLE_MODEL(model, ret, "RVL-101", "Wii Family Edition");
	CASE_CONSOLE_MODEL(model, ret, "RVL-201", "Wii Mini");
	CASE_CONSOLE_MODEL(model, ret, "RVT", ProductInfo_IsRVA() ? "Revolution Arcade(?!)" : "NDEV 1.x(?)");
	CASE_CONSOLE_MODEL(model, ret, "RVD", "NDEV 2.x");

	return "UNKNOWN";
}
#endif

#define AREA_MAP(name) [CONF_AREA_##name] = #name
const char AreaMap[][4] = {
	AREA_MAP(JPN),
	AREA_MAP(USA),
	AREA_MAP(EUR),
	AREA_MAP(AUS),
	AREA_MAP(BRA),
	AREA_MAP(TWN),
	AREA_MAP(ROC),
	AREA_MAP(KOR),
	AREA_MAP(HKG),
	AREA_MAP(ASI),
	AREA_MAP(LTN),
	AREA_MAP(SAF),
	AREA_MAP(CHN),
};
#undef AREA_MAP

int ProductInfo_GetArea(void) {
	char val[4];

	int ret = ProductInfo_Find("AREA", val, sizeof val);
	if (ret <= 0)
		return ret ?: -1;

	for (int i = 0; i < (sizeof AreaMap / sizeof AreaMap[0]); i++) {
		if (strcmp(val, AreaMap[i]) == 0)
			return i;
	}

	printf("Unknown AREA %s\n", val);
	return -1;
}

int ProductInfo_SetArea(int area) {
	if (area < 0 || area >= (sizeof AreaMap / sizeof AreaMap[0]) || AreaMap[area][0] == '\0') {
		printf("Invalid AREA %i\n", area);
		return -1;
	}

	return ProductInfo_Set("AREA", AreaMap[area]);
}

#define REGION_MAP(name) [CONF_REGION_##name] = #name
const char GameRegionMap[][4] = {
	REGION_MAP(JP),
	REGION_MAP(US),
	REGION_MAP(EU),
	REGION_MAP(KR),
	REGION_MAP(CN),
};
#undef REGION_MAP

int ProductInfo_GetGameRegion(void) {
	char val[3];

	int ret = ProductInfo_Find("GAME", val, sizeof val);
	if (ret <= 0)
		return ret ?: -1;

	for (int i = 0; i < (sizeof GameRegionMap / sizeof GameRegionMap[0]); i++) {
		if (strcmp(val, GameRegionMap[i]) == 0)
			return i;
	}

	printf("Unknown GAME %s\n", val);
	return -1;
}

int ProductInfo_SetGameRegion(int game) {
	if (game < 0 || game >= (sizeof GameRegionMap / sizeof GameRegionMap[0]) || GameRegionMap[game][0] == '\0') {
		printf("Invalid GAME %i\n", game);
		return -1;
	}

	return ProductInfo_Set("GAME", GameRegionMap[game]);
}

#define VIDEO_MAP(name) [CONF_VIDEO_##name] = #name
const char VideoRegionMap[][6] = {
	VIDEO_MAP(NTSC),
	VIDEO_MAP(PAL),
	VIDEO_MAP(MPAL),
};
#undef VIDEO_MAP

int ProductInfo_GetVideoRegion(void) {
	char val[6];

	int ret = ProductInfo_Find("VIDEO", val, sizeof val);
	if (ret <= 0)
		return ret ?: -1;

	for (int i = 0; i < (sizeof VideoRegionMap / sizeof VideoRegionMap[0]); i++) {
		if (strcmp(val, VideoRegionMap[i]) == 0)
			return i;
	}

	printf("Unknown VIDEO %s\n", val);
	return -1;
}

int ProductInfo_SetVideoRegion(int video) {
	if (video < 0 || video >= (sizeof VideoRegionMap / sizeof VideoRegionMap[0]) || VideoRegionMap[video][0] == '\0') {
		printf("Invalid VIDEO %i\n", video);
		return -1;
	}

	return ProductInfo_Set("VIDEO", VideoRegionMap[video]);
}


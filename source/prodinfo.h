#pragma once
#include <ogc/conf.h>

int ProductInfo_Init(void);
int ProductInfo_Find(int len; const char* item, char out[len], int len);
int ProductInfo_Set(const char* item, const char* val);
int ProductInfo_Save(void);

extern const char AreaMap[][4];
int ProductInfo_GetArea(void);
int ProductInfo_SetArea(int area);

extern const char GameRegionMap[][4];
int ProductInfo_GetGameRegion(void);
int ProductInfo_SetGameRegion(int game);

extern const char VideoRegionMap[][6];
int ProductInfo_GetVideoRegion(void);
int ProductInfo_SetVideoRegion(int video);


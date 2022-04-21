
#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C"
{
#endif

    char ascii2(char s);

//#ifndef NO_DEBUG
    //use this just like printf();
    void gprintf(const char *str, ...);
    bool InitGecko();
    void hexdump2(void *d, int len);
    void USBGeckoOutput();
    void gecko_log(const char *str, ...);
//#else
//#define gprintf(...)
//#define InitGecko()      false
//#define hexdump( x, y )
//#endif /* NO_DEBUG */

#ifdef __cplusplus
}
#endif

#endif


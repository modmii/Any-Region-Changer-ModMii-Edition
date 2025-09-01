/*-------------------------------------------------------------

detect_settings.h -- detects various system settings

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

#ifndef __SYSMENU_DETECT_H_
#define __SYSMENU_DETECT_H_

// Get the title version of a given title
s32 get_installed_title_version(u64 title);

// Get the IOS version of a given title
u64 get_title_ios(u64 title);

// Get the region that the System menu is currently using
char get_sysmenu_region(void);

#endif

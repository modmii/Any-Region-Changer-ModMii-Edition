<img src="https://user-images.githubusercontent.com/32500606/168495918-67389d97-3ec8-4cdc-9173-7cfc95448042.png" width=25% height=25% alt="ARCME Icon">

# Any Region Changer: Modmii Edition

Any Region Changer: ModMii Edition (Otherwise known as ARCME) allows you to edit any and all of the region information on your Wii.  
The setting changes are non-temporary, i.e. they persist after system reboot.

ARCME is a mod of Any Region Changer (ARC). If ARCME detects a region mismatch, you only have to select the "Auto Fix Settings" to correct it. Older versions of ARC required you to set a Region, save, then change the video mode back to it's previous setting, and save again. ARCME's "Auto Fix Settings" takes care of all that (i.e. it does not change the video mode but changes everything else to align with the installed System Menu region). It's a minor improvement over the original but don't forget it also added support for Motion+ Wii remotes!

 - ARCME is one tool used by ModMii in order to facilitate a Wii Region Change. ARCME by itself is not enough to complete the region change procedure. To softmod or region change your Wii use [ModMii](https://modmii.github.io).

## Compiling
You need to install [devKitPro](https://devkitpro.org/wiki/Getting_Started) with the `wii-dev` group installed, as well as [RuntimeIOSPatch](https://gbatemp.net/threads/wii-vwii-libruntimeiospatch.339606) ([Source](https://gitlab.com/Nanolx/homebrewfilter/-/tree/master/libruntimeiospatch)) properly setup.  
If you are using Windows, it is recommended to use [WSL2](https://docs.microsoft.com/en-us/windows/wsl/about) if avalible.
Once all required tools are installed, run `make` in the same folder as  `Makefile`.

## Disclaimer
This software comes with NO WARRANTY WHATSOEVER. You may use this software at your own risk.  
Neither ModMii nor any source code contributors can take responsibility for any damage caused by this application.  
  
## Warnings
 - As prerequisites you should have the Homebrew Channel, Priiloader, BootMii and a NAND Backup so you can revert any changes pre or post reboot if needed.
 - If `setting.txt` is corrupted for any reason, it is entirely possible that your Wii will no longer boot. 
 - If you change the `GAME` ("Game Region") setting, you may no longer be able to boot discs!
 - Be careful not to change Video mode to a mode unsupported by your display.
 - Setting the `AREA` setting ("Console Area Setting") to a different region than your System Menu WILL cause a Semi-Brick! If for some reason your System-Configured flag is unset (via some installation, or a corrupted SYSCONF) your system WILL become FULLY BRICKED. 

## Issues
Any issues encountered while running ARCME can be reported on Github via [the issues tab](https://github.com/modmii/Any-Region-Changer-ModMii-Edition/issues) on the ARCME repo, or you can follow instructions on [GBATemp](https://gbatemp.net/threads/best-way-to-mod-any-wii-modmii-for-windows-official-support-thread.207126/) for ModMii issues in general.

## License
ARCME is licensed under the GPLv2 in line with ARC and Patchmii's licensing.

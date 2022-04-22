# Any Region Changer: Modmii Edition

Any Region Changer: ModMii Edition (Otherwise known as ARCME) allows you to edit any and all of the region information on your Wii.  
The setting changes are non-temporary, i.e. they persist after system reboot.

You should have the Homebrew Channel installed so you can revert any changes pre or post reboot if needed.

## Compiling
You need to install [devKitPro](https://devkitpro.org/wiki/Getting_Started) with the `wii-dev` group installed, as well as [RuntimeIOSPatch](https://gbatemp.net/threads/wii-vwii-libruntimeiospatch.339606/?msclkid=9449fb2ac11111eca5087aab4842d005) properly setup.  
If you are using Windows, it is recommended to use [WSL2](https://docs.microsoft.com/en-us/windows/wsl/about) if avalible.
Once all required tools are installed, run `make` in the same folder as  `Makefile`.

## Disclaimer
This software comes with NO WARRANTY WHATSOEVER. You may use this software at your own risk.  
Neither ModMii nor any source code contributors can take responsibility for any damage caused by this application.  
  
## Warnings
 - If setting.txt is corrupted for any reason, it is entirely possible that your Wii will no longer boot. 
 - If you change the GAME ("Game Region") setting, you may no longer be able to boot discs!
 - Be careful not to change Video mode to a mode unsupported by your display.
 - If you change your Country code, you will be unable to receive any pending gifts you have in the Wii Shop Channel. (The shop will also warn you of this)
 - Setting the AREA setting ("Console Area Setting") to a different region than your System Menu WILL cause a Semi-Brick! If for some reason your System-Configured flag is unset (via some installation, or a corrupted SYSCONF) your system WILL become FULLY BRICKED. 

## License
ARCME is licensed under the GPLv2 in line with ARC and Patchmii's licensing.
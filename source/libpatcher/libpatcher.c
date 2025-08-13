#include <gccore.h>
#include <ogc/machine/processor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "patches.h"

#define HW_AHBPROT 0x0d800064
#define MEM2_PROT 0x0d8b420a

#define AHBPROT_DISABLED read32(HW_AHBPROT) == 0xFFFFFFFF
#define IOS_MEMORY_START (u32 *)0x933E0000
#define IOS_MEMORY_END (u32 *)0x93FFFFFF

uint8_t in_dolphin = 0xFF;

// Within Dolphin, we have no IOS to patch.
// Additionally, many patches can cause Dolphin to fail.
bool is_dolphin() {
    // We may have detected this before.
    if (in_dolphin != 0xFF) {
        return (in_dolphin == 1);
    }

    s32 fd = IOS_Open("/dev/dolphin", IPC_OPEN_READ);

    // On a real Wii, this returns a negative number as an error.
    // Within Dolphin, we acquire a file descriptor starting at 0 or above.
    // We should close the handle if this is the case.
    if (fd >= 0) {
        IOS_Close(fd);
        in_dolphin = 1;
        return true;
    } else {
        in_dolphin = 0;
        return false;
    }
}

void disable_memory_protections() { write16(MEM2_PROT, 2); }

bool patch_memory_range(u32 *start, u32 *end, const u16 original_patch[],
                        const u16 new_patch[], u32 patch_size) {
    bool patched = false;

    for (u32 *patchme = start; patchme < end; ++patchme) {
        if (memcmp(patchme, original_patch, patch_size) == 0) {
            // Copy our new patch over the existing, and flush.
            memcpy(patchme, new_patch, patch_size);
            DCFlushRange(patchme, patch_size);

            // While this realistically won't do anything for some parts,
            // it's worth a try...
            ICInvalidateRange(patchme, patch_size);

            patched = true;
        }
    }

    return patched;
}

bool patch_ios_range(const u16 original_patch[], const u16 new_patch[],
                     u32 patch_size) {
    // Consider our changes successful under Dolphin.
    if (is_dolphin()) {
        return true;
    }

    return patch_memory_range(IOS_MEMORY_START, IOS_MEMORY_END, original_patch,
                              new_patch, patch_size);
}

bool patch_ahbprot_reset_for_ver(s32 ios_version) {
    // Under Dolphin, we do not need to disable AHBPROT.
    if (is_dolphin()) {
        return true;
    }

    // We'll need to disable MEM2 protections in order to write over IOS.
    disable_memory_protections();

    // Attempt to patch IOS.
    bool patched = patch_ios_range(ticket_check_old, ticket_check_patch,
                                   TICKET_CHECK_SIZE);
    if (!patched) {
        printf("unable to find and patch ES memory!\n");
        return false;
    }

    s32 ios_result = IOS_ReloadIOS(ios_version);
    if (ios_result < 0) {
        printf("unable to reload IOS version! (error %d)\n", ios_result);
        return false;
    }

    // Keep memory protections disabled.
    disable_memory_protections();

    if (AHBPROT_DISABLED) {
        return true;
    } else {
        printf("unable to preserve AHBPROT after IOS reload!\n");
        return false;
    }
}

bool patch_ahbprot_reset() {
    s32 current_ios = IOS_GetVersion();
    if (current_ios < 0) {
        printf("unable to get current IOS version! (error %d)\n", current_ios);
        return false;
    }

    return patch_ahbprot_reset_for_ver(current_ios);
}

bool patch_isfs_permissions() {
    return patch_ios_range(isfs_permissions_old, isfs_permissions_patch,
                           ISFS_PERMISSIONS_SIZE);
}

bool patch_es_identify() {
    return patch_ios_range(es_identify_old, es_identify_patch,
                           ES_IDENTIFY_SIZE);
}

bool patch_ios_verify() {
    return patch_ios_range(ios_verify_old, ios_verify_patch, IOS_VERIFY_SIZE);
}

bool apply_patches() {
    bool ahbprot_fix = patch_ahbprot_reset();
    if (!ahbprot_fix) {
        // patch_ahbprot_reset should log its own errors.
        return false;
    }

    if (!patch_isfs_permissions()) {
        printf("unable to find and patch ISFS permissions!\n");
        return false;
    }

    if (!patch_es_identify()) {
        printf("unable to find and patch ES_Identify!\n");
        return false;
    }

    if (!patch_ios_verify()) {
        printf("unable to find and patch IOSC_VerifyPublicKeySign!\n");
        return false;
    }

    return true;
}

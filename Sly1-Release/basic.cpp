#include "basic.h"
#include "gc.h"
#include <cstdint>
#include <new>

void GetBasicCid(BASIC* pbasic, CID* pcid)
{
	*pcid = pbasic->pvtbasic->cid;
}

bool FIsBasicDerivedFrom(const BASIC* basic, CID cid)
{
    if (!basic) return false;

    VTBASIC *pvt = basic->pvtbasic;

    while (pvt) {
        if (pvt->cid == cid) 
            return true;

        pvt = (VTBASIC*)pvt->pvtSuper;
    }
    return false;
}

void EnsureBasicSidebag(BASIC* pbasic)
{
    if (pbasic == nullptr)
        return;

    CSidebag* const psidebag = pbasic->psidebag.get();
    const std::uintptr_t sidebagAddress = reinterpret_cast<std::uintptr_t>(psidebag);
#if defined(__APPLE__)
    // libc++'s make_shared (PsidebagNew) puts the object 8 bytes past a
    // 16-byte boundary, so a valid sidebag is only aligned to CSidebag itself.
    const bool fCorrupt = psidebag != nullptr &&
        (sidebagAddress < 0x10000u || (sidebagAddress & (alignof(CSidebag) - 1u)) != 0u);
#else
    const bool fCorrupt = psidebag != nullptr &&
        (sidebagAddress < 0x10000u || (sidebagAddress & 0x0fu) != 0u);
#endif

    if (fCorrupt)
    {
        // Do not assign/reset a corrupted shared_ptr: that would dereference
        // its equally corrupted control block. Reconstruct it in place.
        std::construct_at(&pbasic->psidebag, PsidebagNew());
    }
    else if (psidebag == nullptr)
    {
        pbasic->psidebag = PsidebagNew();
    }

    if (pbasic->psidebag != nullptr)
        g_gc.AddRootSidebag(pbasic->psidebag.get());
}

// The sweep's own build of Sly1-Release/basic.cpp and sidebag.cpp, with two
// functions wrapped (test-only; ProjectCane links the unchanged objects and
// the sweep leaves those two objects out, see CMakeLists.txt):
//
// - PsidebagNew records every sidebag the game allocates, so the probe knows
//   which pointers are valid.
// - EnsureBasicSidebag runs the game's function and reports when it replaced
//   a valid sidebag: the object's custom options (AddO/EnsureO/SetO) are gone.
//
// The game's code is compiled exactly as written; only the two definitions
// get other names here, and every other caller (lo.cpp, spliceobj.cpp) links
// to the wrappers.

#include "sweep_sidebag.h"

#include "../../Sly1-Release/basic.h"
#include "../../Sly1-Release/gc.h"
#include "../../Sly1-Release/lo.h"
#include "../../Sly1-Release/sidebag.h"

#include <cstdio>
#include <iterator>
#include <map>
#include <memory>

#define PsidebagNew GamePsidebagNew
#include "../../Sly1-Release/sidebag.cpp"
#undef PsidebagNew

#define EnsureBasicSidebag GameEnsureBasicSidebag
#include "../../Sly1-Release/basic.cpp"
#undef EnsureBasicSidebag

namespace
{
// Every sidebag PsidebagNew returned, by address. The weak pointers keep the
// make_shared blocks (and so the addresses) from being reused while listed.
std::map<const CSidebag*, std::weak_ptr<CSidebag>> s_allocated;
std::vector<sweep_sidebag::Rebuild> s_rebuilds;
int s_allocations = 0;

bool Allocated(const CSidebag* psidebag)
{
    const auto it = s_allocated.find(psidebag);
    return it != s_allocated.end() && !it->second.expired();
}
}

std::shared_ptr<CSidebag> PsidebagNew()
{
    std::shared_ptr<CSidebag> psidebag = GamePsidebagNew();
    // Drop expired entries now and then so the map stays small.
    if (++s_allocations % 4096 == 0)
        for (auto it = s_allocated.begin(); it != s_allocated.end();)
            it = it->second.expired() ? s_allocated.erase(it) : std::next(it);
    s_allocated[psidebag.get()] = psidebag;
    return psidebag;
}

void EnsureBasicSidebag(BASIC* pbasic)
{
    CSidebag* const before = pbasic != nullptr ? pbasic->psidebag.get() : nullptr;
    const bool valid = before != nullptr && Allocated(before);
    std::vector<int> options;
    if (valid)
        for (int i = 0; i < before->m_csbb && i < static_cast<int>(std::size(before->m_asbb)); ++i)
            options.push_back(before->m_asbb[i].n);

    GameEnsureBasicSidebag(pbasic);

    if (!valid || pbasic->psidebag.get() == before)
        return;
    sweep_sidebag::Rebuild rebuild;
    if (pbasic->pvtbasic != nullptr)
    {
        rebuild.cid = static_cast<int>(pbasic->pvtbasic->cid);
        if (FIsBasicDerivedFrom(pbasic, CID_LO))
        {
            const LO* plo = reinterpret_cast<const LO*>(pbasic);
            rebuild.oid = static_cast<int>(plo->oid);
            rebuild.name = plo->pchzName != nullptr ? plo->pchzName : "";
        }
    }
    rebuild.sidebag = reinterpret_cast<uintptr_t>(before);
    rebuild.lostOptions = options;
    std::string lost;
    for (int option : options)
        lost += (lost.empty() ? "" : " ") + std::to_string(option);
    std::fprintf(stderr, "[sidebag-probe] valid sidebag %p of oid %d cid %d '%s' replaced; options lost: [%s]\n",
        static_cast<void*>(before), rebuild.oid, rebuild.cid, rebuild.name.c_str(), lost.c_str());
    s_rebuilds.push_back(std::move(rebuild));
}

namespace sweep_sidebag
{
std::vector<Rebuild> TakeRebuilds()
{
    std::vector<Rebuild> taken;
    taken.swap(s_rebuilds);
    return taken;
}

int Allocations()
{
    return s_allocations;
}
}

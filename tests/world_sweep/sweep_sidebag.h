#pragma once
// Probe on the game's sidebag path (sidebag_probe.cpp). The sweep links its
// own build of basic.cpp and sidebag.cpp with EnsureBasicSidebag and
// PsidebagNew wrapped, so it can tell when EnsureBasicSidebag replaces a
// sidebag the game itself allocated (a valid pointer) with an empty one,
// dropping every custom option (AddO/EnsureO/SetO) stored on the object.

#include <string>
#include <vector>

namespace sweep_sidebag
{
struct Rebuild
{
    int oid = -1;          // -1 when the object is not an LO
    int cid = -1;
    std::string name;      // LO name, "" otherwise
    unsigned long long sidebag = 0; // address of the sidebag that was dropped
    std::vector<int> lostOptions;   // option ids held by the dropped sidebag
};

// Rebuilds since the last call, oldest first.
std::vector<Rebuild> TakeRebuilds();
// Sidebags allocated through PsidebagNew so far (for the self-test).
int Allocations();
}

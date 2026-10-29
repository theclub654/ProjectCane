#pragma once
// The pieces of the child modes (sweep_child.cpp) that the other child files
// (sweep_behaviour_child.cpp) share: options, boot and shutdown, the frame
// stepper and the helpers that move Sly and press buttons.

#include "sweep_core.h"

#include "../../Sly1-Release/Input.h"
#include "../../Sly1-Release/game.h"
#include "../../Sly1-Release/lo.h"

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace sweep_child
{
struct Options
{
    std::map<std::string, std::string> values;
    std::vector<std::string> positional;

    std::string Get(const std::string& key, const std::string& fallback = {}) const
    {
        const auto it = values.find(key);
        return it == values.end() ? fallback : it->second;
    }
    int Int(const std::string& key, int fallback) const
    {
        const auto it = values.find(key);
        return it == values.end() ? fallback : std::atoi(it->second.c_str());
    }
    bool Flag(const std::string& key) const { return values.count(key) != 0; }
};

Options ParseOptions(const std::vector<std::string>& args, size_t first);

// Starts the game on a hidden window (Startup) with the user folder in a
// scratch directory, and drops the splash load StartGame queued.
void BootGame(const Options& options);
// Input devices SDL lets this process see; BootGame fails a test process
// that sees any.
int VisibleInputDevices();
void ShutdownGame();
void WriteJson(const std::filesystem::path& path, const nlohmann::json& json);
// Sidebags replaced since the last call (sidebag_probe.cpp), as report rows.
nlohmann::json TakeSidebagRebuilds(int& count);

struct Stepper
{
    sweep::LevelResult* result = nullptr;
    const LEVELINFO* plevel = nullptr;
    sweep::FrameHashes* hashes = nullptr;
    std::string firstBadObject;
    std::string firstBadOutOfWorld;
    // World positions and orientations; one checker per level, so "parked
    // since frame 0" means since the level's first frame.
    sweep::TransformChecker world{ 1.0e7f };
    // A level change the level asks for is held back (and recorded) unless
    // this is set; then it runs in the next RunGameFrame as in the game.
    bool allowTransitions = false;

    void ScanTransforms(int frame);
    // One RunGameFrame. frame < 0 is the load frame.
    double Step(int frame);
};

LO* PloFromOid(OID oid);
// The current state of the first SMA running `oidSm`, or OID_Nil.
OID SmStateOid(OID oidSm);
nlohmann::json PlayerPosition();
// debug.cpp's "Teleport Sly".
bool TeleportSly(const glm::vec3& pos);
// One press or release through the game's input layer (MOUSE and JOY).
void InjectButton(JOY_BUTTON button, bool down);
nlohmann::json LevelSummary(const sweep::LevelResult& r);
std::string SafeName(std::string name);

// sweep_behaviour_child.cpp
int ModeBehaviour(const Options& options);
int ModeUnlock(const Options& options);
}

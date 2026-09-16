#pragma once
#include "lo.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct IXAudio2;
struct IXAudio2MasteringVoice;
struct IXAudio2SubmixVoice;
struct IXAudio2SourceVoice;

struct SoundWaveFormat
{
    uint16_t formatTag = 1;
    uint16_t channels = 0;
    uint32_t samplesPerSec = 0;
    uint32_t avgBytesPerSec = 0;
    uint16_t blockAlign = 0;
    uint16_t bitsPerSample = 0;
};

inline constexpr std::size_t kSfxCount = 771;
inline constexpr std::size_t kEffectBankCount = 7;
inline constexpr float kSpuMaximumFrequencyRatio = 16383.0f / 4096.0f;
inline constexpr std::array<std::size_t, 8> kEffectBankStarts =
{
    0, 169, 234, 370, 491, 593, 691, 771
};

struct EXC
{
    IEXC iexc;
    DLE dle;
};

struct EFFECTVOICE
{
    IXAudio2SourceVoice* voice = nullptr;
    std::vector<int16_t> pcm;
    float baseVolume = 1.0f;
    float soundVolume = 1.0f;
    float toneVolume = 1.0f;
    int volumeGroup = 0;
    float baseFrequencyRatio = 1.0f;
    int8_t pitchBendLow = 0;
    int8_t pitchBendHigh = 0;
    int authoredPan = 0;
    uint32_t envelopeBits = 0;
    uint32_t envelopeCounter = 0;
    int envelopePhase = 4;
    int envelopeShift = 0;
    int envelopeStep = 0;
    int envelopeLevel = 0;
    int envelopeTarget = 0;
    bool loop = false;
    bool envelopeExponential = false;
    bool envelopeDecreasing = false;
};

struct EFFECTLFO
{
    uint8_t type = 0;
    uint8_t target = 0;
    uint8_t targetExtra = 0;
    uint16_t setupFlags = 0;
    int16_t depth = 0;
    uint32_t stepSize = 0;
    int32_t nextStep = 0;
    int32_t stateHold1 = 0;
    int32_t stateHold2 = 0;
    int32_t range = 0;
    uint32_t tick = 0;
    bool active = false;
};

struct AMB
{
    SFXID sfxid;
    int iSerial;
    SoundBankPtr bnk;
    struct AMB** ppamb;
    float tStarted;
    float uVolAtSource;
    SFXH sfxh;
    glm::vec3 pos;
    class ALO* palo;
    float sStart;
    float sFull;
    int fContinuous;
    int fOutOfRange;
    int fAttached;
    int fStopped;
    float volAttenuated;
    float pan;
    float frq;
    float rDoppler;
    int dnDoppler;
    DLE dle;
    int fPtchOrig;
    int ptchOrig;
	IXAudio2SourceVoice* voice;
	float frequencyRatioBase;
    std::vector<int16_t> pcm;
    std::vector<EFFECTVOICE> voices;
    int soundIndex;
    int nextGrain;
    int grainCountdown;
    int currentPitchBend;
    int grainsToPlay;
    int grainsToSkip;
    bool handlerDone;
    bool skipGrains;
    std::array<int8_t, 4> registers;
    std::array<EFFECTLFO, 4> lfos;
    int lfoVolume = 0;
    int lfoPan = 0;
    int lfoPitchMod = 0;
    int lfoPitchBend = 0;
    struct AMB* parent = nullptr;
    std::vector<struct AMB*> children;
    float tNext;
    LM lmRepeat;
    LM lmRepDist;
};
struct SFX
{
    SFXID sfxid;
    float sStart;
    float sFull;
    float uVol;
    float uPitch;
    LM lmRepeat;
    struct AMB *pamb;
    float uDoppler;
};

struct EFFECTBANK
{
    int ibnk = -1;
    std::vector<uint8_t> bytes;
    uint32_t bankId = 0;
    uint32_t version = 0;
    uint32_t sampleOffset = 0;
    uint32_t sampleSize = 0;

    struct TONE
    {
        int8_t priority = 0;
        int8_t volume = 0;
        int8_t centerNote = 0;
        int8_t centerFine = 0;
        int16_t pan = 0;
        int8_t mapLow = 0;
        int8_t mapHigh = 0;
        int8_t pitchBendLow = 0;
        int8_t pitchBendHigh = 0;
        uint16_t adsr1 = 0;
        uint16_t adsr2 = 0;
        uint16_t flags = 0;
        uint32_t sampleOffset = 0;
        uint32_t reserved = 0;
    };

    struct GRAIN
    {
        uint32_t type = 0;
        int32_t delay = 0;
        std::array<uint8_t, 32> data{};
        bool hasTone = false;
        TONE tone{};
    };

    struct SOUND
    {
        int8_t volume = 0;
        int8_t volumeGroup = 0;
        int16_t pan = 0;
        uint8_t grainCount = 0;
        int8_t instanceLimit = 0;
        uint16_t flags = 0;
        uint32_t firstGrainOffset = 0;
        std::vector<GRAIN> grains;

        bool IsLooper() const { return (flags & 1u) != 0; }
    };

    std::vector<SOUND> sounds;
};

// Maps the release game's global SFXID to a bank number and the sound's
// bank-local index. This is the retail replacement for the prototype SNDBNK.
struct SND
{
    int ibnk;
    int isnd;
};

enum class MusicState
{
    None,
    Loaded,
    Playing
};

struct MusicStreamState
{
    MusicState state = MusicState::None;
    MUSID musid = MUSID_Nil;
    std::filesystem::path path;
	bool paused = false;
};

struct MusicTone
{
    std::filesystem::path wavePath;
    int program = 0;
    int mapLow = 0;
    int mapHigh = 127;
    int volume = 127;
    int pan = 0;
    int centerNote = 60;
    int centerFine = 0;
    int pitchBendLow = 0;
    int pitchBendHigh = 0;
    uint16_t adsr1 = 0;
    uint16_t adsr2 = 0;
    bool loop = false;
    uint32_t loopBegin = 0;
    uint32_t loopLength = 0;
    SoundWaveFormat format{};
    std::vector<uint8_t> pcm;
};

enum class MusicEnvelopePhase
{
    Attack,
    Decay,
    Sustain,
    Release,
    Stopped
};

struct MusicEnvelope
{
    uint32_t bits = 0;
    MusicEnvelopePhase phase = MusicEnvelopePhase::Stopped;
    int shift = 0;
    int step = 0;
    bool exponential = false;
    bool decreasing = false;
    uint32_t counter = 0;
    int level = 0;
    int target = 0;
};

struct MusicProgram
{
    int volume = 127;
    int pan = 0;
};

struct MusicSegment
{
    uint32_t id = 0;
    const uint8_t* begin = nullptr;
    const uint8_t* cursor = nullptr;
    const uint8_t* end = nullptr;
    uint32_t tempo = 500000;
    int32_t ppq = 480;
    uint8_t status = 0;
    int ticksLeft = 0;
    double tickError = 0.0;
    int repeatsRemaining = 0;
    bool needDelta = true;
    bool complete = false;
    std::array<uint8_t, 16> programs{};
    std::array<uint8_t, 16> volumes{};
    std::array<int16_t, 16> pans{};
    std::array<int16_t, 16> pitchBends{};
    std::array<bool, 16> muted{};
};

struct MusicVoice
{
    IXAudio2SourceVoice* voice = nullptr;
    uint32_t segment = 0;
    uint8_t channel = 0;
    uint8_t note = 0;
    float baseVolume = 1.0f;
    float baseFrequencyRatio = 1.0f;
    int pitchBendLow = 0;
    int pitchBendHigh = 0;
    MusicEnvelope envelope{};
};

struct MusicAmeGroup
{
    uint8_t basis = 0;
    std::array<uint8_t, 16> channels{};
    std::array<uint8_t, 16> exciteMin{};
    std::array<uint8_t, 16> exciteMax{};
    uint8_t channelCount = 0;
    bool active = false;
};

struct Music989State
{
    std::vector<MusicTone> tones;
    std::vector<MusicProgram> programs;
    std::vector<uint8_t> sequence;
    std::vector<uint32_t> segmentOffsets;
    std::vector<uint32_t> segmentHandles;
    std::vector<MusicSegment> activeSegments;
    std::vector<MusicVoice> voices;
    std::array<uint8_t, 16> registers{};
    std::array<const uint8_t*, 16> macros{};
    std::array<MusicAmeGroup, 16> groups{};
    std::vector<uint32_t> pendingStarts;
    int cueRepeats = 0;
    int cueVolume = 127;
    int cuePan = 0;
    int cueVolumeGroup = 0;
    double tickAccumulator = 0.0;
    std::chrono::steady_clock::time_point lastUpdate{};
};

struct VagStreamState
{
    bool loaded = false;
    bool started = false;
    bool paused = false;
    bool commentary = false;
    std::filesystem::path path;
    SoundWaveFormat format{};
    std::vector<uint8_t> pcm;
    IXAudio2SourceVoice* voice = nullptr;
};

struct DecodedEffectSample
{
    std::vector<int16_t> pcm;
    uint32_t loopBegin = 0;
    uint32_t loopLength = 0;
    bool loop = false;
};

enum EffectEnvelopePhase
{
    EffectEnvelope_Attack,
    EffectEnvelope_Decay,
    EffectEnvelope_Sustain,
    EffectEnvelope_Release,
    EffectEnvelope_Stopped
};

// Sound implementation helpers
const glm::vec3* PposSoundEar();
template <typename T>
bool ReadBankValue(const std::vector<uint8_t>& bytes, std::size_t offset, T& value);
bool Parse989SfxBank(EFFECTBANK& bank);
std::filesystem::path ExecutableDirectory();
bool IsReadablePath(const char* value);
bool MatchesInsensitive(const std::filesystem::path& path, const std::wstring& name);
std::filesystem::path FindExtractedWav(const std::wstring& fileName);
std::filesystem::path FindCommentaryWav(uint32_t identity, uint32_t sector);
std::filesystem::path FindEffectBank(int ibnk);
std::filesystem::path FindMusicBank(MUSID musid);
bool LoadPcmWave(const std::filesystem::path& path, SoundWaveFormat& format,
    std::vector<uint8_t>& pcm);
bool EnsureEffectReverb();
void ApplyCurrentEffectReverb();
bool EnsureMusicEngine();
void RecalculateChannelVolume(int channel);
void CloseMusicVoice();
std::filesystem::path ResolveVagWav(const char* descriptorOrPath);
void CloseVagStream();
uint16_t MusicLe16(const uint8_t* p);
uint32_t MusicLe32(const uint8_t* p);
bool ParseMusicSequence();
bool DecodeMusicAdpcm(const std::vector<uint8_t>& data, std::size_t begin,
    std::size_t end, MusicTone& tone);
bool LoadMusic989Mus(const std::filesystem::path& path);
void LoadMusicAdpcmLoopPoints(const std::filesystem::path& directory);
bool LoadMusic989Bank(const std::filesystem::path& directory);
bool ReadMusicVlq(MusicSegment& segment, uint32_t& value);
void QueueMusicSegment(uint32_t id);
void StopMusicSegment(uint32_t id);
void SetGlobalExciteValue(int excite);
void ReapplyRegisterAmeGroups(int registerIndex);
bool RunMusicAme(MusicSegment& midi, const uint8_t*& stream);
void ConfigureMusicEnvelope(MusicEnvelope& envelope);
void StartMusicEnvelope(MusicEnvelope& envelope, uint16_t adsr1, uint16_t adsr2);
void ReleaseMusicEnvelope(MusicEnvelope& envelope);
void StepMusicEnvelope(MusicEnvelope& envelope);
void UpdateMusicEnvelopes();
void SetMusicVoicePan(IXAudio2SourceVoice* voice, int pan);
void StartMusicTone(MusicSegment& segment, uint8_t channel, uint8_t note, uint8_t velocity);
void StopMusicTone(uint32_t segment, uint8_t channel, uint8_t note);
void ActivateQueuedMusicSegments();
void StepMusicSegment(MusicSegment& segment);
bool DecodeEffectAdpcm(const EFFECTBANK& bank, uint32_t sampleOffset,
    DecodedEffectSample& decoded);
void SetEffectVoicePan(IXAudio2SourceVoice* voice, int pan);
int EffectSpatialPanDegrees(float pan);
float EffectVoiceVolume(const EFFECTVOICE& active, float applicationVolume);
void ConfigureEffectEnvelope(EFFECTVOICE& active);
void StartEffectEnvelope(EFFECTVOICE& active, uint16_t adsr1, uint16_t adsr2);
void ReleaseEffectEnvelope(EFFECTVOICE& active);
void StepEffectEnvelope(EFFECTVOICE& active);
bool PlayEffectTone(AMB& amb, const EFFECTBANK& bank, const EFFECTBANK::SOUND& sound,
    const EFFECTBANK::TONE& tone, float applicationVolume, float pitchMod);
int16_t EffectControlParam(const EFFECTBANK::GRAIN& grain, std::size_t index);
void SetEffectControlParam(EFFECTBANK::GRAIN& grain, std::size_t index, int16_t value);
void ApplyEffectPitchBend(AMB& amb, int bend);
int EffectLfoValue(EFFECTLFO& lfo);
void TickEffectLfos(AMB& amb);
void ConfigureEffectLfo(AMB& amb, const EFFECTBANK::GRAIN& grain);
void StartEffectChild(AMB& parent, const EFFECTBANK::GRAIN& grain);
void CleanEffectVoices(AMB& amb);
void UpdateEffectEnvelopes(AMB& amb);
void ExecuteEffectGrain(AMB& amb);

// Retail sound API
void StartSound(SFXID sfxid, AMB** ppamb, ALO* palo, glm::vec3* ppos,
    float sStart, float sFull, float uVolAtSource, float frq,
    float uDoppler, LM* plmRepeat, LM* plmRepDist);
AMB* PambAlloc();
void DropPamb(AMB** ppamb);
void RemoveAmb(AMB* pamb);
void StopSound(AMB* pamb, int msRampdown);
void PausePamb(AMB* pamb);
void ContinuePamb(AMB* pamb);
void SetAMRegister(int ireg, byte bVal);
void RefreshMidiRegister(int ireg);
void SetPambVol(AMB* pamb, float uVolAtSource);
void CalculateVolPan(float sDist, glm::vec3* pposSound, float* pvol, float* ppan,
    float uvol, float sStart, float sFull);
int FAmbientsPaused();
void SetPambFrq(AMB* pamb, float frq);
void SetDoppler(AMB* pamb);
void ScheduleNextIntermittentSound(AMB* pamb);
void UnsetExcitementHyst(EXC* pexc);
EXC* PexcAlloc();
void RemoveExc(EXC* pexc);
void SetIexcCurHigh(EXC* pexcParm);
EXC* PexcSetExcitement(IEXC iexc);
void UnsetExcitement(EXC* pexc);
void StartAloAmbient(float sStart, float sFull, ALO* palo, AMB** pamb);
bool FPauseForVag();
int FVagPlaying();
bool FHasAmbientVag();
int HsNextFootFall();
void NextSneakyFootstep();
void HandleWipeVolumes(int fMusicWipes, float u);
void ResetCommentaryAudio();
void SetAttractVolume(bool fMuted);
void SetAttractSoundOption(bool fEnabled);
void StopVag();
void SetMvgkRvol(float uVol, int mgvk, int iChannel);
void ResumeVag();
void PauseVag();
void ContinueVag();
void PauseMusicSequencer();
void ContinueMusicSequencer();
void PreloadVag(const char* filename);
void PushSwReverb(SW* psw, REVERBK rvrbk, int depth);
void PopSwReverb(SW* psw);
void PlayCommentaryAudio(char* audioFileLocation);
bool FPlayingCommentaryAudio();
void ContinueMusic();
void PauseMusic();
void PreloadMusidSong(MUSID musid);
void StartMusidSong(MUSID musid);
void UnloadMusic();
void KillMusic();
void KillSounds(int msRampdown);
void KillExcitement();
void ResetAMRegisters();
void KillSoundSystem();
SoundBankPtr SbpEnsureBank(int ibnk);
void UnloadEffectBank();
bool FContinuousSound(SFXID sfxid);
void SetSwDefaultReverb(SW* psw, REVERBK rvrbk, int depth);
void StartupSound();
void InitializeSoundOptions();
void SetMasterVolume(float uVol);
void SetSwIntermittentSound(SW* psw, SFXID sfxid);
void SetSwIntermittentVolPct(SW* psw, LM plmVolPct);
void SetSwIntermittentRepeat(SW* psw, LM plmRepeat);
void StartSwIntermittentSounds(SW* psw);
void UpdateSounds();

// Sound globals
extern int s_hvagAmbient;
extern IEXC g_iexcHyst;
extern int s_sfxidLast;
extern int g_fSneakyFeet;
extern IXAudio2* s_xaudio2;
extern IXAudio2MasteringVoice* s_musicMasterVoice;
extern IXAudio2SubmixVoice* s_effectReverbVoice;
extern REVERBK s_effectReverbKind;
extern int s_effectReverbDepth;
extern float s_masterVolume;
extern bool s_stereoOutputEnabled;
extern MusicStreamState s_musicStream;
extern float s_musicVolume;
extern float s_dialogMusicDuck;
extern std::array<uint8_t, 16> s_amRegisters;
extern uint8_t s_globalExcite;
extern IEXC s_iexcCurHigh;
extern int s_cframeExcitementHyst;
extern AMB* s_pambVagAmbient;
extern std::array<std::array<float, 4>, 11> s_relativeVolumes;
extern std::array<float, 4> s_channelVolumes;
extern Music989State s_music989;
extern std::array<std::unique_ptr<EFFECTBANK>, kEffectBankCount> s_effectBanks;
extern int s_ibnkWorldCur;
extern std::array<bool, kSfxCount> s_mpsfxidcont;
extern std::array<bool, kEffectBankCount> s_mpibnkfEverLoaded;
extern double s_effectTickAccumulator;
extern std::chrono::steady_clock::time_point s_effectLastUpdate;
extern std::array<int8_t, 32> s_effectGlobalRegisters;
extern float s_tLastIntermittent;
extern int s_fPauseAmbients;
extern VagStreamState s_vagStream;
extern const std::array<const wchar_t*, kEffectBankCount> kEffectBankNames;
extern LM s_lmRepeatDefault;
extern LM s_lmDistDefault;
extern const std::array<SND, kSfxCount> g_mpsfxidsndbnk;

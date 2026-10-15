using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace File_Extractor_GUI.Services;

public sealed class Sly1MediaExtractionService
{
    private const int SectorSize = 0x800;
    private const uint ElfVirtualBase = 0x00100000, ElfFileBase = 0x1000;
    private const uint LevelTableVa = 0x00247AF0, SoundTableVa = 0x00244F40, CommentaryTableVa = 0x00248748;
    private const uint AmbientTableVa = 0x002482D8, CutsceneTableVa = 0x00248490, PromptVa = 0x0026A118;
    private const uint SplashSector = 0xC8F03, SplashSize = 0xAB4004;
    private static readonly string[] BankNames = ["Common", "Intro", "Underwater", "Muggshot", "Voodoo", "Snow", "Clockwerk"];

    private static readonly (string Level, uint Id, uint Sector, uint Size)[] Music =
    [
        ("Paris, France",0x55,0xC8E0E,0x7A6A0),("The Hideout",0x43,0xD3411,0x6F7A0),("A Cunning Disguise",0x47,0xC4602,0x72324),
        ("A Stealthy Approach",0x51,0xD44ED,0x745F8),("High Class Heist",0x59,0xCDE0B,0x74674),("Into the Machine",0x3F,0xC839D,0x7B414),
        ("Prowling the Grounds",0x44,0xBFA16,0x746D4),("The Eye of the Storm",0x57,0xC354B,0x7A0C4),("The Fire Down Below",0x46,0xC130C,0x775E0),
        ("The Gunboat Graveyard",0x42,0xCAC3D,0x70D4C),("Treasure in the Depths",0x3E,0xD4E9D,0x748F8),("A Rocky Start",0x50,0xBC6B4,0x64A34),
        ("At the Dog Track",0x4E,0xCFBE5,0x726F8),("Back Alley Heist",0x4C,0xC1B99,0x77978),("Boneyard Casino",0x49,0xCF341,0x71A18),
        ("Last Call",0x48,0xC02DE,0x731B8),("Muggshot's Turf",0x40,0xCE9BB,0x5DB18),("Murray's Big Gamble",0x4A,0xCD592,0x767B8),
        ("Straight to the Top",0x4C,0xD04EF,0x77978),("Two to Tango",0x4D,0xD5A8B,0x788FC),("A Deadly Dance",0x68,0xBE35F,0x771E8),
        ("A Ghastly Voyage",0x65,0xC6338,0x76D9C),("A Grave Undertaking",0x55,0xCCD68,0x7A6A0),("Descent into Danger",0x64,0xC25C6,0x6EBB8),
        ("Down Home Cooking",0x62,0xC50BB,0x73978),("Piranha Lake",0x67,0xD4AFA,0x73978),("The Dread Swamp Path",0x60,0xC6D86,0x76A60),
        ("The Lair of the Beast",0x64,0xCB6A5,0x6EBB8),("The Swamp's Dark Center",0x64,0xD1852,0x6EBB8),("A Desperate Race",0x4E,0xCBD9B,0x726F8),
        ("A Perilous Ascent",0x53,0xD2424,0x773CC),("Duel by the Dragon",0x5B,0xBD2A0,0x6DA90),("Flame Fu!",0x56,0xC4B48,0x6EA6C),
        ("Flaming Temple of Flame",0x54,0xBEF0F,0x7737C),("Inside the Stronghold",0x5B,0xC7870,0x6DA90),("Rapid Fire Assault",0x5D,0xC2F89,0x6C2BC),
        ("The King of the Hill",0x5C,0xD2D42,0x78064),("The Unseen Foe",0x44,0xC5A33,0x746D4),("A Daring Rescue",0x38,0xBD8B1,0x788F8),
        ("A Hazardous Path",0x35,0xC3CB5,0x77C7C),("A Strange Reunion",0x34,0xC0AAC,0x95774),("A Temporary Truce",0x39,0xD0BD7,0x74DAC),
        ("Bentley Comes Through",0x36,0xD50F1,0x8C528),("Burning Rubber",0x3A,0xCC1CA,0x77614),("Sinking Peril",0x37,0xD3A9E,0x77F08)
    ];

    public async Task ExtractAsync(string isoPath, string output, long exeIsoOffset, IProgress<Sly1ExtractionProgress>? progress, CancellationToken ct)
    {
        await using var iso = new FileStream(isoPath, FileMode.Open, FileAccess.Read, FileShare.Read, 131072, true);
        var dialogs = LoadDialogTable();
        var total = Music.Select(x => x.Id).Distinct().Count() + 7 + 33 + dialogs.Count + 18 + 6 + 1;
        var done = 0;
        void Report(string item) => progress?.Report(new(item, done, total, done, total));

        var worlds = Path.Combine(output, "Worlds"); var sounds = Path.Combine(worlds, "Sounds");
        var musicDir = Path.Combine(worlds, "Music"); Directory.CreateDirectory(musicDir);
        await File.WriteAllLinesAsync(Path.Combine(musicDir, "MusicMap.txt"), Music.Select(x => $"{x.Level} | MUSID 0x{x.Id:X} | sector 0x{x.Sector:X} | bytes 0x{x.Size:X}"), ct);
        foreach (var item in Music.GroupBy(x => x.Id).Select(x => x.First()))
        {
            Report($"Music / MUSID_{item.Id:X2}.mus");
            await CopyIsoAsync(iso, item.Sector, item.Size, Path.Combine(musicDir, $"MUSID_{item.Id:X2}.mus"), ct); done++;
        }

        var banks = await ReadDescriptorsAsync(iso, exeIsoOffset, SoundTableVa, 7, ct);
        var bankDir = Path.Combine(worlds, "Sound Banks"); Directory.CreateDirectory(bankDir);
        for (var i = 0; i < banks.Count; i++) { Report($"Sound Banks / {BankNames[i]}.bnk"); await CopyDescriptorAsync(iso, banks[i], Path.Combine(bankDir, BankNames[i] + ".bnk"), false, ct); done++; }

        var commentaryBytes = await ReadAtAsync(iso, VaOffset(exeIsoOffset, CommentaryTableVa), 33 * 44, ct);
        var commentaryDir = Path.Combine(sounds, "Commentary"); Directory.CreateDirectory(commentaryDir);
        var commentaryMap = new List<string>();
        for (var i = 0; i < 33; i++)
        {
            var span = commentaryBytes.AsSpan(i * 44, 44); var levelId = Le32(span, 0); var duration = BitConverter.Int32BitsToSingle(unchecked((int)Le32(span, 4))); var descriptor = Descriptor(span.Slice(8, 32)); var speakerVa = Le32(span, 40);
            if (descriptor.Sector == 0 || descriptor.Size == 0) { done++; continue; }
            var name = $"VAG_{descriptor.Identity:X4}_{descriptor.Sector:X6}.wav"; Report($"Sounds / Commentary / {name}");
            var vag = await ReadAtAsync(iso, (long)descriptor.Sector * SectorSize, checked((int)descriptor.Size), ct);
            await File.WriteAllBytesAsync(Path.Combine(commentaryDir, name), Sly1AudioCodec.DecodeVag(vag), ct);
            var speakers = await ReadExeStringAsync(iso, exeIsoOffset, speakerVa, ct) ?? "Unknown";
            speakers = speakers.Replace("\r\n", "\n").Replace("\n", Environment.NewLine);
            commentaryMap.Add($"{name} | levelId 0x{levelId:X} | duration {duration.ToString(CultureInfo.InvariantCulture)} | identity 0x{descriptor.Identity:X} | sector 0x{descriptor.Sector:X} | bytes 0x{descriptor.Size:X} | speakers {speakers}"); done++;
        }
        await File.WriteAllLinesAsync(Path.Combine(commentaryDir, "CommentaryMap.txt"), commentaryMap, ct);

        var levelNames = await ReadLevelNamesAsync(iso, exeIsoOffset, ct);
        var initialized = new HashSet<uint>();
        foreach (var dialog in dialogs)
        {
            var levelName = dialog.LevelId switch
            {
                3 => "Paris",
                4 => "The Hideout",
                _ => levelNames.GetValueOrDefault(dialog.LevelId, $"Level_{dialog.LevelId}")
            };
            var episode = Episode(dialog.LevelId); var dir = Path.Combine(worlds, episode, levelName, "Dialog"); Directory.CreateDirectory(dir);
            var name = $"VAG_{dialog.Identity:X4}_{dialog.Sector:X6}.wav"; Report($"Worlds / {episode} / {levelName} / Dialog / {name}");
            var size = dialog.Size;
            if (size == 0) { var header = await ReadAtAsync(iso, (long)dialog.Sector * SectorSize, 0x30, ct); size = ReadBe32(header, 0x0C) + 0x30; }
            var vag = await ReadAtAsync(iso, (long)dialog.Sector * SectorSize, checked((int)size), ct);
            await File.WriteAllBytesAsync(Path.Combine(dir, name), Sly1AudioCodec.DecodeVag(vag), ct);
            var mode = initialized.Add(dialog.LevelId) ? FileMode.Create : FileMode.Append;
            await using (var map = new FileStream(Path.Combine(dir, "DialogMap.txt"), mode, FileAccess.Write, FileShare.Read, 4096, true))
            await using (var writer = new StreamWriter(map)) await writer.WriteLineAsync($"{name} | levelId 0x{dialog.LevelId:X} | identity 0x{dialog.Identity:X} | sector 0x{dialog.Sector:X} | bytes 0x{size:X} | source {dialog.Source}"); done++;
        }

        var cutscenes = await ReadDescriptorsAsync(iso, exeIsoOffset, CutsceneTableVa, 17, ct);
        var cutsceneDir = Path.Combine(output, "Cutscenes"); Directory.CreateDirectory(cutsceneDir);
        for (var i = 0; i < 18; i++)
        {
            var d = i == 17 ? new DescriptorData(SplashSector, SplashSize, 0x344) : cutscenes[i];
            var outputIndex = i == 17 ? 17 : i == 0 ? 16 : i - 1; var stem = $"Cutscene_{outputIndex:00}"; Report($"Cutscenes / {stem}");
            var pss = await ReadAtAsync(iso, (long)d.Sector * SectorSize, checked((int)d.Size), ct); var streams = Sly1AudioCodec.DemuxPss(pss);
            await File.WriteAllBytesAsync(Path.Combine(cutsceneDir, stem + ".m2v"), streams.Video, ct); await File.WriteAllBytesAsync(Path.Combine(cutsceneDir, stem + ".wav"), streams.Audio, ct); done++;
        }

        var ambient = await ReadDescriptorsAsync(iso, exeIsoOffset, AmbientTableVa, 6, ct); var ambientDir = Path.Combine(sounds, "JLO Ambient"); Directory.CreateDirectory(ambientDir);
        for (var i = 0; i < ambient.Count; i++) { var name = $"JloAmbient_{i + 1}.wav"; Report($"Sounds / JLO Ambient / {name}"); await CopyDescriptorAsync(iso, ambient[i], Path.Combine(ambientDir, name), true, ct); done++; }
        var prompt = (await ReadDescriptorsAsync(iso, exeIsoOffset, PromptVa, 1, ct))[0]; var promptDir = Path.Combine(sounds, "Prompt"); Directory.CreateDirectory(promptDir); Report("Sounds / Prompt / PromptPrk18.wav"); await CopyDescriptorAsync(iso, prompt, Path.Combine(promptDir, "PromptPrk18.wav"), true, ct); done++; Report("Media extraction complete");
    }

    private static async Task<List<DescriptorData>> ReadDescriptorsAsync(FileStream iso, long exe, uint va, int count, CancellationToken ct)
    { var bytes = await ReadAtAsync(iso, VaOffset(exe, va), count * 32, ct); var result = new List<DescriptorData>(count); for (var i = 0; i < count; i++) result.Add(Descriptor(bytes.AsSpan(i * 32, 32))); return result; }
    private static DescriptorData Descriptor(ReadOnlySpan<byte> d) => new(Le32(d,0)^Le32(d,20), Le32(d,4)^Le32(d,28), Le32(d,8)^Le32(d,24));
    private static async Task CopyDescriptorAsync(FileStream iso, DescriptorData d, string path, bool wav, CancellationToken ct)
    { if (d.Sector == 0 || d.Size == 0) throw new InvalidDataException("Invalid media descriptor."); var data = await ReadAtAsync(iso,(long)d.Sector*SectorSize,checked((int)d.Size),ct); await File.WriteAllBytesAsync(path,wav?Sly1AudioCodec.DecodeVag(data):data,ct); }
    private static async Task CopyIsoAsync(FileStream iso,uint sector,uint size,string path,CancellationToken ct) => await File.WriteAllBytesAsync(path,await ReadAtAsync(iso,(long)sector*SectorSize,checked((int)size),ct),ct);
    private static long VaOffset(long exe,uint va) => exe + (va-ElfVirtualBase) + ElfFileBase;
    private static async Task<byte[]> ReadAtAsync(FileStream stream,long offset,int size,CancellationToken ct) { if(offset<0||size<0||offset>stream.Length||size>stream.Length-offset) throw new EndOfStreamException("Media points outside the ISO."); var b=new byte[size]; stream.Seek(offset,SeekOrigin.Begin); var p=0; while(p<size){var n=await stream.ReadAsync(b.AsMemory(p),ct);if(n==0)throw new EndOfStreamException();p+=n;}return b; }
    private static async Task<string?> ReadExeStringAsync(FileStream iso,long exe,uint va,CancellationToken ct) { if(va<ElfVirtualBase)return null; var b=new List<byte>(); iso.Seek(VaOffset(exe,va),SeekOrigin.Begin); var one=new byte[1]; for(var i=0;i<256;i++){if(await iso.ReadAsync(one,ct)!=1)return null;if(one[0]==0)return Encoding.ASCII.GetString(b.ToArray());b.Add(one[0]);}return null; }
    private static async Task<Dictionary<uint,string>> ReadLevelNamesAsync(FileStream iso,long exe,CancellationToken ct)
    {
        var table=await ReadAtAsync(iso,VaOffset(exe,LevelTableVa),46*44,ct);var result=new Dictionary<uint,string>();
        for(var i=0;i<46;i++){var span=table.AsSpan(i*44,44);var id=Le32(span,32);var name=await ReadExeStringAsync(iso,exe,Le32(span,36),ct);if(!string.IsNullOrWhiteSpace(name)&&!result.ContainsKey(id))result.Add(id,Sanitize(name));}
        return result;
    }
    private static string Sanitize(string name){var chars=name.ToCharArray();for(var i=0;i<chars.Length;i++)if(chars[i] is '<' or '>' or ':' or '"' or '/' or '\\' or '|' or '?' or '*'||char.IsControl(chars[i]))chars[i]='_';name=new string(chars).TrimEnd(' ','.');return string.IsNullOrWhiteSpace(name)?"Unnamed Level":name;}
    private static uint Le32(ReadOnlySpan<byte>d,int o)=>BinaryPrimitives.ReadUInt32LittleEndian(d.Slice(o,4)); private static uint ReadBe32(byte[]d,int o)=>BinaryPrimitives.ReadUInt32BigEndian(d.AsSpan(o,4));
    private static string Episode(uint id)=>(id>>8)switch{1=>"Tide of Terror",2=>"Sunset Snake Eyes",3=>"Vicious Voodoo",4=>"Fire in the sky",5=>"The Cold Heart of Hate",_=>string.Empty};
    private static List<DialogData> LoadDialogTable()
    {
        var assembly=Assembly.GetExecutingAssembly(); var resource=assembly.GetManifestResourceNames().Single(x=>x.EndsWith("DialogAudioTable.inc",StringComparison.OrdinalIgnoreCase)); using var reader=new StreamReader(assembly.GetManifestResourceStream(resource)!); var text=reader.ReadToEnd();
        var regex=new Regex(@"\{\s*(?:0x)?([0-9A-F]+),\s*(?:0x)?([0-9A-F]+),\s*(?:0x)?([0-9A-F]+),\s*(?:0x)?([0-9A-F]+),\s*DialogAudioSource::(\w+)\s*\}",RegexOptions.IgnoreCase); var result=new List<DialogData>();
        foreach(Match m in regex.Matches(text)) result.Add(new(ParseHex(m.Groups[1].Value),ParseHex(m.Groups[2].Value),ParseHex(m.Groups[3].Value),ParseHex(m.Groups[4].Value),m.Groups[5].Value)); return result;
    }
    private static uint ParseHex(string value)=>uint.Parse(value,NumberStyles.HexNumber,CultureInfo.InvariantCulture);
    private readonly record struct DescriptorData(uint Sector,uint Size,uint Identity); private readonly record struct DialogData(uint LevelId,uint Sector,uint Size,uint Identity,string Source);
}

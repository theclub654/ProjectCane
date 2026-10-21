using System;
using System.IO;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace File_Extractor_GUI.Services;

internal static class ExtractionManifestService
{
    public const string FileName = "extraction_manifest.json";

    public static async Task WriteAsync(
        string outputFolder,
        Sly1DiscRegion region,
        CancellationToken cancellationToken)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(outputFolder);

        var manifest = region switch
        {
            Sly1DiscRegion.NtscU => new ExtractionManifest(
                SchemaVersion: 1,
                Game: "Sly Cooper and the Thievius Raccoonus",
                Region: "NTSC-U",
                Serial: "SCUS-971.98",
                LevelFormat: "PCBRX",
                LevelFormatVersion: 19,
                Languages: ["English"]),
            Sly1DiscRegion.Pal => new ExtractionManifest(
                SchemaVersion: 1,
                Game: "Sly Raccoon",
                Region: "PAL",
                Serial: "SCES-509.17",
                LevelFormat: "PCBRX",
                LevelFormatVersion: 19,
                Languages: ["English", "French", "German", "Spanish", "Italian"]),
            Sly1DiscRegion.Japanese => new ExtractionManifest(
                SchemaVersion: 1,
                Game: "Kaitou Sly Cooper",
                Region: "NTSC-J",
                Serial: "SCPS-150.36",
                LevelFormat: "PCBRX",
                LevelFormatVersion: 19,
                Languages: ["Japanese"],
                VoiceLanguages: ["Japanese", "English"]),
            _ => throw new ArgumentOutOfRangeException(nameof(region), region, "Unsupported Sly 1 disc region.")
        };

        Directory.CreateDirectory(outputFolder);
        var path = Path.Combine(outputFolder, FileName);
        var temporaryPath = path + ".tmp";
        var options = new JsonSerializerOptions
        {
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
            WriteIndented = true
        };

        try
        {
            await using (var output = new FileStream(
                temporaryPath,
                FileMode.Create,
                FileAccess.Write,
                FileShare.None,
                bufferSize: 4096,
                useAsync: true))
            {
                await JsonSerializer.SerializeAsync(output, manifest, options, cancellationToken);
            }

            File.Move(temporaryPath, path, overwrite: true);
        }
        finally
        {
            if (File.Exists(temporaryPath))
                File.Delete(temporaryPath);
        }
    }

    private sealed record ExtractionManifest(
        int SchemaVersion,
        string Game,
        string Region,
        string Serial,
        string LevelFormat,
        int LevelFormatVersion,
        string[] Languages,
        string[]? VoiceLanguages = null);
}

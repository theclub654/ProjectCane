using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace File_Extractor_GUI.Services;

public sealed partial class Sly1PalExtractionService
{
    private const int LevelCount = 46;
    private static readonly string[] PalLanguages =
        { "English", "French", "German", "Spanish", "Italian" };
    private static readonly string[] JapaneseVoiceLanguages = { "Japanese", "English" };

    public async Task ExtractAsync(
        string isoPath,
        string outputFolder,
        Sly1DiscRegion region,
        IProgress<Sly1ExtractionProgress>? progress,
        CancellationToken cancellationToken)
    {
        if (region is not (Sly1DiscRegion.Pal or Sly1DiscRegion.Japanese))
            throw new ArgumentOutOfRangeException(nameof(region));
        var languages = region == Sly1DiscRegion.Japanese
            ? JapaneseVoiceLanguages : PalLanguages;
        var extractedLevelCount = languages.Length * LevelCount;
        var totalWorkItems = extractedLevelCount + LevelCount;
        var regionName = region == Sly1DiscRegion.Japanese ? "Japanese" : "PAL";
        var extractorPath = FindExtractor();
        var startInfo = new ProcessStartInfo
        {
            FileName = extractorPath,
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true
        };
        startInfo.ArgumentList.Add("extract");
        startInfo.ArgumentList.Add(isoPath);
        startInfo.ArgumentList.Add(outputFolder);

        using var process = new Process { StartInfo = startInfo };
        var errors = new StringBuilder();
        process.ErrorDataReceived += (_, args) =>
        {
            if (args.Data is not null)
                errors.AppendLine(args.Data);
        };

        if (!process.Start())
            throw new InvalidOperationException($"The {regionName} extractor could not be started.");

        process.BeginErrorReadLine();
        var completed = 0;
        while (await process.StandardOutput.ReadLineAsync(cancellationToken) is { } line)
        {
            var match = LevelProgressRegex().Match(line);
            if (!match.Success)
                continue;

            completed++;
            progress?.Report(new Sly1ExtractionProgress(
                $"{match.Groups[1].Value} / level {match.Groups[2].Value} of {LevelCount}",
                completed,
                totalWorkItems,
                completed,
                totalWorkItems));
        }

        try
        {
            await process.WaitForExitAsync(cancellationToken);
        }
        catch (OperationCanceledException)
        {
            if (!process.HasExited)
                process.Kill(entireProcessTree: true);
            throw;
        }

        if (process.ExitCode != 0)
            throw new InvalidDataException($"{regionName} extraction failed: {errors.ToString().Trim()}");

        var worldsDirectory = Path.Combine(outputFolder, "Worlds");
        var levelIdentityMapPath = Path.Combine(outputFolder, ".pal-level-identities.txt");
        if (region == Sly1DiscRegion.Pal && !File.Exists(levelIdentityMapPath))
            throw new InvalidDataException("The PAL extractor did not produce its level identity map.");
        var levels = Directory.EnumerateFiles(worldsDirectory, "*.brx", SearchOption.AllDirectories)
            .Where(path => IsLanguageLevel(path, languages))
            .OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
            .ToArray();
        if (levels.Length != extractedLevelCount)
            throw new InvalidDataException($"Expected {extractedLevelCount} {regionName} level variants, but found {levels.Length}.");

        var groups = levels.GroupBy(GetLanguageIndependentLevelPath,
                StringComparer.OrdinalIgnoreCase)
            .OrderBy(group => group.Key, StringComparer.OrdinalIgnoreCase)
            .ToArray();
        if (groups.Length != LevelCount)
            throw new InvalidDataException($"Expected {LevelCount} {regionName} level groups, but found {groups.Length}.");

        for (var index = 0; index < groups.Length; index++)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var files = groups[index].ToDictionary(
                path => Directory.GetParent(path)!.Name,
                StringComparer.OrdinalIgnoreCase);
            var languagePaths = languages.Select(language =>
                files.TryGetValue(language, out var path) ? path :
                    throw new InvalidDataException($"{regionName} level '{groups[index].Key}' is missing {language}."))
                .ToArray();
            var outputPath = groups[index].Key;
            var relativePath = Path.GetRelativePath(worldsDirectory, outputPath);
            progress?.Report(new Sly1ExtractionProgress(
                $"Combining {regionName} variants for {relativePath}",
                extractedLevelCount + index,
                totalWorkItems,
                extractedLevelCount + index,
                totalWorkItems));
            await BrxRebuildService.RebuildLocalizedCombinedAsync(
                languagePaths, outputPath, region, cancellationToken,
                region == Sly1DiscRegion.Pal ? levelIdentityMapPath : null);
        }

        progress?.Report(new Sly1ExtractionProgress(
            $"Rebuilt {regionName} levels and localized media",
            totalWorkItems,
            totalWorkItems,
            totalWorkItems,
            totalWorkItems));
    }

    private static bool IsLanguageLevel(string path, IReadOnlyCollection<string> languages)
    {
        var parent = Directory.GetParent(path)?.Name;
        return parent is not null && languages.Contains(parent, StringComparer.OrdinalIgnoreCase);
    }

    private static string GetLanguageIndependentLevelPath(string path)
    {
        var languageDirectory = Directory.GetParent(path)
            ?? throw new InvalidDataException($"PAL level '{path}' has no language directory.");
        var levelDirectory = languageDirectory.Parent
            ?? throw new InvalidDataException($"PAL level '{path}' has no level directory.");
        return Path.Combine(levelDirectory.FullName, Path.GetFileName(path));
    }

    private static string FindExtractor()
    {
        var executable = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? "Sly1PalExtractor.exe"
            : "Sly1PalExtractor";
        try
        {
            return BundledToolService.Extract(executable);
        }
        catch (FileNotFoundException)
        {
            // Keep source-tree lookup available for development builds.
        }
        var candidates = new List<string>();
        var directory = new DirectoryInfo(AppContext.BaseDirectory);
        candidates.Add(Path.Combine(directory.FullName, executable));
        candidates.Add(Path.Combine(directory.FullName, "Tools", executable));

        for (var depth = 0; depth < 6 && directory is not null; depth++, directory = directory.Parent)
        {
            candidates.Add(Path.Combine(directory.FullName, "Tools", "PalExtractor", "x64", "Debug", executable));
            candidates.Add(Path.Combine(directory.FullName, "Tools", "PalExtractor", "x64", "Release", executable));
        }

        var newest = candidates.Where(File.Exists)
            .Select(path => new FileInfo(path))
            .OrderByDescending(file => file.LastWriteTimeUtc)
            .FirstOrDefault();
        if (newest is not null)
            return newest.FullName;

        throw new FileNotFoundException($"The bundled PAL extractor ({executable}) is missing. Rebuild the ProjectCane solution or reinstall the package.");
    }

    [GeneratedRegex(@"^\[(English|French|German|Spanish|Italian|Japanese)\s+(\d+)/46\]")]
    private static partial Regex LevelProgressRegex();
}

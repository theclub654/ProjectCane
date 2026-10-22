using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace File_Extractor_GUI.Services;

internal static class BrxRebuildService
{
    public static async Task RebuildLocalizedCombinedAsync(
        IReadOnlyList<string> languagePaths,
        string outputPath,
        Sly1DiscRegion region,
        CancellationToken cancellationToken,
        string? levelIdentityMapPath = null)
    {
        var expectedCount = region == Sly1DiscRegion.Japanese ? 2 : 5;
        if (languagePaths.Count != expectedCount)
            throw new ArgumentException($"A combined {region} level requires {expectedCount} inputs.", nameof(languagePaths));

        var outputDirectory = Path.GetDirectoryName(outputPath)
            ?? throw new InvalidOperationException("The level output path has no directory.");
        Directory.CreateDirectory(outputDirectory);
        var temporaryPaths = new List<string>(5);
        var rebuiltPaths = new List<string>(5);
        try
        {
            if (region == Sly1DiscRegion.Japanese)
                await ValidateJapaneseSourceLocalizationParityAsync(languagePaths,
                    cancellationToken);

            foreach (var languagePath in languagePaths)
            {
                var temporaryPath = Path.Combine(outputDirectory,
                    $".{Path.GetFileNameWithoutExtension(outputPath)}.{Guid.NewGuid():N}.brx");
                File.Copy(languagePath, temporaryPath, overwrite: true);
                temporaryPaths.Add(temporaryPath);
            }
            foreach (var temporaryPath in temporaryPaths)
            {
                var rebuiltPath = Path.Combine(outputDirectory,
                    Path.GetFileNameWithoutExtension(temporaryPath) + "_Rebuilt.brx");
                await RunRebuilderAsync(new[] { temporaryPath }, rebuiltPath, cancellationToken,
                    region == Sly1DiscRegion.Japanese ? "japanese" : "pal",
                    levelIdentityMapPath);
                await ValidatePcBrxAsync(rebuiltPath, requirePalLocalization: true,
                    expectedVersion: 17, cancellationToken: cancellationToken);
                rebuiltPaths.Add(rebuiltPath);
            }

            for (var index = 0; index < rebuiltPaths.Count; ++index)
                await ValidateReferencedAudioAsync(rebuiltPaths[index], languagePaths[index],
                    cancellationToken);
            if (region == Sly1DiscRegion.Japanese)
                await ValidateJapaneseLocalizationParityAsync(rebuiltPaths, cancellationToken);

            var variantIds = region == Sly1DiscRegion.Japanese
                ? new uint[] { 0, 1 }
                : new uint[] { 0, 1, 2, 3, 4 };
            await WriteUnifiedContainerAsync(rebuiltPaths, outputPath,
                variantIds, defaultLanguage: 0,
                cancellationToken: cancellationToken);
            foreach (var languagePath in languagePaths)
                TryDelete(languagePath);
        }
        finally
        {
            foreach (var path in temporaryPaths)
                TryDelete(path);
            foreach (var path in rebuiltPaths)
                TryDelete(path);
        }
    }

    private static async Task WriteUnifiedContainerAsync(
        IReadOnlyList<string> payloadPaths,
        string outputPath,
        IReadOnlyList<uint> languageIds,
        uint defaultLanguage,
        CancellationToken cancellationToken)
    {
        if (payloadPaths.Count == 0 || payloadPaths.Count != languageIds.Count)
            throw new ArgumentException("A unified level requires one language ID per payload.");
        if (!languageIds.Contains(defaultLanguage) || languageIds.Distinct().Count() != languageIds.Count)
            throw new ArgumentException("The unified level language directory is invalid.", nameof(languageIds));

        const uint pcBrxMagic = 0x52424350;
        const uint pcBrxVersion = 19;
        const uint unifiedMagic = 0x494E5550; // "PUNI"
        const uint unifiedSchema = 1;
        var payloads = new List<byte[]>(payloadPaths.Count);
        foreach (var path in payloadPaths)
            payloads.Add(await File.ReadAllBytesAsync(path, cancellationToken));

        var chunks = new List<byte[]>();
        var chunkIds = new Dictionary<string, uint>(StringComparer.Ordinal);
        var payloadChunkIds = new List<List<uint>>(payloads.Count);
        foreach (var payload in payloads)
        {
            var references = new List<uint>();
            foreach (var chunk in SplitContentDefinedChunks(payload))
            {
                var key = Convert.ToHexString(SHA256.HashData(chunk));
                if (!chunkIds.TryGetValue(key, out var id))
                {
                    id = (uint)chunks.Count;
                    chunkIds.Add(key, id);
                    chunks.Add(chunk);
                }
                else if (!chunks[(int)id].AsSpan().SequenceEqual(chunk))
                {
                    throw new InvalidDataException("A content hash collision occurred while packing the level.");
                }
                references.Add(id);
            }
            payloadChunkIds.Add(references);
        }

        for (var payloadIndex = 0; payloadIndex < payloads.Count; ++payloadIndex)
        {
            var offset = 0;
            foreach (var chunkId in payloadChunkIds[payloadIndex])
            {
                var chunk = chunks[(int)chunkId];
                if (offset > payloads[payloadIndex].Length - chunk.Length ||
                    !payloads[payloadIndex].AsSpan(offset, chunk.Length).SequenceEqual(chunk))
                {
                    throw new InvalidDataException("The unified level chunk map did not reconstruct its source payload.");
                }
                offset += chunk.Length;
            }
            if (offset != payloads[payloadIndex].Length)
                throw new InvalidDataException("The unified level chunk map has the wrong payload length.");
        }

        var temporaryOutput = outputPath + $".{Guid.NewGuid():N}.tmp";
        try
        {
            await using (var output = new FileStream(temporaryOutput, FileMode.CreateNew,
                FileAccess.Write, FileShare.None, 4096, useAsync: true))
            {
                using var writer = new BinaryWriter(output, Encoding.UTF8, leaveOpen: true);
                writer.Write(pcBrxMagic);
                writer.Write(pcBrxVersion);
                writer.Write(unifiedMagic);
                writer.Write(unifiedSchema);
                writer.Write((uint)payloads.Count);
                writer.Write(defaultLanguage);
                writer.Write((uint)chunks.Count);
                for (var language = 0; language < payloads.Count; ++language)
                {
                    writer.Write(languageIds[language]);
                    writer.Write((ulong)payloads[language].Length);
                    writer.Write((uint)payloadChunkIds[language].Count);
                    foreach (var chunkId in payloadChunkIds[language])
                        writer.Write(chunkId);
                }
                foreach (var chunk in chunks)
                {
                    writer.Write((uint)chunk.Length);
                    writer.Write(Crc32(chunk));
                    writer.Write(chunk);
                }
                writer.Flush();
                await output.FlushAsync(cancellationToken);
            }
            File.Move(temporaryOutput, outputPath, overwrite: true);
        }
        finally
        {
            TryDelete(temporaryOutput);
        }
    }

    private static uint Crc32(ReadOnlySpan<byte> data)
    {
        uint crc = 0xFFFFFFFFu;
        foreach (var value in data)
        {
            crc ^= value;
            for (var bit = 0; bit < 8; ++bit)
            {
                var mask = 0u - (crc & 1u);
                crc = (crc >> 1) ^ (0xEDB88320u & mask);
            }
        }
        return ~crc;
    }

    private static IEnumerable<byte[]> SplitContentDefinedChunks(byte[] payload)
    {
        const int minimumSize = 16 * 1024;
        const int maximumSize = 256 * 1024;
        const ulong boundaryMask = (1UL << 16) - 1; // About 64 KiB average.
        var start = 0;
        ulong hash = 0;
        for (var index = 0; index < payload.Length; ++index)
        {
            hash = (hash << 1) + GearValue(payload[index]);
            var length = index - start + 1;
            if (length < minimumSize || (length < maximumSize && (hash & boundaryMask) != 0))
                continue;
            var chunk = new byte[length];
            Buffer.BlockCopy(payload, start, chunk, 0, length);
            yield return chunk;
            start = index + 1;
            hash = 0;
        }
        if (start < payload.Length)
        {
            var chunk = new byte[payload.Length - start];
            Buffer.BlockCopy(payload, start, chunk, 0, chunk.Length);
            yield return chunk;
        }
    }

    private static ulong GearValue(byte value)
    {
        ulong state = value + 0x9E3779B97F4A7C15UL;
        state ^= state >> 30;
        state *= 0xBF58476D1CE4E5B9UL;
        state ^= state >> 27;
        state *= 0x94D049BB133111EBUL;
        return state ^ (state >> 31);
    }

    public static async Task RebuildAsync(
        string sourcePath,
        CancellationToken cancellationToken,
        bool requirePalLocalization = false)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(sourcePath);
        var decompressed = await File.ReadAllBytesAsync(sourcePath, cancellationToken);
        await RebuildAsync(decompressed, sourcePath, cancellationToken, requirePalLocalization);
    }

    public static async Task RebuildAsync(
        byte[] decompressed,
        string outputPath,
        CancellationToken cancellationToken,
        bool requirePalLocalization = false)
    {
        ArgumentNullException.ThrowIfNull(decompressed);
        ArgumentException.ThrowIfNullOrWhiteSpace(outputPath);

        var outputDirectory = Path.GetDirectoryName(outputPath)
            ?? throw new InvalidOperationException("The level output path has no directory.");
        Directory.CreateDirectory(outputDirectory);

        var temporaryPath = Path.Combine(
            outputDirectory,
            $".{Path.GetFileNameWithoutExtension(outputPath)}.{Guid.NewGuid():N}.brx");
        var rebuiltPath = Path.Combine(
            outputDirectory,
            Path.GetFileNameWithoutExtension(temporaryPath) + "_Rebuilt.brx");

        try
        {
            await File.WriteAllBytesAsync(temporaryPath, decompressed, cancellationToken);

            await RunRebuilderAsync(new[] { temporaryPath }, rebuiltPath, cancellationToken);
            await ValidatePcBrxAsync(rebuiltPath, requirePalLocalization,
                expectedVersion: 17, cancellationToken: cancellationToken);

            await WriteUnifiedContainerAsync(new[] { rebuiltPath }, outputPath,
                new uint[] { 0 }, defaultLanguage: 0,
                cancellationToken: cancellationToken);
        }
        finally
        {
            TryDelete(temporaryPath);
            TryDelete(rebuiltPath);
        }
    }

    private static async Task ValidatePcBrxAsync(
        string path,
        bool requirePalLocalization,
        uint expectedVersion,
        CancellationToken cancellationToken)
    {
        const uint pcBrxMagic = 0x52424350; // "PCBR" little-endian
        var header = new byte[8];

        await using var input = new FileStream(
            path,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read,
            bufferSize: 4096,
            useAsync: true);
        if (await input.ReadAsync(header, cancellationToken) != header.Length ||
            BitConverter.ToUInt32(header, 0) != pcBrxMagic ||
            BitConverter.ToUInt32(header, 4) != expectedVersion)
        {
            throw new InvalidDataException(
                $"The BRX rebuilder returned success but did not produce a PCBRX v{expectedVersion} file.");
        }

        if (requirePalLocalization)
        {
            const uint localizationMagic = 0x434F4C50u; // "PLOC"
            if (input.Length < 8)
                throw new InvalidDataException("The rebuilt PAL PCBRX has no localization footer.");

            input.Seek(-8, SeekOrigin.End);
            var footer = new byte[8];
            if (await input.ReadAsync(footer, cancellationToken) != footer.Length ||
                BitConverter.ToUInt32(footer, 0) != localizationMagic)
            {
                throw new InvalidDataException(
                    "The PAL PCBRX has no localization table. Rebuild Sly1BrxRebuilder and run extraction again.");
            }
        }
    }

    private static async Task ValidateJapaneseLocalizationParityAsync(
        IReadOnlyList<string> payloadPaths,
        CancellationToken cancellationToken)
    {
        if (payloadPaths.Count != 2)
            throw new InvalidDataException("Japanese extraction requires exactly two voice payloads.");

        var first = await ReadLocalizationPayloadAsync(payloadPaths[0], cancellationToken);
        var second = await ReadLocalizationPayloadAsync(payloadPaths[1], cancellationToken);
        if (!first.AsSpan().SequenceEqual(second))
        {
            throw new InvalidDataException(
                "Japanese voice slots contain different localization tables; refusing to build a package whose subtitles could change with the voice selector.");
        }
    }

    private static async Task ValidateJapaneseSourceLocalizationParityAsync(
        IReadOnlyList<string> sourcePaths,
        CancellationToken cancellationToken)
    {
        if (sourcePaths.Count != 2)
            throw new InvalidDataException("Japanese extraction requires exactly two voice sources.");

        var first = await ReadJapaneseSourceLocalizationTablesAsync(sourcePaths[0],
            cancellationToken);
        var second = await ReadJapaneseSourceLocalizationTablesAsync(sourcePaths[1],
            cancellationToken);
        if (!first.AsSpan().SequenceEqual(second))
        {
            throw new InvalidDataException(
                "Japanese voice slots contain different UI/subtitle reference tables; refusing to combine them as voice-only variants.");
        }
    }

    private static async Task<byte[]> ReadJapaneseSourceLocalizationTablesAsync(
        string path,
        CancellationToken cancellationToken)
    {
        var bytes = await File.ReadAllBytesAsync(path, cancellationToken);
        var offset = 8;
        if (bytes.Length < offset + 7)
            throw new InvalidDataException($"Japanese level '{path}' is truncated before localization data.");

        var debugInfo = bytes[offset++] != 0;
        offset += 2; // Sound-bank selector.
        var nameCount = ReadUInt32(bytes, ref offset, path);
        if (debugInfo && nameCount > 1)
        {
            for (uint index = 1; index < nameCount; ++index)
            {
                var byteCount = ReadUInt16(bytes, ref offset, path);
                Advance(bytes, ref offset, byteCount, path);
            }
        }

        var localizationStart = offset;
        for (var table = 0; table < 2; ++table)
        {
            var stringCount = ReadUInt16(bytes, ref offset, path);
            for (var index = 0; index < stringCount; ++index)
            {
                var characterCount = ReadUInt16(bytes, ref offset, path);
                Advance(bytes, ref offset, checked(characterCount * 2), path);
            }
        }
        return bytes.AsSpan(localizationStart, offset - localizationStart).ToArray();
    }

    private static ushort ReadUInt16(byte[] bytes, ref int offset, string path)
    {
        if (offset > bytes.Length - sizeof(ushort))
            throw new InvalidDataException($"Japanese level '{path}' has truncated localization data.");
        var value = BitConverter.ToUInt16(bytes, offset);
        offset += sizeof(ushort);
        return value;
    }

    private static uint ReadUInt32(byte[] bytes, ref int offset, string path)
    {
        if (offset > bytes.Length - sizeof(uint))
            throw new InvalidDataException($"Japanese level '{path}' has a truncated name table.");
        var value = BitConverter.ToUInt32(bytes, offset);
        offset += sizeof(uint);
        return value;
    }

    private static void Advance(byte[] bytes, ref int offset, int count, string path)
    {
        if (count < 0 || offset > bytes.Length - count)
            throw new InvalidDataException($"Japanese level '{path}' has an invalid localization table.");
        offset += count;
    }

    private static async Task<byte[]> ReadLocalizationPayloadAsync(
        string path,
        CancellationToken cancellationToken)
    {
        const uint localizationMagic = 0x434F4C50u; // "PLOC"
        var bytes = await File.ReadAllBytesAsync(path, cancellationToken);
        if (bytes.Length < 8 || BitConverter.ToUInt32(bytes, bytes.Length - 8) != localizationMagic)
            throw new InvalidDataException($"'{path}' has no localization footer.");
        var size = BitConverter.ToUInt32(bytes, bytes.Length - 4);
        if (size > (uint)(bytes.Length - 8))
            throw new InvalidDataException($"'{path}' has an invalid localization footer.");
        return bytes.AsSpan(bytes.Length - 8 - checked((int)size), checked((int)size)).ToArray();
    }

    private static async Task ValidateReferencedAudioAsync(
        string payloadPath,
        string sourceLanguagePath,
        CancellationToken cancellationToken)
    {
        var payload = await File.ReadAllBytesAsync(payloadPath, cancellationToken);
        var languageDirectory = Path.GetDirectoryName(sourceLanguagePath)
            ?? throw new InvalidDataException($"Language level '{sourceLanguagePath}' has no directory.");
        var dialogDirectory = Path.Combine(languageDirectory, "Dialogs");
        var commentaryDirectory = Path.Combine(languageDirectory, "Commentary");
        var references = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

        for (var offset = 0; offset + 4 <= payload.Length; ++offset)
        {
            if (payload[offset] != (byte)'V' || payload[offset + 1] != (byte)'A' ||
                payload[offset + 2] != (byte)'G' || payload[offset + 3] != (byte)'_')
                continue;

            var end = offset;
            var limit = Math.Min(payload.Length, offset + 32);
            while (end < limit && payload[end] != 0)
                ++end;
            if (end == limit)
                continue;
            var fileName = Encoding.ASCII.GetString(payload, offset, end - offset);
            if (fileName.EndsWith(".wav", StringComparison.OrdinalIgnoreCase))
                references.Add(fileName);
            offset = end;
        }

        foreach (var fileName in references)
        {
            if (!File.Exists(Path.Combine(dialogDirectory, fileName)) &&
                !File.Exists(Path.Combine(commentaryDirectory, fileName)))
            {
                throw new InvalidDataException(
                    $"Localized level '{sourceLanguagePath}' references missing audio '{fileName}'.");
            }
        }
    }

    private static async Task RunRebuilderAsync(
        IReadOnlyList<string> inputs,
        string rebuiltPath,
        CancellationToken cancellationToken,
        string? region = null,
        string? levelIdentityMapPath = null)
    {
        var startInfo = new ProcessStartInfo
        {
            FileName = FindRebuilder(),
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true
        };
        foreach (var input in inputs)
            startInfo.ArgumentList.Add(input);
        if (region is not null)
            startInfo.ArgumentList.Add(region);
        if (levelIdentityMapPath is not null)
            startInfo.ArgumentList.Add(levelIdentityMapPath);

        using var process = new Process { StartInfo = startInfo };
        var output = new StringBuilder();
        var errors = new StringBuilder();
        process.OutputDataReceived += (_, args) => AppendLine(output, args.Data);
        process.ErrorDataReceived += (_, args) => AppendLine(errors, args.Data);
        if (!process.Start())
            throw new InvalidOperationException("The BRX rebuilder could not be started.");
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();
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
        {
            var details = errors.Length != 0 ? errors.ToString() : output.ToString();
            throw new InvalidDataException($"Failed to rebuild level: {details.Trim()}");
        }
        if (!File.Exists(rebuiltPath))
            throw new InvalidDataException("The BRX rebuilder did not produce an output file.");
    }

    private static string FindRebuilder()
    {
        var executableName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? "Sly1BrxRebuilder.exe"
            : "Sly1BrxRebuilder";
        try
        {
            return BundledToolService.Extract(executableName);
        }
        catch (FileNotFoundException)
        {
            // Development builds made before the native helper exists can
            // still use a helper from the source/build tree.
        }
        var baseDirectory = AppContext.BaseDirectory;
        var candidates = new List<string>
        {
            Path.Combine(baseDirectory, executableName),
            Path.Combine(baseDirectory, "Tools", executableName)
        };

        var directory = new DirectoryInfo(baseDirectory);
        for (var depth = 0; depth < 6 && directory is not null; depth++, directory = directory.Parent)
        {
            candidates.Add(Path.Combine(directory.FullName, "bin", executableName));
            candidates.Add(Path.Combine(directory.FullName, "Tools", "BrxRebuilder", "x64", "Debug", executableName));
            candidates.Add(Path.Combine(directory.FullName, "Tools", "BrxRebuilder", "x64", "Release", executableName));
        }

        var newest = candidates.Where(File.Exists)
            .Select(path => new FileInfo(path))
            .OrderByDescending(file => file.LastWriteTimeUtc)
            .FirstOrDefault();
        if (newest is not null)
            return newest.FullName;

        throw new FileNotFoundException(
            $"The bundled BRX rebuilder ({executableName}) is missing. Rebuild the ProjectCane solution or reinstall the package.");
    }

    private static void AppendLine(StringBuilder builder, string? line)
    {
        if (line is not null)
            builder.AppendLine(line);
    }

    private static void TryDelete(string path)
    {
        try
        {
            if (File.Exists(path))
                File.Delete(path);
        }
        catch (IOException)
        {
        }
        catch (UnauthorizedAccessException)
        {
        }
    }
}

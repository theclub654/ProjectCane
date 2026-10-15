using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace File_Extractor_GUI.Services;

public sealed record Sly1ExtractionProgress(
    string CurrentItem,
    int CompletedFiles,
    int TotalFiles,
    long ProcessedBytes,
    long TotalBytes)
{
    public double Percentage => TotalBytes <= 0
        ? 0
        : Math.Clamp(ProcessedBytes * 100d / TotalBytes, 0, 100);
}

public sealed class Sly1ExtractionService
{
    private const int LevelCount = 46;
    private const int LevelInfoSize = 44;
    private const int SectorSize = 0x800;
    private const int ChunkSize = 0x2000;
    private const int MaximumMatchLength = 10;
    private const int MaximumLevelNameLength = 256;
    private const int MaximumCompressedLevelSize = 256 * 1024 * 1024;

    private const uint LevelTableVirtualAddress = 0x00247AF0;
    private const uint ElfSegmentVirtualAddress = 0x00100000;
    private const uint ElfSegmentFileOffset = 0x00001000;

    private static readonly long LevelTableExecutableOffset =
        LevelTableVirtualAddress - ElfSegmentVirtualAddress + ElfSegmentFileOffset;

    public async Task ExtractLevelsAsync(
        string isoPath,
        string outputFolder,
        long executableIsoOffset,
        IProgress<Sly1ExtractionProgress>? progress = null,
        CancellationToken cancellationToken = default)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(isoPath);
        ArgumentException.ThrowIfNullOrWhiteSpace(outputFolder);

        var worldsDirectory = Path.Combine(outputFolder, "Worlds");
        Directory.CreateDirectory(worldsDirectory);

        await using var iso = new FileStream(
            isoPath,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read,
            bufferSize: 128 * 1024,
            useAsync: true);

        var tableBytes = new byte[LevelCount * LevelInfoSize];
        await ReadExactlyAtAsync(
            iso,
            executableIsoOffset + LevelTableExecutableOffset,
            tableBytes,
            cancellationToken);

        var levels = new List<LevelEntry>(LevelCount);
        long totalCompressedBytes = 0;

        for (var index = 0; index < LevelCount; index++)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var entry = ParseLevelEntry(tableBytes.AsSpan(index * LevelInfoSize, LevelInfoSize));

            var sector = entry.EncryptedSector ^ entry.SectorKey;
            var compressedSize = entry.EncryptedSize ^ entry.SizeKey;

            if (compressedSize == 0 || compressedSize > MaximumCompressedLevelSize)
                throw new InvalidDataException($"Level {index} has an invalid compressed size: {compressedSize}.");

            var dataOffset = checked((long)sector * SectorSize);
            if (dataOffset < 0 || dataOffset > iso.Length || compressedSize > iso.Length - dataOffset)
                throw new InvalidDataException($"Level {index} points outside the ISO.");

            var levelName = await ReadExecutableStringAsync(
                iso,
                executableIsoOffset,
                entry.LevelNameVirtualAddress,
                cancellationToken);

            levelName = SanitizeFileName(string.IsNullOrWhiteSpace(levelName)
                ? $"Level_{index}"
                : levelName);

            if ((entry.LevelId & 0xFFFFU) == 0x0003U)
                levelName = "Paris";

            levels.Add(new LevelEntry(
                entry.LevelId,
                levelName,
                dataOffset,
                checked((int)compressedSize)));

            totalCompressedBytes = checked(totalCompressedBytes + compressedSize);
        }

        long processedBytes = 0;

        for (var index = 0; index < levels.Count; index++)
        {
            cancellationToken.ThrowIfCancellationRequested();
            var level = levels[index];

            progress?.Report(new Sly1ExtractionProgress(
                $"Worlds / {GetEpisodeName(level.LevelId)} / {level.Name}",
                index,
                levels.Count,
                processedBytes,
                totalCompressedBytes));

            var compressed = new byte[level.CompressedSize];
            await ReadExactlyAtAsync(iso, level.DataOffset, compressed, cancellationToken);
            var decompressed = Decompress(compressed, cancellationToken);

            var episodeDirectory = GetEpisodeDirectory(worldsDirectory, level.LevelId);
            var levelDirectory = Path.Combine(episodeDirectory, level.Name);
            Directory.CreateDirectory(levelDirectory);

            var outputPath = Path.Combine(levelDirectory, level.Name + ".brx");
            progress?.Report(new Sly1ExtractionProgress(
                $"Rebuilding Worlds / {GetEpisodeName(level.LevelId)} / {level.Name}",
                index,
                levels.Count,
                processedBytes,
                totalCompressedBytes));
            await BrxRebuildService.RebuildAsync(
                decompressed,
                outputPath,
                cancellationToken);

            processedBytes += level.CompressedSize;
            progress?.Report(new Sly1ExtractionProgress(
                $"Worlds / {GetEpisodeName(level.LevelId)} / {level.Name}",
                index + 1,
                levels.Count,
                processedBytes,
                totalCompressedBytes));
        }
    }

    private static ParsedLevelEntry ParseLevelEntry(ReadOnlySpan<byte> data) => new(
        ReadUInt32(data, 0),
        ReadUInt32(data, 4),
        ReadUInt32(data, 20),
        ReadUInt32(data, 28),
        ReadUInt32(data, 32),
        ReadUInt32(data, 36));

    private static uint ReadUInt32(ReadOnlySpan<byte> data, int offset) =>
        BinaryPrimitives.ReadUInt32LittleEndian(data.Slice(offset, 4));

    private static async Task<string?> ReadExecutableStringAsync(
        FileStream iso,
        long executableIsoOffset,
        uint virtualAddress,
        CancellationToken cancellationToken)
    {
        if (virtualAddress < ElfSegmentVirtualAddress)
            return null;

        var executableOffset =
            (long)(virtualAddress - ElfSegmentVirtualAddress) + ElfSegmentFileOffset;
        var isoOffset = executableIsoOffset + executableOffset;

        if (isoOffset < 0 || isoOffset >= iso.Length)
            return null;

        var bytes = new List<byte>(64);
        var oneByte = new byte[1];
        iso.Seek(isoOffset, SeekOrigin.Begin);

        for (var index = 0; index < MaximumLevelNameLength; index++)
        {
            if (await iso.ReadAsync(oneByte, cancellationToken) != 1)
                return null;

            if (oneByte[0] == 0)
                return Encoding.ASCII.GetString(bytes.ToArray());

            bytes.Add(oneByte[0]);
        }

        return null;
    }

    private static byte[] Decompress(ReadOnlySpan<byte> input, CancellationToken cancellationToken)
    {
        using var output = new MemoryStream(Math.Max(input.Length * 3, ChunkSize));
        var work = new byte[ChunkSize + MaximumMatchLength];
        var inputPosition = 0;
        var outputPosition = 0;
        uint flags = 0;

        while (inputPosition < input.Length)
        {
            cancellationToken.ThrowIfCancellationRequested();
            flags >>= 1;

            if ((flags & 0x100U) == 0)
            {
                if (inputPosition >= input.Length)
                    break;

                flags = 0xFF00U | input[inputPosition++];
            }

            if (inputPosition >= input.Length)
                break;

            var firstByte = input[inputPosition++];
            if ((flags & 1U) != 0)
            {
                work[outputPosition++] = firstByte;
            }
            else
            {
                if (inputPosition >= input.Length)
                    break;

                var secondByte = input[inputPosition++];
                var token = (ushort)(firstByte | (secondByte << 8));
                var sourcePosition = token & 0x1FFF;
                var copyLength = (token >> 13) + 3;

                for (var index = 0; index < copyLength; index++)
                    work[outputPosition++] = work[sourcePosition++ & 0x1FFF];
            }

            if (outputPosition < ChunkSize)
                continue;

            output.Write(work, 0, ChunkSize);
            var spillCount = outputPosition - ChunkSize;
            if (spillCount > 0)
                Buffer.BlockCopy(work, ChunkSize, work, 0, spillCount);

            outputPosition = spillCount;
        }

        output.Write(work, 0, outputPosition);
        return output.ToArray();
    }

    private static async Task ReadExactlyAtAsync(
        FileStream stream,
        long offset,
        byte[] destination,
        CancellationToken cancellationToken)
    {
        if (offset < 0 || offset > stream.Length || destination.Length > stream.Length - offset)
            throw new EndOfStreamException("The requested data is outside the ISO.");

        stream.Seek(offset, SeekOrigin.Begin);
        var totalRead = 0;
        while (totalRead < destination.Length)
        {
            var read = await stream.ReadAsync(destination.AsMemory(totalRead), cancellationToken);
            if (read == 0)
                throw new EndOfStreamException("Unexpected end of ISO.");

            totalRead += read;
        }
    }

    private static string SanitizeFileName(string name)
    {
        var characters = name.ToCharArray();
        for (var index = 0; index < characters.Length; index++)
        {
            if (IsUnsafeFileNameCharacter(characters[index]))
                characters[index] = '_';
        }

        var sanitized = new string(characters).TrimEnd(' ', '.');
        return string.IsNullOrWhiteSpace(sanitized) ? "Unnamed Level" : sanitized;
    }

    private static bool IsUnsafeFileNameCharacter(char character) =>
        character is '<' or '>' or ':' or '"' or '/' or '\\' or '|' or '?' or '*' ||
        char.IsControl(character);

    private static string GetEpisodeDirectory(string worldsDirectory, uint levelId) =>
        Path.Combine(worldsDirectory, GetEpisodeName(levelId));

    private static string GetEpisodeName(uint levelId) => (levelId >> 8) switch
    {
        1 => "Tide of Terror",
        2 => "Sunset Snake Eyes",
        3 => "Vicious Voodoo",
        4 => "Fire in the sky",
        5 => "The Cold Heart of Hate",
        _ => string.Empty
    };

    private readonly record struct ParsedLevelEntry(
        uint EncryptedSector,
        uint EncryptedSize,
        uint SectorKey,
        uint SizeKey,
        uint LevelId,
        uint LevelNameVirtualAddress);

    private readonly record struct LevelEntry(
        uint LevelId,
        string Name,
        long DataOffset,
        int CompressedSize);
}

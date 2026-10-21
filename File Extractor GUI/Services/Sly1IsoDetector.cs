using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace File_Extractor_GUI.Services;

public sealed record Sly1IsoDetectionResult(
    bool IsValid,
    string Message,
    string? Serial = null,
    long ExecutableOffset = 0,
    long ExecutableLength = 0,
    Sly1DiscRegion Region = Sly1DiscRegion.Unknown);

public enum Sly1DiscRegion
{
    Unknown,
    NtscU,
    Pal,
    Japanese
}

public static partial class Sly1IsoDetector
{
    private const int SectorSize = 2048;
    private const int PrimaryVolumeDescriptorSector = 16;
    private const string NtscSerial = "SCUS-97198";
    private const string NtscExecutable = "SCUS_971.98";
    private const string PalSerial = "SCES-50917";
    private const string PalExecutable = "SCES_509.17";
    private const string JapaneseSerial = "SCPS-15036";
    private const string JapaneseExecutable = "SCPS_150.36";

    public static async Task<Sly1IsoDetectionResult> DetectAsync(
        string isoPath,
        CancellationToken cancellationToken = default)
    {
        if (!File.Exists(isoPath))
            return Invalid("The selected ISO no longer exists.");

        if (!string.Equals(Path.GetExtension(isoPath), ".iso", StringComparison.OrdinalIgnoreCase))
            return Invalid("Select a PlayStation 2 ISO file.");

        try
        {
            await using var iso = new FileStream(
                isoPath,
                FileMode.Open,
                FileAccess.Read,
                FileShare.Read,
                bufferSize: 64 * 1024,
                useAsync: true);

            var volumeDescriptor = new byte[SectorSize];
            if (!await ReadExactlyAtAsync(
                    iso,
                    (long)PrimaryVolumeDescriptorSector * SectorSize,
                    volumeDescriptor,
                    cancellationToken))
            {
                return Invalid("The ISO is too small or could not be read.");
            }

            if (volumeDescriptor[0] != 1 ||
                Encoding.ASCII.GetString(volumeDescriptor, 1, 5) != "CD001" ||
                volumeDescriptor[6] != 1)
            {
                return Invalid("The image does not contain a valid ISO9660 filesystem.");
            }

            var root = ReadDirectoryRecord(volumeDescriptor, 156);
            if (root is null || !root.Value.IsDirectory)
                return Invalid("The ISO9660 root directory is invalid.");

            var rootBytes = new byte[root.Value.DataLength];
            if (!await ReadExactlyAtAsync(
                    iso,
                    (long)root.Value.ExtentSector * SectorSize,
                    rootBytes,
                    cancellationToken))
            {
                return Invalid("The ISO9660 root directory could not be read.");
            }

            var entries = ReadDirectory(rootBytes);
            var systemCnf = FindFile(entries, "SYSTEM.CNF");
            if (systemCnf is null)
                return Invalid("SYSTEM.CNF was not found in the ISO.");

            var systemCnfBytes = new byte[systemCnf.Value.DataLength];
            if (!await ReadExactlyAtAsync(
                    iso,
                    (long)systemCnf.Value.ExtentSector * SectorSize,
                    systemCnfBytes,
                    cancellationToken))
            {
                return Invalid("SYSTEM.CNF could not be read.");
            }

            var systemCnfText = Encoding.ASCII.GetString(systemCnfBytes);
            var bootMatch = BootExecutableRegex().Match(systemCnfText);
            if (!bootMatch.Success)
                return Invalid("SYSTEM.CNF does not contain a valid BOOT2 entry.");

            var bootExecutable = NormalizeIsoName(bootMatch.Groups[1].Value);
            var region = bootExecutable.ToUpperInvariant() switch
            {
                NtscExecutable => Sly1DiscRegion.NtscU,
                PalExecutable => Sly1DiscRegion.Pal,
                JapaneseExecutable => Sly1DiscRegion.Japanese,
                _ => Sly1DiscRegion.Unknown
            };
            if (region == Sly1DiscRegion.Unknown)
            {
                return Invalid($"Unsupported PlayStation 2 build ({bootExecutable}).");
            }

            var executable = FindFile(entries, bootExecutable);
            if (executable is null)
                return Invalid($"{bootExecutable} was not found in the ISO root.");

            var executableOffset = (long)executable.Value.ExtentSector * SectorSize;
            var elfMagic = new byte[4];
            if (!await ReadExactlyAtAsync(iso, executableOffset, elfMagic, cancellationToken) ||
                elfMagic[0] != 0x7F ||
                elfMagic[1] != (byte)'E' ||
                elfMagic[2] != (byte)'L' ||
                elfMagic[3] != (byte)'F')
            {
                return Invalid($"{bootExecutable} is not a valid ELF executable.");
            }

            var (message, serial) = region switch
            {
                Sly1DiscRegion.Pal => ("Sly 1 PAL detected.", PalSerial),
                Sly1DiscRegion.Japanese => ("Kaitou Sly Cooper Japanese release detected.", JapaneseSerial),
                _ => ("Sly 1 NTSC-U detected.", NtscSerial)
            };
            return new Sly1IsoDetectionResult(
                true,
                message,
                serial,
                executableOffset,
                executable.Value.DataLength,
                region);
        }
        catch (OperationCanceledException)
        {
            throw;
        }
        catch (Exception exception) when (
            exception is IOException or UnauthorizedAccessException or ArgumentException)
        {
            return Invalid("The ISO could not be opened or read.");
        }
    }

    private static Sly1IsoDetectionResult Invalid(string message) => new(false, message);

    private static IsoDirectoryRecord? FindFile(
        IReadOnlyList<IsoDirectoryRecord> entries,
        string expectedName) =>
        entries.FirstOrDefault(entry =>
            !entry.IsDirectory &&
            string.Equals(entry.Name, expectedName, StringComparison.OrdinalIgnoreCase));

    private static List<IsoDirectoryRecord> ReadDirectory(byte[] directoryData)
    {
        var entries = new List<IsoDirectoryRecord>();
        var offset = 0;

        while (offset < directoryData.Length)
        {
            var recordLength = directoryData[offset];
            if (recordLength == 0)
            {
                offset = ((offset / SectorSize) + 1) * SectorSize;
                continue;
            }

            if (offset + recordLength > directoryData.Length)
                break;

            var record = ReadDirectoryRecord(directoryData, offset);
            if (record is not null && record.Value.Name is not "." and not "..")
                entries.Add(record.Value);

            offset += recordLength;
        }

        return entries;
    }

    private static IsoDirectoryRecord? ReadDirectoryRecord(byte[] data, int offset)
    {
        if (offset < 0 || offset + 34 > data.Length)
            return null;

        var recordLength = data[offset];
        var nameLength = data[offset + 32];
        if (recordLength < 34 || offset + recordLength > data.Length || offset + 33 + nameLength > data.Length)
            return null;

        var extent = BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(offset + 2, 4));
        var length = BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(offset + 10, 4));
        var flags = data[offset + 25];

        string name;
        if (nameLength == 1 && data[offset + 33] == 0)
            name = ".";
        else if (nameLength == 1 && data[offset + 33] == 1)
            name = "..";
        else
            name = NormalizeIsoName(Encoding.ASCII.GetString(data, offset + 33, nameLength));

        return new IsoDirectoryRecord(extent, checked((int)length), (flags & 0x02) != 0, name);
    }

    private static string NormalizeIsoName(string name)
    {
        var normalized = name.Trim().Replace('/', '\\');
        var lastSeparator = normalized.LastIndexOf('\\');
        if (lastSeparator >= 0)
            normalized = normalized[(lastSeparator + 1)..];

        var versionSeparator = normalized.IndexOf(';');
        if (versionSeparator >= 0)
            normalized = normalized[..versionSeparator];

        return normalized.Trim();
    }

    private static async Task<bool> ReadExactlyAtAsync(
        FileStream stream,
        long offset,
        byte[] destination,
        CancellationToken cancellationToken)
    {
        if (offset < 0 || offset > stream.Length || destination.Length > stream.Length - offset)
            return false;

        stream.Seek(offset, SeekOrigin.Begin);
        var totalRead = 0;

        while (totalRead < destination.Length)
        {
            var read = await stream.ReadAsync(
                destination.AsMemory(totalRead),
                cancellationToken);

            if (read == 0)
                return false;

            totalRead += read;
        }

        return true;
    }

    [GeneratedRegex(@"(?im)^\s*BOOT2\s*=\s*cdrom0:\\+([^;\r\n]+)(?:;\d+)?\s*$")]
    private static partial Regex BootExecutableRegex();

    private readonly record struct IsoDirectoryRecord(
        uint ExtentSector,
        int DataLength,
        bool IsDirectory,
        string Name);
}

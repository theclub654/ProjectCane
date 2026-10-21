using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;

namespace File_Extractor_GUI.Services;

internal static class BundledToolService
{
    public static string Extract(string toolName)
    {
        var assembly = Assembly.GetExecutingAssembly();
        var resourceName = $"ProjectCane.FileExtractor.Tools.{toolName}";
        using var resource = assembly.GetManifestResourceStream(resourceName)
            ?? throw new FileNotFoundException($"The embedded tool '{toolName}' is missing from this File Extractor build.");

        using var memory = new MemoryStream();
        resource.CopyTo(memory);
        var bytes = memory.ToArray();
        var hash = Convert.ToHexString(SHA256.HashData(bytes)).Substring(0, 16);
        var toolDirectory = Path.Combine(Path.GetTempPath(), "ProjectCane", "FileExtractorTools", hash);
        var toolPath = Path.Combine(toolDirectory, toolName);

        Directory.CreateDirectory(toolDirectory);
        if (!File.Exists(toolPath) || new FileInfo(toolPath).Length != bytes.Length)
            File.WriteAllBytes(toolPath, bytes);

        if (!RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            File.SetUnixFileMode(toolPath,
                UnixFileMode.UserRead | UnixFileMode.UserWrite | UnixFileMode.UserExecute |
                UnixFileMode.GroupRead | UnixFileMode.GroupExecute |
                UnixFileMode.OtherRead | UnixFileMode.OtherExecute);
        }

        return toolPath;
    }
}

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace File_Extractor_GUI.Services;

internal static class BrxRebuildService
{
    public static async Task RebuildAsync(
        byte[] decompressed,
        string outputPath,
        CancellationToken cancellationToken)
    {
        ArgumentNullException.ThrowIfNull(decompressed);
        ArgumentException.ThrowIfNullOrWhiteSpace(outputPath);

        var rebuilderPath = FindRebuilder();
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

            var startInfo = new ProcessStartInfo
            {
                FileName = rebuilderPath,
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true
            };
            startInfo.ArgumentList.Add(temporaryPath);

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
                throw new InvalidDataException(
                    $"Failed to rebuild {Path.GetFileName(outputPath)}: {details.Trim()}");
            }

            if (!File.Exists(rebuiltPath))
                throw new InvalidDataException("The BRX rebuilder did not produce an output file.");

            File.Move(rebuiltPath, outputPath, overwrite: true);
        }
        finally
        {
            TryDelete(temporaryPath);
            TryDelete(rebuiltPath);
        }
    }

    private static string FindRebuilder()
    {
        var executableName = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? "Sly1BrxRebuilder.exe"
            : "Sly1BrxRebuilder";
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

        foreach (var candidate in candidates)
        {
            if (File.Exists(candidate))
                return candidate;
        }

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

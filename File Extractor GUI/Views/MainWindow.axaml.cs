using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Platform.Storage;
using File_Extractor_GUI.Services;
using System;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace File_Extractor_GUI.Views;

public partial class MainWindow : Window
{
    private string? _selectedIsoPath;
    private string? _outputFolderPath;
    private bool _isIsoValid;
    private Sly1IsoDetectionResult? _isoDetection;
    private CancellationTokenSource? _extractionCancellation;

    public MainWindow()
    {
        InitializeComponent();

        if (System.Runtime.InteropServices.RuntimeInformation.IsOSPlatform(System.Runtime.InteropServices.OSPlatform.OSX))
        {
            // The game looks for extracted data in Application Support first.
            var home = Environment.GetEnvironmentVariable("HOME");
            if (string.IsNullOrEmpty(home))
                home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
            _outputFolderPath = Path.Combine(home, "Library", "Application Support", "ProjectCane");
            OutputFolderPathText.Text = _outputFolderPath;
            OutputFolderPathText.Foreground = new SolidColorBrush(Color.Parse("#D5E2EE"));
            ChangeFolderButton.Content = "Change Folder";
        }
    }

    private void TitleBar_PointerPressed(object? sender, PointerPressedEventArgs e)
    {
        if (e.GetCurrentPoint(this).Properties.IsLeftButtonPressed)
            BeginMoveDrag(e);
    }

    private void Minimize_Click(object? sender, RoutedEventArgs e) =>
        WindowState = WindowState.Minimized;

    private void Close_Click(object? sender, RoutedEventArgs e)
    {
        _extractionCancellation?.Cancel();
        Close();
    }

    private async void BrowseIso_Click(object? sender, RoutedEventArgs e)
    {
        var files = await StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions
        {
            Title = "Select Sly 1 ISO",
            AllowMultiple = false,
            FileTypeFilter =
            [
                new FilePickerFileType("PlayStation 2 ISO")
                {
                    Patterns = ["*.iso"]
                }
            ]
        });

        var selectedFile = files.FirstOrDefault();
        if (selectedFile is null)
            return;

        var localPath = selectedFile.Path.LocalPath;
        _selectedIsoPath = localPath;
        SelectedIsoNameText.Text = Path.GetFileName(localPath);
        SelectedIsoPathText.Text = localPath;
        DetectionBadge.IsVisible = true;
        DropPromptText.Text = "Checking selected ISO...";
        DropSubtext.Text = "Looking for a supported Sly 1 executable";

        var detection = await Sly1IsoDetector.DetectAsync(localPath);
        ShowDetectionResult(detection);
    }

    private async void ChooseOutputFolder_Click(object? sender, RoutedEventArgs e)
    {
        var folders = await StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions
        {
            Title = "Choose ProjectCane output folder",
            AllowMultiple = false
        });

        var selectedFolder = folders.FirstOrDefault();
        if (selectedFolder is null)
            return;

        var localPath = selectedFolder.Path.LocalPath;
        if (string.IsNullOrWhiteSpace(localPath) || !Directory.Exists(localPath))
            return;

        _outputFolderPath = localPath;
        OutputFolderPathText.Text = localPath;
        OutputFolderPathText.Foreground = new SolidColorBrush(Color.Parse("#D5E2EE"));
        ChangeFolderButton.Content = "Change Folder";
        UpdateWorkflowState();
    }

    private void ShowDetectionResult(Sly1IsoDetectionResult detection)
    {
        if (detection.IsValid)
        {
            _isIsoValid = true;
            _isoDetection = detection;
            DetectionBadge.Background = new SolidColorBrush(Color.Parse("#082721"));
            DetectionBadge.BorderBrush = new SolidColorBrush(Color.Parse("#176052"));
            DetectionIcon.Background = new SolidColorBrush(Color.Parse("#42D9AF"));
            DetectionIconText.Text = "✓";
            DetectedGameText.Foreground = new SolidColorBrush(Color.Parse("#57DDBA"));
            DetectedGameText.Text = "Sly 1 detected";
            var region = detection.Region switch
            {
                Sly1DiscRegion.Pal => "PAL",
                Sly1DiscRegion.Japanese => "NTSC-J",
                _ => "NTSC-U"
            };
            DetectedSerialText.Text = $"{region} • {detection.Serial}";
            DropPromptText.Text = $"Sly 1 {region} ISO ready";
            DropSubtext.Text = "Choose an output folder to continue";
            UpdateWorkflowState();
            return;
        }

        _isIsoValid = false;
        _isoDetection = null;
        DetectionBadge.Background = new SolidColorBrush(Color.Parse("#2A1118"));
        DetectionBadge.BorderBrush = new SolidColorBrush(Color.Parse("#7B2E3B"));
        DetectionIcon.Background = new SolidColorBrush(Color.Parse("#D85A68"));
        DetectionIconText.Text = "×";
        DetectedGameText.Foreground = new SolidColorBrush(Color.Parse("#F0808A"));
        DetectedGameText.Text = "Unsupported ISO";
        DetectedSerialText.Text = detection.Message;
        DropPromptText.Text = "That ISO is not supported";
        DropSubtext.Text = "Select a supported NTSC-U or PAL Sly 1 disc image";
        UpdateWorkflowState();
    }

    private async void ExtractFiles_Click(object? sender, RoutedEventArgs e)
    {
        if (!_isIsoValid ||
            _isoDetection is null ||
            string.IsNullOrWhiteSpace(_selectedIsoPath) ||
            string.IsNullOrWhiteSpace(_outputFolderPath))
        {
            return;
        }

        _extractionCancellation?.Dispose();
        _extractionCancellation = new CancellationTokenSource();
        var cancellationToken = _extractionCancellation.Token;

        BrowseIsoButton.IsEnabled = false;
        ChangeFolderButton.IsEnabled = false;
        ExtractButton.IsEnabled = false;
        ExtractButtonText.Text = "Extracting...";
        ProgressPanel.IsVisible = true;
        ProgressStatusText.Text = "Preparing level extraction...";
        ExtractionProgressBar.Value = 0;
        ProgressPercentText.Text = "0%";
        CurrentItemText.Text = "Current item: Reading level table";
        FileCountText.Text = _isoDetection.Region switch
        {
            Sly1DiscRegion.Pal => "0 / 276 tasks",
            Sly1DiscRegion.Japanese => "0 / 138 tasks",
            _ => "0 / 46 files"
        };
        ByteCountText.Text = "0 MB / 0 MB";
        SetStepComplete(Step1Circle, Step1Number, Step1Title, Step1Description);
        SetStepComplete(Step2Circle, Step2Number, Step2Title, Step2Description);
        SetStepActive(Step3Circle, Step3Number, Step3Title, Step3Description, "3");

        var progress = new Progress<Sly1ExtractionProgress>(UpdateExtractionProgress);
        try
        {
            await Task.Run(
                async () =>
                {
                    if (_isoDetection.Region is Sly1DiscRegion.Pal or Sly1DiscRegion.Japanese)
                    {
                        await new Sly1PalExtractionService().ExtractAsync(
                            _selectedIsoPath,
                            _outputFolderPath,
                            _isoDetection.Region,
                            progress,
                            cancellationToken);
                    }
                    else
                    {
                        await new Sly1ExtractionService().ExtractLevelsAsync(
                            _selectedIsoPath,
                            _outputFolderPath,
                            _isoDetection.ExecutableOffset,
                            progress,
                            cancellationToken);

                        await new Sly1MediaExtractionService().ExtractAsync(
                            _selectedIsoPath,
                            _outputFolderPath,
                            _isoDetection.ExecutableOffset,
                            progress,
                            cancellationToken);
                    }

                    await ExtractionManifestService.WriteAsync(
                        _outputFolderPath,
                        _isoDetection.Region,
                        cancellationToken);
                },
                cancellationToken);

            ExtractionProgressBar.Value = 100;
            ProgressPercentText.Text = "100%";
            ProgressStatusText.Text = "Extraction complete";
            CurrentItemText.Text = _isoDetection.Region switch
            {
                Sly1DiscRegion.Pal => "Extracted all five PAL languages, audio, and cutscenes",
                Sly1DiscRegion.Japanese => "Extracted Japanese text, both voice tracks, audio, and cutscenes",
                _ => "Extracted Sly 1 levels, audio, and cutscenes"
            };
            ExtractButtonText.Text = "Extraction Complete";
            SetStepComplete(Step3Circle, Step3Number, Step3Title, Step3Description);
        }
        catch (OperationCanceledException)
        {
            ProgressStatusText.Text = "Extraction cancelled";
            ExtractButtonText.Text = "Extract Files";
        }
        catch (Exception exception)
        {
            ProgressStatusText.Text = "Extraction failed";
            CurrentItemText.Text = exception.Message;
            CurrentItemText.Foreground = new SolidColorBrush(Color.Parse("#F0808A"));
            ExtractButtonText.Text = "Try Again";
        }
        finally
        {
            BrowseIsoButton.IsEnabled = true;
            ChangeFolderButton.IsEnabled = true;

            if (!cancellationToken.IsCancellationRequested)
                ExtractButton.IsEnabled = true;
        }
    }

    private void UpdateExtractionProgress(Sly1ExtractionProgress progress)
    {
        ExtractionProgressBar.Value = progress.Percentage;
        ProgressPercentText.Text = $"{progress.Percentage:0}%";
        ProgressStatusText.Text = "Extracting Sly 1 game files...";
        CurrentItemText.Text = $"Current item: {progress.CurrentItem}";
        CurrentItemText.Foreground = new SolidColorBrush(Color.Parse("#829BB0"));
        FileCountText.Text = $"{progress.CompletedFiles} / {progress.TotalFiles} files";
        ByteCountText.Text = progress.TotalBytes == progress.TotalFiles
            ? string.Empty
            : $"{FormatBytes(progress.ProcessedBytes)} / {FormatBytes(progress.TotalBytes)}";
    }

    private static string FormatBytes(long byteCount)
    {
        const double megabyte = 1024d * 1024d;
        const double gigabyte = megabyte * 1024d;

        return byteCount >= gigabyte
            ? $"{byteCount / gigabyte:0.00} GB"
            : $"{byteCount / megabyte:0.0} MB";
    }

    private void UpdateWorkflowState()
    {
        if (!_isIsoValid)
        {
            SetStepActive(Step1Circle, Step1Number, Step1Title, Step1Description, "1");
            SetStepInactive(Step2Circle, Step2Number, Step2Title, Step2Description, "2");
            SetStepInactive(Step3Circle, Step3Number, Step3Title, Step3Description, "3");
            ExtractButton.IsEnabled = false;
            return;
        }

        SetStepComplete(Step1Circle, Step1Number, Step1Title, Step1Description);

        if (string.IsNullOrWhiteSpace(_outputFolderPath))
        {
            SetStepActive(Step2Circle, Step2Number, Step2Title, Step2Description, "2");
            SetStepInactive(Step3Circle, Step3Number, Step3Title, Step3Description, "3");
            ExtractButton.IsEnabled = false;
            return;
        }

        SetStepComplete(Step2Circle, Step2Number, Step2Title, Step2Description);
        SetStepActive(Step3Circle, Step3Number, Step3Title, Step3Description, "3");
        ExtractButton.IsEnabled = true;
    }

    private static void SetStepActive(
        Border circle,
        TextBlock number,
        TextBlock title,
        TextBlock description,
        string stepNumber)
    {
        circle.Background = new SolidColorBrush(Color.Parse("#0B2436"));
        circle.BorderBrush = new SolidColorBrush(Color.Parse("#31B5E6"));
        number.Text = stepNumber;
        number.Foreground = new SolidColorBrush(Color.Parse("#ECF3FA"));
        title.Foreground = new SolidColorBrush(Color.Parse("#ECF3FA"));
        description.Foreground = new SolidColorBrush(Color.Parse("#7991A7"));
    }

    private static void SetStepComplete(
        Border circle,
        TextBlock number,
        TextBlock title,
        TextBlock description)
    {
        circle.Background = new SolidColorBrush(Color.Parse("#082721"));
        circle.BorderBrush = new SolidColorBrush(Color.Parse("#35CFA8"));
        number.Text = "✓";
        number.Foreground = new SolidColorBrush(Color.Parse("#57DDBA"));
        title.Foreground = new SolidColorBrush(Color.Parse("#A9C7BE"));
        description.Foreground = new SolidColorBrush(Color.Parse("#66857C"));
    }

    private static void SetStepInactive(
        Border circle,
        TextBlock number,
        TextBlock title,
        TextBlock description,
        string stepNumber)
    {
        circle.Background = new SolidColorBrush(Color.Parse("#081522"));
        circle.BorderBrush = new SolidColorBrush(Color.Parse("#314963"));
        number.Text = stepNumber;
        number.Foreground = new SolidColorBrush(Color.Parse("#7E96AF"));
        title.Foreground = new SolidColorBrush(Color.Parse("#89A2BA"));
        description.Foreground = new SolidColorBrush(Color.Parse("#60798F"));
    }
}

using CommunityToolkit.Mvvm.ComponentModel;

namespace File_Extractor_GUI.ViewModels;

public partial class MainViewModel : ViewModelBase
{
    [ObservableProperty]
    private string _greeting = "Welcome to Avalonia!";
}

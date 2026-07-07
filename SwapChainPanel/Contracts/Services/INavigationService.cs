using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Navigation;

namespace SwapChainPanel.Contracts.Services
{
    internal interface INavigationService
    {
        event NavigatedEventHandler Navigated;

        bool CanGoBack { get; }

        Frame? Frame { get; set; }

        bool NavigateTo(string pageKey, object? parameter = null, bool clearNavigation = false);

        bool GoBack();
    }
}

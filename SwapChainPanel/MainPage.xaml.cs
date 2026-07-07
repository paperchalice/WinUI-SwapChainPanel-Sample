using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using SwapChainPanel.Contracts.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace SwapChainPanel
{

    internal enum NotifyType
    {
        StatusMessage,
        ErrorMessage
    };

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    internal sealed partial class MainPage : Page
    {
        private Frame HiddenFrame = new() {
            Visibility = Visibility.Collapsed,
        };

        public MainPage()
        {
            InitializeComponent();

            // This frame is hidden, meaning it is never shown.  It is simply used to load
            // each scenario page and then pluck out the input and output sections and
            // place them into the UserControls on the main page.
            ContentRoot.Children.Add(HiddenFrame);

            SetFeatureName(FEATURE_NAME);
            Scenarios.SelectionChanged += Scenarios_SelectionChanged;
        }

        private async void HyperlinkButton_Click(object sender, RoutedEventArgs e)
        {
            await Windows.System.Launcher.LaunchUriAsync(new Uri(((HyperlinkButton)sender).Tag.ToString()!));
        }

        /// <summary>
        /// This method is responsible for loading the individual input and output sections for each scenario.  This 
        /// is based on navigating a hidden Frame to the ScenarioX.xaml page and then extracting out the input
        /// and output sections into the respective UserControl on the main page.
        /// </summary>
        /// <param name="scenarioName"></param>
        public void LoadScenario(Type scenarioClass)
        {

            // Load the ScenarioX.xaml file into the Frame.
            HiddenFrame.Navigate(scenarioClass, this);

            // Get the top element, the Page, so we can look up the elements
            // that represent the input and output sections of the ScenarioX file.
            var hiddenPage = (Page)HiddenFrame.Content;

            // Get each element.
            UIElement? input = hiddenPage.FindName("Input") as UIElement;
            UIElement? output = hiddenPage.FindName("Output") as UIElement;


            // Find the LayoutRoot which parents the input and output sections in the main page.
            Panel? panel = hiddenPage.FindName("LayoutRoot") as Panel;

            if (panel != null)
            {
                // Get rid of the content that is currently in the intput and output sections.
                panel.Children.Remove(input);
                panel.Children.Remove(output);

                // Populate the input and output sections with the newly loaded content.
                InputSection.Content = input;
                OutputSection.Content = output;
            }
        }

        void Scenarios_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (Scenarios.SelectedItem != null)
            {
                NotifyUser("", NotifyType.StatusMessage);

                ListBoxItem selectedListBoxItem = (ListBoxItem)Scenarios.SelectedItem;
                Scenario scenario = (Scenario)selectedListBoxItem.Content;
                LoadScenario(scenario.ClassType);
            }
        }

        private void SetFeatureName(string str)
        {
            FeatureName.Text = str;
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            PopulateScenarios();
            // InvalidateLayout();
        }

        public void NotifyUser(string strMessage, NotifyType type)
        {
            switch (type)
            {
                // Use the status message style.
                case NotifyType.StatusMessage:
                    StatusBlock.Style = Resources["StatusStyle"] as Style;
                    break;
                // Use the error message style.
                case NotifyType.ErrorMessage:
                    StatusBlock.Style = Resources["ErrorStyle"] as Style;
                    break;
            }
            StatusBlock.Text = strMessage;

            // Collapse the StatusBlock if it has no text to conserve real estate.
            if (StatusBlock.Text != string.Empty)
            {
                StatusBlock.Visibility = Visibility.Visible;
            }
            else
            {
                StatusBlock.Visibility = Visibility.Collapsed;
            }
        }

        private void PopulateScenarios()
        {
            ObservableCollection<object> ScenarioList = [];
            // Populate the ListBox with the list of scenarios as defined in Constants.cs.
            foreach (var (s, i) in scenarios.Select((val, idx) => (val, idx)))
            {
                s.Title = $"{i+1}) {s}";
                ListBoxItem item = new()
                {
                    Content = s,
                    Name = s.ClassType.FullName
                };
                ScenarioList.Add(item);
            }

            // Bind the ListBox to the scenario list.
            Scenarios.ItemsSource = ScenarioList;

            // Starting scenario is the first or based upon a previous selection.
            int startingScenarioIndex = -1;

            Scenarios.SelectedIndex = startingScenarioIndex != -1 ? startingScenarioIndex : 0;
            Scenarios.ScrollIntoView(Scenarios.SelectedItem);
        }
    }
}

using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace SwapChainPanel
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario4 : Page
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;

        public Scenario4()
        {
            InitializeComponent();

            rootPage.SizeChanged += MainPage_SizeChanged;
            ParagraphText.SizeChanged += ParagraphText_SizeChanged;
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            UpdateOutputLayout();
            HighlighterPanel.StartProcessingInput();
        }

        /// <summary>
        /// Invoked when this page will no longer be displayed in a Frame.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            HighlighterPanel.StopProcessingInput();
        }

        /// <summary>
        /// Event handler for the main page SizeChanged event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MainPage_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            UpdateOutputLayout();
        }

        /// <summary>
        /// Updates size and position of elements on the page when the size changes.
        /// </summary>
        private void UpdateOutputLayout()
        {
            Output.Width = ((FrameworkElement)MainPage.Current.FindName("ContentRoot")).ActualWidth - (ParagraphBorder.BorderThickness.Left + ParagraphBorder.BorderThickness.Right);
        }

        /// <summary>
        /// Event handler for the ParagraphText SizeChanged event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void ParagraphText_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            HighlighterPanel.Width = e.NewSize.Width / 2;
            HighlighterPanel.Height = e.NewSize.Height / 2;
        }
    }
}

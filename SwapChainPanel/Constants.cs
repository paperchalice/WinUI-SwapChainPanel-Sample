using System;
using System.Collections.Generic;
using System.Text;

namespace SwapChainPanel
{
    public class Scenario
    {
        public string Title { get; set; } = string.Empty;

        public required Type ClassType { get; set; }

        public override string ToString()
        {
            return Title;
        }
    }

    internal partial class MainPage
    {
        // Change the string below to reflect the name of your sample.
        // This is used on the main page as the title of the sample.
        public const string FEATURE_NAME = "DirectX interop with SwapChainPanel";

        // Change the array below to reflect the name of your scenarios.
        // This will be used to populate the list of scenarios on the main page with
        // which the user will choose the specific scenario that they are interested in.
        // These should be in the form: "Navigating to a web page".
        // The code in MainPage will take care of turning this into: "1) Navigating to a web page"
        readonly List<Scenario> scenarios =
        [
            new Scenario() { Title = "SwapChainPanel", ClassType = typeof(Scenario1) },
            //new Scenario() { Title = "Independent Input", ClassType = typeof(Scenario2) },
            new() { Title = "Handling scale changes", ClassType = typeof(Scenario3) },
            new Scenario() { Title = "Highlighting using CompositeMode", ClassType = typeof(Scenario4) },
            //new Scenario() { Title = "Accessibility: UI Automation", ClassType = typeof(Scenario5) }
        ];
    }
}

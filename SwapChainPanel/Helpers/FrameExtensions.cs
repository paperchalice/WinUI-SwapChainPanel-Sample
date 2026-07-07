using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.UI.Xaml.Controls;

namespace SwapChainPanel.Helpers
{
    public static class FrameExtensions
    {
        extension(Frame frame)
        {
            public object? PageViewModel =>
                frame.Content?.GetType().GetProperty("ViewModel")?.GetValue(frame.Content, null);
        }
    }
}

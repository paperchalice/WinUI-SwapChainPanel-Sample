using System;
using System.Collections.Generic;
using System.Text;

namespace SwapChainPanel.Contracts.Views
{
    internal interface INavigationAware
    {
        void OnNavigatedTo(object parameter);

        void OnNavigatedFrom();
    }
}

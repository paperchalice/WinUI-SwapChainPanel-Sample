using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SwapChainPanel.Contracts.Services
{
    internal interface IPageService
    {
        Type GetPageType(string key);
    }
}

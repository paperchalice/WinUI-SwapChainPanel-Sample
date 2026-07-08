#pragma once

// Workaround from https://github.com/microsoft/cppwinrt/blob/master/nuget/modules.md

#define WINRT_IMPORT_MODULE

// Import the C++/WinRT namespaces used across this project
import winrt.Windows.Foundation;
import winrt.Windows.Foundation.Collections;
import winrt.Windows.System;
import winrt.Microsoft.UI.Xaml.Markup;
// ... other namespaces your project needs

// Component modules
import winrt.DirectXPanels;

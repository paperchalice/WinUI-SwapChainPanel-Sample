#include "pch.h"

#define WINRT_IMPORT_MODULE
import winrt.DirectXPanels;
import winrt.Microsoft.UI.Xaml;

// clang-format off
#include "RecognitionResultUpdatedEventArgs.h"
#include "RecognitionResultUpdatedEventArgs.g.cpp"
// clang-format on

namespace winrt::DirectXPanels::implementation
{
com_array<hstring> RecognitionResultUpdatedEventArgs::Results()
{
    return com_array<hstring>{m_results.begin(), m_results.end()};
}
void RecognitionResultUpdatedEventArgs::Results(array_view<hstring const> value)
{
    if (value.size() == m_results.size())
    {
        std::ranges::copy(value, m_results.begin());
        return;
    }
    com_array<hstring> results{value.begin(), value.end()};
    m_results = std::move(results);
}
} // namespace winrt::DirectXPanels::implementation

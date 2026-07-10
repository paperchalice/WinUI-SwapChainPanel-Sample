#pragma once
#include "RecognitionResultUpdatedEventArgs.g.h"

namespace winrt::DirectXPanels::implementation
{
    struct RecognitionResultUpdatedEventArgs : RecognitionResultUpdatedEventArgsT<RecognitionResultUpdatedEventArgs>
    {
        RecognitionResultUpdatedEventArgs() = default;

        com_array<hstring> Results();
        void Results(array_view<hstring const> value);

      private:
        com_array<hstring> m_results;
    };
}
namespace winrt::DirectXPanels::factory_implementation
{
    struct RecognitionResultUpdatedEventArgs : RecognitionResultUpdatedEventArgsT<RecognitionResultUpdatedEventArgs, implementation::RecognitionResultUpdatedEventArgs>
    {
    };
}

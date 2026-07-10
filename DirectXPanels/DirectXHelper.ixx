module;
#include "pch.h"

#include <d2d1.h>

export module DX:DirectXHelper;

import std;
import std.compat;

import winrt.Windows.ApplicationModel;
import winrt.Windows.Foundation;
import winrt.Windows.Storage;
import winrt.Windows.UI;

using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace concurrency;

export namespace DX
{
// Helper class for COM exceptions
class com_exception : public std::exception
{
  public:
    com_exception(HRESULT hr) : result(hr)
    {
    }

    const char *what() const noexcept override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
        return s_str;
    }

  private:
    HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw com_exception(hr);
    }
}

// Converts between Color types.
inline D2D1_COLOR_F ConvertToColorF(Windows::UI::Color color)
{
    return D2D1::ColorF(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f);
}

// Converts between Point types.
inline D2D1_POINT_2F ConvertToPoint2F(Windows::Foundation::Point point)
{
    return D2D1::Point2F(point.X, point.Y);
}

// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
inline float ConvertDipsToPixels(float dips)
{
    static const float dipsPerInch = 96.0f;
    return floorf(dips * Windows::Graphics::Display::DisplayInformation::GetForCurrentView().LogicalDpi() / dipsPerInch +
                  0.5f); // Round to nearest integer.
}

// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
inline Windows::Foundation::Point ConvertToScaledPoint(Windows::Foundation::Point point, float dpi)
{
    static const float dipsPerInch = 96.0f;
    return Windows::Foundation::Point(point.X * dpi / dipsPerInch, point.Y * dpi / dipsPerInch);
}

// Function that reads from a binary file asynchronously.
inline task<std::vector<byte>> ReadDataAsync(const std::wstring &filename)
{
    // StorageFolder folder = Package::Current().InstalledLocation();
    std::wstring prefixBuffer(256, L'\0');
    GetModuleFileNameW(nullptr, prefixBuffer.data(), static_cast<DWORD>(prefixBuffer.size()));
    while (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        prefixBuffer.resize(2 * prefixBuffer.size(), L'\0');
        GetModuleFileNameW(nullptr, prefixBuffer.data(), static_cast<DWORD>(prefixBuffer.size()));
    }
    std::filesystem::path prefix(prefixBuffer);
    auto folderPath = prefix.parent_path();
    auto filePath = folderPath / filename;

    return create_task([filePath]() -> task<std::vector<byte>> {
        auto file = co_await StorageFile::GetFileFromPathAsync(filePath.c_str());
        Streams::IBuffer fileBuffer = co_await FileIO::ReadBufferAsync(file);
        std::vector<byte> returnBuffer;
        returnBuffer.resize(fileBuffer.Length());
        Streams::DataReader::FromBuffer(fileBuffer).ReadBytes(returnBuffer);
        co_return returnBuffer;
    });
}
} // namespace DX

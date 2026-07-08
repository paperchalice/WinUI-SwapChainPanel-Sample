module;
#include "pch.h"

export module DX:DirectXHelper;

import std;
import std.compat;

import winrt.Windows.ApplicationModel;
import winrt.Windows.Foundation;
import winrt.Windows.Storage;

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

// Function that reads from a binary file asynchronously.
inline task<std::vector<byte>> ReadDataAsync(const std::wstring &filename)
{
    StorageFolder folder = Package::Current().InstalledLocation();
     StorageFile file = co_await folder.GetFileAsync(filename);
     Streams::IBuffer fileBuffer = co_await FileIO::ReadBufferAsync(file);
     std::vector<byte> returnBuffer;
     returnBuffer.resize(fileBuffer.Length());
     Streams::DataReader::FromBuffer(fileBuffer).ReadBytes(returnBuffer);
     co_return returnBuffer;
}
} // namespace DX

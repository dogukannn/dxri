#pragma once

#include "directx/d3dx12.h"
#include <directx/d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <exception>
#include <stdio.h>
#include <wincodec.h>
#include <atlbase.h>

extern ID3D12Device* GDevice;

class com_exception : public std::exception
{
public:
    com_exception(HRESULT hr) noexcept : result(hr) {}

    const char* what() const noexcept override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
        return s_str;
    }

    HRESULT get_result() const noexcept { return result; }

private:
    HRESULT result;
};

inline void ThrowIfFailed(HRESULT hr) noexcept(false)
{
    if (FAILED(hr))
    {
        throw com_exception(hr);
    }
}


WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);

// get the number of bits per pixel for a dxgi format
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

// get the dxgi format equivilent of a wic format
DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);

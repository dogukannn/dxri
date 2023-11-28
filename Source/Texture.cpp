#include "Texture.h"

#include "WICTextureLoader.h"
#include "ResourceUploadBatch.h"

void Texture::LoadFromFile(ID3D12Device* device, ID3D12CommandQueue* commandQueue, LPCWSTR filename)
{
	DirectX::ResourceUploadBatch resourceUpload(device);

	resourceUpload.Begin();

	ThrowIfFailed(
		CreateWICTextureFromFile(device, resourceUpload, filename,
			&Resource, true)
	);

	// Upload the resources to the GPU.
	auto uploadResourcesFinished = resourceUpload.End(commandQueue);

	// Wait for the upload thread to terminate
	uploadResourcesFinished.wait();

	Width = Resource->GetDesc().Width;
	Height = Resource->GetDesc().Height;
    Format = Resource->GetDesc().Format;
}

int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, UINT64& bytesPerRow)
{
	static IWICImagingFactory2 *wicFactory;

    // reset decoder, frame and converter since these will be different for each image we load
    IWICBitmapDecoder *wicDecoder = NULL;
    IWICBitmapFrameDecode *wicFrame = NULL;
    IWICFormatConverter *wicConverter = NULL;

    if(wicFactory == NULL)
    {
		CoInitialize(NULL);
		ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory2, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory)));
    }

    ThrowIfFailed(wicFactory->CreateDecoderFromFilename(
        filename,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &wicDecoder));

    ThrowIfFailed(wicDecoder->GetFrame(0, &wicFrame));

    WICPixelFormatGUID pixelFormat;
    ThrowIfFailed(wicFrame->GetPixelFormat(&pixelFormat));

    UINT textureWidth, textureHeight;
    ThrowIfFailed(wicFrame->GetSize(&textureWidth, &textureHeight));

    DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	if(DXGI_FORMAT_UNKNOWN)
	{
		//TODO convert the image https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-textures-from-file
	}

    UINT64 bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat);
    bytesPerRow = (textureWidth * bitsPerPixel) / 8;
    UINT64 imageSize = bytesPerRow * textureHeight;

    *imageData = (BYTE*)malloc(imageSize);

    ThrowIfFailed(wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData));

    resourceDescription = {};
    resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDescription.Alignment = 0;
    resourceDescription.Width = textureWidth;
    resourceDescription.Height = textureHeight;
    resourceDescription.DepthOrArraySize = 1;
    resourceDescription.MipLevels = 1;
    resourceDescription.Format = dxgiFormat;
    resourceDescription.SampleDesc.Count = 1;
    resourceDescription.SampleDesc.Quality = 0;
    resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

    return imageSize;
}

void Texture::LoadFromFileManual(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* commandAllocator, LPCWSTR filename)
{
    ID3D12Resource* textureBuffer;
    ID3D12Resource* textureBufferUploadHeap;

    D3D12_RESOURCE_DESC textureDesc;
    UINT64 imageBytesPerRow;
    BYTE* imageData;
    UINT64 imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"../Assets/lost_empire-RGBA.png", imageBytesPerRow);

    ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&textureBuffer)));
    textureBuffer->SetName(L"Texture Buffer Resource Heap");

    UINT64 textureUploadBufferSize;
    device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

    ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureBufferUploadHeap)));
    textureBuffer->SetName(L"Texture Buffer Upload Resource Heap");

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = &imageData[0];
    textureData.RowPitch = imageBytesPerRow;
    textureData.SlicePitch = imageBytesPerRow * textureDesc.Height;


	ID3D12GraphicsCommandList* uploadCommandList;
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
											commandAllocator, nullptr,
											IID_PPV_ARGS(&uploadCommandList)));
    UpdateSubresources(uploadCommandList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);
    uploadCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));
    uploadCommandList->Close();

    ID3D12CommandList* ppCommandLists[] = {uploadCommandList};
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    Resource = textureBuffer;
    Width = textureDesc.Width;
    Height = textureDesc.Height;
    Format = textureDesc.Format;
}

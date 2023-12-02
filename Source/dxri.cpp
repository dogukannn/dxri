#include "include/dxri.h"
#include <iostream>
#include "log.h"

DXRI::DXRI()
{

}

void DXRI::Initialize()
{
    CreateDevice();
}

ID3D12DescriptorHeap* DXRI::CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	ID3D12DescriptorHeap* descHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = type;
	desc.NumDescriptors = numDescriptors;
	desc.Flags = flags;
    ThrowIfFailed(Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap)));

    return descHeap;
}

ID3D12CommandQueue* DXRI::CreateCommandQueue()
{
    ID3D12CommandQueue* commandQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
    return commandQueue;
}

ID3D12CommandAllocator* DXRI::CreateCommandAllocator()
{
    ID3D12CommandAllocator* commandAllocator;
    ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
    return commandAllocator;
}

ID3D12Fence* DXRI::CreateFence()
{
    ID3D12Fence* fence;
    ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    return fence;
}

void DXRI::CreateDevice()
{
	ID3D12Debug* debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();

    // Select DirectX12 Physical Adapter
    ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)));

    Adapter = nullptr;
    for (UINT i = 0; factory->EnumAdapters1(i, &Adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        Adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
        {
            li() << "Device info : " << std::wstring(desc.Description);
            break;
        }
        Adapter->Release();
    }

	// Create DirectX12 device
    ThrowIfFailed(D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device)));
}

IDXGISwapChain3* DXRI::CreateSwapChain(LONG width, LONG height, UINT backbufferCount, HWND windowHandle, ID3D12CommandQueue* commandQueue)
{

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.BufferCount = backbufferCount;
    swapchainDesc.Width = width;
    swapchainDesc.Height = height;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.SampleDesc.Count = 1;

    IDXGISwapChain1* swapchain1;
    IDXGISwapChain3* swapchain3;

    ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue, windowHandle, &swapchainDesc, NULL, NULL, &swapchain1));

    ThrowIfFailed(swapchain1->QueryInterface(IID_PPV_ARGS(&swapchain3)));

    return swapchain3;
}

std::vector<ID3D12Resource*> DXRI::CreateRenderTargets(ID3D12DescriptorHeap* descHeap, IDXGISwapChain3* swapchain,
                                                       UINT backbufferCount)
{
    std::vector<ID3D12Resource*> renderTargets;
    renderTargets.resize(backbufferCount);

    UINT rtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(descHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT n = 0; n < backbufferCount; n++)
    {
        ThrowIfFailed(swapchain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
        Device->CreateRenderTargetView(renderTargets[n], nullptr, rtvHandle);
        rtvHandle.ptr += (1 * rtvDescriptorSize);
    }
    return renderTargets;
}

ID3D12Resource* DXRI::CreateDepthBuffer(UINT width, UINT height, ID3D12DescriptorHeap* descHeap)
{
    ID3D12Resource* depthBuffer;

    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthBuffer)
    ));
	descHeap->SetName(L"Depth/Stencil Resource Heap");
    depthBuffer->SetName(L"Depth Buffer");

	Device->CreateDepthStencilView(depthBuffer, &depthStencilDesc, descHeap->GetCPUDescriptorHandleForHeapStart());

    return depthBuffer;
}

ID3D12GraphicsCommandList* DXRI::CreateGraphicsCommandList(ID3D12CommandAllocator* commandAllocator,
	ID3D12PipelineState* pipelineState)
{
    ID3D12GraphicsCommandList* commandList;
	ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
											commandAllocator, pipelineState,
											IID_PPV_ARGS(&commandList)));
    return commandList;
}

ID3D12Resource* DXRI::CreateRawUploadBuffer(UINT size)
{
    ID3D12Resource* resource;
	D3D12_HEAP_PROPERTIES cbHeapProperties;
	cbHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	cbHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	cbHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	cbHeapProperties.CreationNodeMask = 1;
	cbHeapProperties.VisibleNodeMask = 1;


	D3D12_RESOURCE_DESC cbResourceDesc;
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Alignment = 0;
	cbResourceDesc.Width = size;
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.SampleDesc.Quality = 0;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	cbResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(Device->CreateCommittedResource(
		&cbHeapProperties, D3D12_HEAP_FLAG_NONE, &cbResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource)));

    return resource;
}


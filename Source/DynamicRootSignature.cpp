#include "DynamicRootSignature.h"

#include <iostream>

DynamicRootSignature::DynamicRootSignature()
{
}

bool DynamicRootSignature::Initialize(ID3D12Device* device, VertexShader* vertexShader, PixelShader* pixelShader)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData;
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


    for (auto& [name, idx]: vertexShader->Parameters.FreeParameterIndexMap)
	{
        auto rootParam = vertexShader->Parameters.RootParameters[idx];
        Parameters.RootParameters.push_back(rootParam);
        Parameters.FreeParameterIndexMap[name] = Parameters.RootParameters.size() - 1;
	}

    for (auto& [name, descTable]: vertexShader->Parameters.DescriptorTableIndexMap)
	{
        auto rootParam = vertexShader->Parameters.RootParameters[descTable.Index];
        DescriptorTableIndexed newDescTable = descTable;
        rootParam.DescriptorTable.pDescriptorRanges = newDescTable.DescriptorRanges.data();
        rootParam.DescriptorTable.NumDescriptorRanges = newDescTable.DescriptorRanges.size();
        Parameters.RootParameters.push_back(rootParam);
        newDescTable.Index = Parameters.RootParameters.size() - 1;
        Parameters.DescriptorTableIndexMap[name] = newDescTable;
	}

    for (auto& [name, idx]: pixelShader->Parameters.FreeParameterIndexMap)
	{
        auto rootParam = pixelShader->Parameters.RootParameters[idx];
        Parameters.RootParameters.push_back(rootParam);
        Parameters.FreeParameterIndexMap[name] = Parameters.RootParameters.size() - 1;
	}

    for (auto& [name, descTable]: pixelShader->Parameters.DescriptorTableIndexMap)
	{
        auto rootParam = pixelShader->Parameters.RootParameters[descTable.Index];
        DescriptorTableIndexed newDescTable = descTable;
        Parameters.DescriptorTableIndexMap[name] = newDescTable;
        rootParam.DescriptorTable.pDescriptorRanges = Parameters.DescriptorTableIndexMap[name].DescriptorRanges.data();
        rootParam.DescriptorTable.NumDescriptorRanges = Parameters.DescriptorTableIndexMap[name].DescriptorRanges.size();
        Parameters.RootParameters.push_back(rootParam);
        Parameters.DescriptorTableIndexMap[name].Index = Parameters.RootParameters.size() - 1;
	}

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.Desc_1_1.NumParameters = Parameters.RootParameters.size();
    rootSignatureDesc.Desc_1_1.pParameters = Parameters.RootParameters.data();
    rootSignatureDesc.Desc_1_1.NumStaticSamplers = 1;
    rootSignatureDesc.Desc_1_1.pStaticSamplers = &sampler;

    ID3DBlob* signature;
    ID3DBlob* error;

    try
    {
        ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));
        ThrowIfFailed(
            device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
        rootSignature->SetName(L"Dynamic Root Signature");
    }
    catch (std::exception e)
    {
        const char* errStr = (const char*)error->GetBufferPointer();
        std::cout << errStr;
        error->Release();
        error = nullptr;
        return false;
    }

    if(signature)
    {
        signature->Release();
        signature = nullptr;
    }

    return true;
}

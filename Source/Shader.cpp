#include "Shader.h"

#include <algorithm>
#include <optional>


Shader::Shader()
{
}

void Shader::Reflect(ShaderCompileOutput shaderData, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	D3D12_SHADER_DESC shaderDesc{};

	ThrowIfFailed(shaderData.ShaderReflection->GetDesc(&shaderDesc));

	std::vector<std::pair<uint32_t,uint32_t>> textureBindingIndices;
	for(uint32_t i = 0; i < shaderDesc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc;
		ThrowIfFailed(shaderData.ShaderReflection->GetResourceBindingDesc(i, &shaderInputBindDesc));

		if(shaderInputBindDesc.Type == D3D_SIT_CBUFFER)
		{
			Parameters.FreeParameterIndexMap[shaderInputBindDesc.Name] = static_cast<uint32_t>(Parameters.RootParameters.size());
			ID3D12ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer = shaderData.ShaderReflection->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC constantBufferDesc = {};
			shaderReflectionConstantBuffer->GetDesc(&constantBufferDesc);

			D3D12_ROOT_PARAMETER1 rootParameter;
			rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParameter.ShaderVisibility = shaderVisibility;

			D3D12_ROOT_DESCRIPTOR1 rootDescriptor = {};
			rootDescriptor.ShaderRegister = shaderInputBindDesc.BindPoint;
			rootDescriptor.RegisterSpace = shaderInputBindDesc.Space;
			rootDescriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;

			rootParameter.Descriptor = rootDescriptor;

			Parameters.RootParameters.push_back(rootParameter);
		}

		if (shaderInputBindDesc.Type == D3D_SIT_TEXTURE)
		{
			D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc;
			ThrowIfFailed(shaderData.ShaderReflection->GetResourceBindingDesc(i, &shaderInputBindDesc));
			textureBindingIndices.push_back({ shaderInputBindDesc.BindPoint, i});
		}
	}

	std::sort(textureBindingIndices.begin(), textureBindingIndices.end());

	DescriptorTableIndexed textureDescriptorTable;
	struct BindRange
	{
		uint32_t BindPointStart;
		uint32_t BindPointCount;
	};
	std::optional<BindRange> lastBindPoint = std::nullopt;
	for(uint32_t i = 0; i < textureBindingIndices.size(); i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc;
		ThrowIfFailed(shaderData.ShaderReflection->GetResourceBindingDesc(textureBindingIndices[i].second, &shaderInputBindDesc));
		bool newRange = true;

		if (lastBindPoint.has_value() && lastBindPoint->BindPointStart + lastBindPoint->BindPointCount == shaderInputBindDesc.BindPoint)
			newRange = false;

		lastBindPoint = { shaderInputBindDesc.BindPoint, shaderInputBindDesc.BindCount };
		textureDescriptorTable.IndexMap[shaderInputBindDesc.Name] = static_cast<uint32_t>(textureDescriptorTable.IndexMap.size());
		if(newRange)
		{
			D3D12_DESCRIPTOR_RANGE1 range = {};
			range.BaseShaderRegister = shaderInputBindDesc.BindPoint;
			range.NumDescriptors = shaderInputBindDesc.BindCount;
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.RegisterSpace = shaderInputBindDesc.Space; //not supporting multiple register spaces for now
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
			textureDescriptorTable.DescriptorRanges.push_back(range);
		}
		else
		{
			uint32_t rangeIndex = textureDescriptorTable.DescriptorRanges.size() - 1;
			textureDescriptorTable.DescriptorRanges[rangeIndex].NumDescriptors += shaderInputBindDesc.BindCount;
		}
	}

	if(!textureDescriptorTable.DescriptorRanges.empty())
	{
		D3D12_ROOT_PARAMETER1 textureDescTableParameter = {};
		textureDescTableParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		textureDescTableParameter.ShaderVisibility = shaderVisibility;
		//textureDescTableParameter.DescriptorTable.NumDescriptorRanges = textureDescriptorTable.DescriptorRanges.size();
		//textureDescTableParameter.DescriptorTable.pDescriptorRanges = textureDescriptorTable.DescriptorRanges.data();

		textureDescriptorTable.Index = static_cast<uint32_t>(Parameters.RootParameters.size());
		Parameters.DescriptorTableIndexMap["Textures"] = textureDescriptorTable;
		textureDescTableParameter.DescriptorTable.NumDescriptorRanges = Parameters.DescriptorTableIndexMap["Textures"].DescriptorRanges.size();
		textureDescTableParameter.DescriptorTable.pDescriptorRanges = Parameters.DescriptorTableIndexMap["Textures"].DescriptorRanges.data();
		Parameters.RootParameters.push_back(textureDescTableParameter);
	}
}

D3D12_SHADER_BYTECODE Shader::GetShaderByteCode()
{
	D3D12_SHADER_BYTECODE bytecode = {};
	bytecode.BytecodeLength = ShaderBlob->GetBufferSize();
	bytecode.pShaderBytecode = ShaderBlob->GetBufferPointer();
	return bytecode;
}

DXGI_FORMAT maskToFormat(BYTE mask, D3D_REGISTER_COMPONENT_TYPE componentType)
{
	switch (componentType)
	{
	case D3D_REGISTER_COMPONENT_FLOAT32:
		switch (mask)
		{
			case 0b1:
				return DXGI_FORMAT_R32_FLOAT;
			case 0b11:
				return DXGI_FORMAT_R32G32_FLOAT;
			case 0b111:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			default:
				return DXGI_FORMAT_UNKNOWN;
		}
	case D3D_REGISTER_COMPONENT_SINT32:
		switch (mask)
		{
			case 0b1:
				return DXGI_FORMAT_R32_SINT;
			case 0b11:
				return DXGI_FORMAT_R32G32_SINT;
			case 0b111:
				return DXGI_FORMAT_R32G32B32_SINT;
			default:
				return DXGI_FORMAT_UNKNOWN;
		}
	case D3D_REGISTER_COMPONENT_UINT32:
		switch (mask)
		{
			case 0b1:
				return DXGI_FORMAT_R32_UINT;
			case 0b11:
				return DXGI_FORMAT_R32G32_UINT;
			case 0b111:
				return DXGI_FORMAT_R32G32B32_UINT;
			default:
				return DXGI_FORMAT_UNKNOWN;
		}
	default:
		return DXGI_FORMAT_UNKNOWN;
	}

}

VertexShader::VertexShader(LPCWSTR shaderFile): Shader()
{
	auto shaderCompiler = ShaderCompiler::GetInstance();
	ShaderCompileOutput shaderData;
	shaderCompiler->CompileVertexShader(shaderFile, shaderData);
	ShaderBlob = shaderData.ShaderBlob;
	D3D12_SHADER_DESC shaderDesc{};
	ThrowIfFailed(shaderData.ShaderReflection->GetDesc(&shaderDesc));

	InputElementSemanticNames.reserve(shaderDesc.InputParameters);
	InputElementDescs.reserve(shaderDesc.InputParameters);

	for(uint32_t parameterIndex = 0; parameterIndex < shaderDesc.InputParameters; parameterIndex++)
	{
		D3D12_SIGNATURE_PARAMETER_DESC signatureParameterDesc = {};
		shaderData.ShaderReflection->GetInputParameterDesc(parameterIndex, &signatureParameterDesc);

		InputElementSemanticNames.emplace_back(signatureParameterDesc.SemanticName);

		D3D12_INPUT_ELEMENT_DESC inputElementDesc;
		inputElementDesc.SemanticName = InputElementSemanticNames.back().c_str();
		inputElementDesc.SemanticIndex = signatureParameterDesc.SemanticIndex;
		inputElementDesc.Format = maskToFormat(signatureParameterDesc.Mask, signatureParameterDesc.ComponentType);
		inputElementDesc.InputSlot = 0u;
		inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		inputElementDesc.InstanceDataStepRate = 0u;

		InputElementDescs.emplace_back(inputElementDesc);
	}

	InputLayoutDesc.NumElements = static_cast<uint32_t>(InputElementDescs.size());
	InputLayoutDesc.pInputElementDescs = InputElementDescs.data();

	Reflect(shaderData, D3D12_SHADER_VISIBILITY_VERTEX);
}

PixelShader::PixelShader(LPCWSTR shaderFile): Shader()
{
	auto shaderCompiler = ShaderCompiler::GetInstance();
	ShaderCompileOutput shaderData;
	shaderCompiler->CompilePixelShader(shaderFile, shaderData);
	ShaderBlob = shaderData.ShaderBlob;

	Reflect(shaderData, D3D12_SHADER_VISIBILITY_PIXEL);
}

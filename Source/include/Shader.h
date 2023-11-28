#pragma once
#include <map>

#include "ShaderCompiler.h"

struct DescriptorTableIndexed
{
	uint32_t Index;
	std::vector<D3D12_DESCRIPTOR_RANGE1> DescriptorRanges;
	std::map<std::string, std::uint32_t> IndexMap;
};

struct ShaderParameters
{
	std::vector<D3D12_ROOT_PARAMETER1> RootParameters;

	std::map<std::string, uint32_t> FreeParameterIndexMap;
	std::map<std::string, DescriptorTableIndexed> DescriptorTableIndexMap;
};

class Shader
{
protected:
	Shader();
	void Reflect(ShaderCompileOutput shaderData, D3D12_SHADER_VISIBILITY shaderVisibility);
public:
	std::string ShaderName;
	CComPtr<ID3DBlob> ShaderBlob;
	D3D12_SHADER_BYTECODE GetShaderByteCode();

	ShaderParameters Parameters;
};

class VertexShader : public Shader
{
public:
	VertexShader(LPCWSTR shaderFile);
	std::vector<std::string> InputElementSemanticNames;
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
};

class PixelShader : public Shader
{
public:
	PixelShader(LPCWSTR shaderFile);
};

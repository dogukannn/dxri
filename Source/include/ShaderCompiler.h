#pragma once
#include <atlbase.h>
#include <d3dcommon.h>
#include <dxcapi.h>
#include <d3d12shader.h>

struct ShaderCompileOutput
{
	ShaderCompileOutput() = default;
	CComPtr<ID3DBlob> ShaderBlob;
	CComPtr<ID3D12ShaderReflection> ShaderReflection;
	CComPtr<IDxcBlobUtf16> ShaderName;
};

class ShaderCompiler
{
public:

	static ShaderCompiler* GetInstance();
	inline static ShaderCompiler* Instance;
	
	bool CompileVertexShader(LPCWSTR shaderPath, ShaderCompileOutput& outCompileResults, LPCWSTR shaderName = L"") const;
	bool CompilePixelShader(LPCWSTR shaderPath, ShaderCompileOutput& outCompileResults, LPCWSTR shaderName = L"") const;

	CComPtr<IDxcUtils> Utils;
	CComPtr<IDxcCompiler3> Compiler;
	CComPtr<IDxcIncludeHandler> IncludeHandler;

private:
	ShaderCompiler();
	bool CompileShader(LPCWSTR* args, UINT argSize, LPCWSTR shaderPath, LPCWSTR shaderName,
	                   ShaderCompileOutput& outResults) const;
};

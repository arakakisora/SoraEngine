#pragma once
#include <string>
#include <map>
#include <tuple>
#include <d3d12shader.h>  // D3D_SHADER_INPUT_TYPE
#include <d3d12.h>        // D3D12_SHADER_VISIBILITY
#include <dxcapi.h>
#include <wrl.h>
#include <vector>


// リソースの一意な識別子
struct ShaderResourceKey {
	D3D_SHADER_INPUT_TYPE type;           // SRV / CBV / UAV
	D3D12_SHADER_VISIBILITY visibility;   // VERTEX / PIXEL など
	UINT bindPoint;                       // b0 / t1 の "0"や"1"
	UINT space;                           // register space（通常は0）

	// ShaderResourceKey型の等価比較演算子（ == ）を定義
	// すべてのメンバ（type, visibility, bindPoint, space）が等しい場合に true を返す
	bool operator==(const ShaderResourceKey& other) const {
		return type == other.type && visibility == other.visibility &&
			bindPoint == other.bindPoint && space == other.space;
	}

	// ShaderResourceKey型の比較演算子（<）を定義（std::map用）
	// std::tieでタプルを作成し、辞書順比較を行う
	bool operator<(const ShaderResourceKey& other) const {
		return std::tie(type, visibility, bindPoint, space) <
			std::tie(other.type, other.visibility, other.bindPoint, other.space);
	}
};

// 名前付きバインド情報
struct BindResourceInfo {
	ShaderResourceKey key;
	std::string name; // HLSLでの名前（例："Material"）
};
// バインド情報テーブルの別名
using ShaderResourceMap = std::map<ShaderResourceKey, BindResourceInfo>;

struct PipelineConfig {
	IDxcBlob* vsBlob = nullptr; // 頂点シェーダ
	IDxcBlob* psBlob = nullptr; // ピクセルシェーダ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature; // RootSig
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout; // 頂点レイアウト
	DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // RenderTarget Format
	DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT; // Depth Format
};





//バインド情報を取得する関数（バッファ・テクスチャ・UAVなど）
ShaderResourceMap ReflectShaderResources(IDxcBlob* shaderBlob, D3D12_SHADER_VISIBILITY visibility);

Microsoft::WRL::ComPtr<ID3D12RootSignature>CreateRootSignatureFromResourceMap(ID3D12Device* device, const ShaderResourceMap& resourceMap);

Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(ID3D12Device* device, const PipelineConfig& config);


std::vector<D3D12_INPUT_ELEMENT_DESC> CreateInputLayout(IDxcBlob* vsBlob);


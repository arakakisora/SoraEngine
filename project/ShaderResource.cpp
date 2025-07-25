#include "ShaderResource.h"
#include <d3dcompiler.inl>
#include <assert.h>
#include <externals/DirectXTex/d3dx12.h>


#pragma comment(lib, "d3dcompiler.lib")

ShaderResourceMap ReflectShaderResources(IDxcBlob* shaderBlob, D3D12_SHADER_VISIBILITY visibility)
{
	ShaderResourceMap resourceMap;

	//ShaderReflection オブジェクトを取得
	ID3D12ShaderReflection* reflector = nullptr;
	HRESULT hr = D3DReflect(
		shaderBlob->GetBufferPointer(),	//HLSLバイナリ先頭	
		shaderBlob->GetBufferSize(),	//バイナリサイズ
		IID_PPV_ARGS(&reflector)		//リフレクションオブジェクトを取得
	);

	//失敗した場合はからマップを返す
	if (FAILED(hr)) {

		assert(false && "Failed to reflect shader resources.");
		return resourceMap;
	}
	//シェーダーの全体情報を取得
	D3D12_SHADER_DESC shaderDesc;
	reflector->GetDesc(&shaderDesc);

	//バインドされてるループごとにループ
	for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
		D3D12_SHADER_INPUT_BIND_DESC bindDesc; {}
		reflector->GetResourceBindingDesc(i, &bindDesc);//i番目のリソースを取得

		// 取得対象を限定（使いたい型だけを対象に）
		if (bindDesc.Type != D3D_SIT_CBUFFER &&        // 定数バッファ
			bindDesc.Type != D3D_SIT_TEXTURE &&        // テクスチャ（SRV）
			bindDesc.Type != D3D_SIT_STRUCTURED &&     // 構造化バッファ
			bindDesc.Type != D3D_SIT_UAV_RWTYPED) {    // UAV（書き込み用バッファ）
			continue; // 上記以外のリソース（サンプラなど）はスキップ
		}

		// リソース位置を識別するキーを作成（レジスタ番号、可視性、種類など）
		ShaderResourceKey key{
			bindDesc.Type,        // リソース型（CBV, SRV, UAV など）
			visibility,           // 対応するシェーダーステージ（VERTEX, PIXELなど）
			bindDesc.BindPoint,   // バインドポイント（例：b0, t1 の "0" 部分）
			bindDesc.Space        // register space（通常は0）
		};

		//名前付きバインド情報を構築
		BindResourceInfo info;
		info.key = key;
		info.name = std::string(bindDesc.Name); // 名前をstd::stringに変換 HLSL側で書かれた変数名（例："Material"）

		//マップに登録
		resourceMap[key] = info;

	}
	return resourceMap;

}

Microsoft::WRL::ComPtr<ID3D12RootSignature>CreateRootSignatureFromResourceMap(ID3D12Device* device, const ShaderResourceMap& resourceMap)
{
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;    // ルートパラメータのリスト
	std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;// デスクリプタレンジのリスト srv/uav用
	bool hasSampler = false; // サンプラがあるかどうかのフラグ

	//各リソースに対してルート構成を行う
	for (const auto& [key, info] : resourceMap) {
		switch (key.type) {
			//コンスタントバッファ　（cbv）
		case D3D_SIT_CBUFFER: { // 定数バッファ
			// 定数バッファ（CBV）は直接ルートに登録
			D3D12_ROOT_PARAMETER param{};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			param.ShaderVisibility = key.visibility;
			param.Descriptor.ShaderRegister = key.bindPoint;
			param.Descriptor.RegisterSpace = key.space;
			rootParameters.push_back(param);
			break;
		}

		case D3D_SIT_TEXTURE:  // テクスチャ（SRV）
		case D3D_SIT_STRUCTURED: // 構造化バッファ（SRV）
		{
			// SRVはテーブルにまとめる（1つずつ登録）
			D3D12_DESCRIPTOR_RANGE range{};
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.NumDescriptors = 1;
			range.BaseShaderRegister = key.bindPoint;
			range.RegisterSpace = key.space;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			descriptorRanges.push_back(range);
			break;
		}
		case D3D_SIT_UAV_RWTYPED: {
			// UAV（必要なら追加）
			D3D12_DESCRIPTOR_RANGE range{};
			range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			range.NumDescriptors = 1;
			range.BaseShaderRegister = key.bindPoint;
			range.RegisterSpace = key.space;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			descriptorRanges.push_back(range);
			break;
		}
		case D3D_SIT_SAMPLER: {
			hasSampler = true;
			break;
		}
		default:
			// その他（バインドしない）
			break;
		}

	}

	if (!descriptorRanges.empty()) {
		D3D12_ROOT_PARAMETER tableParam{};
		tableParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		tableParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // 全てのシェーダーで使用可能
		 
		D3D12_ROOT_DESCRIPTOR_TABLE table{};
		table.NumDescriptorRanges = static_cast<UINT>(descriptorRanges.size());
		table.pDescriptorRanges = descriptorRanges.data(); // デスクリプタレンジの配列を設定

		tableParam.DescriptorTable = table; // ルートパラメータにテーブルを設定
		rootParameters.push_back(tableParam); // ルートパラメータリストに追加
	}

	//必要に応じて静的サンプラを登録(gSamples:s0)
	D3D12_STATIC_SAMPLER_DESC staticSampler{};
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	staticSampler.MinLOD = 0.0f;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0; // s0
	staticSampler.RegisterSpace = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ルートシグネチャ記述
	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.NumParameters = static_cast<UINT>(rootParameters.size());
	desc.pParameters = rootParameters.data();
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	if (hasSampler) {
		desc.NumStaticSamplers = 1;
		desc.pStaticSamplers = &staticSampler;
	}

	// バイナリ化（シリアライズ）＋作成
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		assert(false && "RootSignature serialization failed");
		return nullptr;
	}

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig = nullptr;
	hr = device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSig)
	);

	assert(SUCCEEDED(hr));
	return rootSig;

}

Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(ID3D12Device* device, const PipelineConfig& config)
{
	assert(config.vsBlob && config.psBlob);
	assert(config.rootSignature);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = config.rootSignature.Get();

	// シェーダーバイナリの設定
	psoDesc.VS = { config.vsBlob->GetBufferPointer(), config.vsBlob->GetBufferSize() };
	psoDesc.PS = { config.psBlob->GetBufferPointer(), config.psBlob->GetBufferSize() };

	// 頂点レイアウトの設定
	psoDesc.InputLayout.pInputElementDescs = config.inputLayout.data();
	psoDesc.InputLayout.NumElements = static_cast<UINT>(config.inputLayout.size());

	// ラスタライザステート（デフォルト）
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	// ブレンドステート（デフォルト）
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	// 深度ステンシルステート（デフォルト有効）
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DSVFormat = config.dsvFormat;

	// レンダーターゲット（1つ固定）
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = config.rtvFormat;

	// トポロジー（基本三角形リスト）
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// サンプル（マルチサンプルなし）
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.SampleMask = UINT_MAX;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
	assert(SUCCEEDED(hr));
	return pso;


}

std::vector<D3D12_INPUT_ELEMENT_DESC> CreateInputLayout(IDxcBlob* vsBlob)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

	ID3D12ShaderReflection* reflector = nullptr;
	HRESULT hr = D3DReflect(
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		IID_PPV_ARGS(&reflector)
	);
	if (FAILED(hr)) {
		assert(false && "Failed to reflect VS");
		return inputLayout;
	}

	D3D12_SHADER_DESC shaderDesc{};
	reflector->GetDesc(&shaderDesc);

	for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
		D3D12_SIGNATURE_PARAMETER_DESC paramDesc{};
		reflector->GetInputParameterDesc(i, &paramDesc);

		D3D12_INPUT_ELEMENT_DESC element{};
		element.SemanticName = paramDesc.SemanticName;
		element.SemanticIndex = paramDesc.SemanticIndex;
		element.InputSlot = 0; // 固定
		element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		element.InstanceDataStepRate = 0;

		// 型 → DXGI_FORMAT に変換
		if (paramDesc.Mask == 1) {
			element.Format = DXGI_FORMAT_R32_FLOAT;
		} else if (paramDesc.Mask <= 3) {
			element.Format = DXGI_FORMAT_R32G32_FLOAT;
		} else if (paramDesc.Mask <= 7) {
			element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		} else if (paramDesc.Mask <= 15) {
			element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		inputLayout.push_back(element);
	}

	return inputLayout;
}



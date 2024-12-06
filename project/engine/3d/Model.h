#pragma once
#include "ModelCommon.h"
#include "RenderingData.h"
class Model
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ModelCommon *modeleCommon,const std::string&directorypath,const std::string& filename);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	

	static MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename);
	static ModelData LoadObjeFile(const std::string& ditrectoryPath, const std::string& filename);

private:
	//モデル共通部のポインタ
	ModelCommon* modelCommon_ = nullptr;
	//モデル
	//objファイルのデータ
	ModelData modelData;
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	//ばふぁリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//マテリアル
	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//マテリアルにデータを書き込む	
	Material* materialData = nullptr;
	//std::string textureFilePath_;

};


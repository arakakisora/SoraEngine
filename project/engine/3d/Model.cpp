#include "Model.h"
#include <fstream>
#include <sstream>
#include <assert.h>
#include "TextureManager.h"
#include "SrvManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


void Model::Initialize(ModelCommon* modeleCommon, const std::string& directorypath, const std::string& filename)
{
	/*textureFilePath_ = filename;*/

	modelCommon_ = modeleCommon;

	modelData = LoadObjeFile(directorypath, filename);

	//モデルオブジェクト
	//モデル用のVetexResouceを作成
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	//マテリアル
	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	materialResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色
	materialData->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };

	materialData->enableLighting = true;//有効にするか否か
	materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();
	materialData->shiniess = 60.0f;


	//.objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	//読み込んだテクスチャ番号を取得
	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

}

void Model::Draw()
{
	//VertexBufferViewを設定
	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//マテリアルのCBufferの場所を設定
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定
	modelCommon_->GetSRVManager()->SetGraficsRootDescriptorTable(2, TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath));
	//描画！
	modelCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);


}



MaterialData Model::LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename)
{

	MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directorypath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなっかたら止める
	while (std::getline(file, line)) {
		std::string identifile;
		std::stringstream s(line);
		s >> identifile;

		//identifierの応じた処理
		if (identifile == "map_Kd") {

			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directorypath + "/" + textureFilename;

		}


	}

	return materialData;

}

ModelData Model::LoadObjeFile(const std::string& ditrectoryPath, const std::string& filename)
{
	ModelData modelData;//構築するModekData
	Assimp::Importer importer;
	std::string path = ditrectoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());//メッシュが何の歯対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());//法線情報がない
		assert(mesh->HasTextureCoords(0));//テクスチャ座標がない

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);//三角形以外は対応しない

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {

				uint32_t vectorIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vectorIndex];
				aiVector3D& normal = mesh->mNormals[vectorIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vectorIndex];
				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y };
				//aiprocess_MakeLefthandedはz*=-1で、右手->左手に変換するので手動で対応
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);

			}

		}
	}

	//マテリアルの解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString texturePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
			modelData.material.textureFilePath = ditrectoryPath + "/" + texturePath.C_Str();
		} 
	}

	
	return modelData;
}

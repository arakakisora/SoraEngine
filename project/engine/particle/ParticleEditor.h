#pragma once
#ifdef USE_IMGUI
#include "imgui.h"
#include <vector>
#include <string>
#include "ParticleMnager.h"

/// <summary>
/// パーティクルエディタ（ImGui）
/// ParticleManager の中身（particleGroups）をいじるためのツールクラス
/// </summary>
class ParticleEditor
{
public:
	explicit ParticleEditor(ParticleManager* manager)
		: manager_(manager)
	{
	}
	/// <summary>
	/// エディタの描画
	/// </summary>
	/// <remarks>
	void DrawImguiEditor();
	/// <summary>
	/// 基本設定UI
	/// </summary>
	/// <param name="currentName"></param>
	void BasicIMGui(const std::string& currentName);
	/// <summary>
	/// 頂点タイプ選択UI
	/// </summary>
	void VertexTypeIMGui(const std::string& currentName);
	/// <summary>
	// テクスチャ選択UI
	/// </summary>
	/// <param name="currentName"></param>
	void TextureSelectIMGui(const std::string& currentName);
	/// <summary>
	// ディレクトリからテクスチャ一覧を読み込み
	/// </summary>
	void LoadTexturesFromDirectory(const std::string& directory);
	/// <summary>
	/// ビヘイビア選択UI
	/// </summary>
	/// <param name="currentName"></param>
	void BehaviorIMGui(const std::string& currentName);
	/// <summary>
	/// エフェクト全体の設定UI
	/// </summary>
	void CreateModeIMGui();
	

	// 共通部品
	void DrawBehaviorSelector(const char* label, int& index);
	void DrawMeshSelector(const char* label, int& index);
	void DrawTextureSelector(const char* label, int& index);
	VerticesType GetMeshTypeFromIndex(int index) const;

private:
	ParticleManager* manager_ = nullptr;
	int currentIndex_ = 0;   // 現在選択中のエフェクト
	std::vector<std::string> textureFilePaths_; // テクスチャファイルパス一覧
	std::string textureDirectory_; // テクスチャディレクトリパス

	// 新規作成用
	char newEffectName_[128] = "NewEffect"; // 新規エフェクトの名前入力用バッファ
	int newBehaviorIndex_ = 0; // 新規エフェクトのビヘイビア選択用インデックス
	int newMeshIndex_ = 2;//Quadを初期化
	int newTextureIndex_ = 0;//テクスチャ選択の初期化

	int editBehaviorIndex_ = 0;

};
#endif // USE_IMGUI





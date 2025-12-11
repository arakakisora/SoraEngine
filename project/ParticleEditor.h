#pragma once
#ifdef USE_IMGUI
#include "imgui.h"
#include <vector>
#include <string>

class ParticleMnager;
/// <summary>
/// パーティクルエディタ（ImGui）
/// ParticleMnager の中身（particleGroups）をいじるためのツールクラス
/// </summary>
class ParticleEditor
{
public:
    explicit ParticleEditor(ParticleMnager* manager)
        : manager_(manager)
    {}

    // エディタウィンドウを描画
    void DrawImguiEditor();
	// 基本設定UI
	void BasicIMGui(const std::string& currentName);
	// 頂点タイプ選択UI
	void VertexTypeIMGui(const std::string& currentName);
	// テクスチャ選択UI
	void TextureSelectIMGui(const std::string& currentName);
	// ディレクトリからテクスチャ一覧を読み込み
	void LoadTexturesFromDirectory(const std::string& directory);
	void BehaviorIMGui(const std::string& currentName);

private:
    ParticleMnager* manager_ = nullptr;
    int currentIndex_ = 0;   // 現在選択中のエフェクト
	std::vector<std::string> textureFilePaths_; // テクスチャファイルパス一覧
	std::string textureDirectory_; // テクスチャディレクトリパス
};
#endif // USE_IMGUI





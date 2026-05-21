#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "Sprite.h"
#include "SpriteCommon.h"
#include "Vector2.h"

class UIeditor {
public:
    static UIeditor* GetInstance();
    static void DestroyInstance();

    void Initialize(SpriteCommon* spriteCommon);
    void Finalize();

    void SetScene(const std::string& sceneId);
    void Render();

#ifdef _DEBUG
    void DebugImGui();
#endif

private:
    struct UIElement {
        std::string name = "NewUI";
        std::string texturePath;

        Vector2 position{ 640.0f, 360.0f };
        Vector2 size{ 128.0f, 128.0f };

        bool visible = true;

        std::unique_ptr<Sprite> sprite;
    };

    struct UIScene {
        std::string id;
        std::vector<UIElement> elements;
    };

private:
    /// <summary>
	/// UIフォルダをスキャンして、テクスチャファイルをリストアップする
    /// </summary>
    void ScanUIFolder();
    /// <summary>
	/// 現在のシーンにUIエレメントを追加する（デバッグ用）
    /// </summary>
    void AddElement();
    /// <summary>
	/// 現在のシーンの選択中エレメントを削除する（デバッグ用）
    /// </summary>
    void DeleteSelectedElement();

    /// <summary>
	/// 現在のシーンのUIエレメントを保存する（デバッグ用）
    /// </summary>
    /// <param name="filePath"></param>
    void Save(const std::string& filePath);
    /// <summary>
	/// 指定したファイルからシーンのUIエレメントを読み込む（デバッグ用）
    /// </summary>
    /// <param name="filePath"></param>
    void Load(const std::string& filePath);

    

private:
    static std::unique_ptr<UIeditor> instance_;

    SpriteCommon* spriteCommon_ = nullptr;

    std::unordered_map<std::string, UIScene> scenes_;
    std::vector<std::string> uiTextureFiles_;

    std::string currentSceneId_ = "GamePlay";

    int selectedElementIndex_ = -1;

    bool visible_ = true;
    bool debugOpen_ = true;
};
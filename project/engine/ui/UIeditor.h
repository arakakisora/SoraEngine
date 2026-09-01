#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "Sprite.h"
#include "SpriteCommon.h"
#include "Vector2.h"

enum class UIElementType {
    Image,
    Rect
};

class UIeditor {
public:
    static UIeditor* GetInstance();
    static void DestroyInstance();
    /// <summary>
	/// 初期化
    /// </summary>
    /// <param name="spriteCommon"></param>
    void Initialize(SpriteCommon* spriteCommon);
    /// <summary>
	/// 終了
    /// </summary>
    void Finalize();

    /// <summary>
	/// シーンの切り替え
    /// </summary>
    /// <param name="sceneId"></param>
    void SetScene(const std::string& sceneId);
    /// <summary>
	/// 現在のシーンの描画
    /// </summary>
    void Render();

    /// <summary>
    /// 指定されたシーンIDと要素名に対して、押下（プレス）アニメーションを再生します。
    /// </summary>
    /// <param name="sceneId">アニメーションを再生するシーンの識別子。</param>
    /// <param name="elementName">アニメーションを再生する要素の名前。</param>
    void PlayPressAnimation(const std::string& sceneId, const std::string& elementName);
    /// <summary>
	/// 指定されたシーンIDと要素名に対して、押下状態を設定します。
    /// </summary>
    /// <param name="sceneId"></param>
    /// <param name="elementName"></param>
    /// <param name="isPressed"></param>
    void SetPressed(const std::string& sceneId, const std::string& elementName, bool isPressed);

    void SetCount(
        const std::string& sceneId,
        const std::string& elementName,
        int count
    );

#ifdef _DEBUG
    void DebugImGui();
#endif

private:
    struct UIElement {
        std::string name = "NewUI";
        std::string texturePath;
		// 位置とサイズの初期値は画面中央と128x128
        Vector2 position{ 640.0f, 360.0f };
        Vector2 size{ 128.0f, 128.0f };
		// 表示フラグ
        bool visible = true;
		// SpriteはUIエレメントの描画に使用
        std::unique_ptr<Sprite> sprite;

        // UIの種類
        UIElementType type = UIElementType::Image;
        // DrawCount用
        int count = 0;
        // 数字画像の1文字分のサイズ
        Vector2 digitTextureSize{ 32.0f, 48.0f };
        // 数字0が始まる位置
        Vector2 digitTextureOrigin{ 0.0f, 0.0f };

        bool pressAnimEnabled = true;
        // 押した時アニメ
        float pressAnimTime = 0.0f;
        float pressAnimDuration = 0.12f;
        float pressScale = 0.85f;
        // 長押しアニメ
        bool isPressed = false;
        float currentScale = 1.0f;
        float targetScale = 1.0f;
        float animSpeed = 0.25f;

        
    };

    struct UIScene {
		// シーンID
        std::string id;
		// UIエレメントのリスト
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
    /// <summary>
	/// シーンが存在しない場合に空のシーンを追加する
    /// </summary>
    /// <param name="sceneId"></param>
    void AddSceneIfMissing(const std::string& sceneId);
    /// <summary>
	/// 指定されたシーンIDと要素名に対して、カウント値を設定します
    /// </summary>
    /// <param name="sceneId"></param>
    /// <param name="elementName"></param>
    /// <param name="count"></param>
     

    void DrawUI(UIElement& element);
  

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
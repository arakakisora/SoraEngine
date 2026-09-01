#define NOMINMAX

#include "UIeditor.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <algorithm>
#ifdef _DEBUG
#include <imgui.h>
#endif

#include "json.hpp"
using json = nlohmann::json;

std::unique_ptr<UIeditor> UIeditor::instance_ = nullptr;

UIeditor* UIeditor::GetInstance() {
	if (!instance_) {
		instance_ = std::make_unique<UIeditor>();
	}

	return instance_.get();
}

void UIeditor::DestroyInstance() {
	instance_.reset();
}

void UIeditor::Initialize(SpriteCommon* spriteCommon) {
	spriteCommon_ = spriteCommon;
	assert(spriteCommon_);

	ScanUIFolder();

	scenes_.clear();

	currentSceneId_ = "GamePlay";

	// デフォルトのシーンとUIエレメントを作成
	Load("Resources/UI/ui_layout.json");

	AddSceneIfMissing("Title");
	AddSceneIfMissing("StageSelect");
	AddSceneIfMissing("GamePlay");
	AddSceneIfMissing("GameOver");
	AddSceneIfMissing("GameClear");
}

void UIeditor::Finalize() {
	scenes_.clear();
	uiTextureFiles_.clear();
	spriteCommon_ = nullptr;
}

void UIeditor::SetScene(const std::string& sceneId) {
	currentSceneId_ = sceneId;

	if (!scenes_.contains(sceneId)) {
		UIScene scene;
		scene.id = sceneId;
		scenes_[sceneId] = std::move(scene);
	}

	selectedElementIndex_ = -1;
}


void UIeditor::Render() {
	if (!visible_) {
		return;
	}

	if (!spriteCommon_) {
		return;
	}

	auto it = scenes_.find(currentSceneId_);

	if (it == scenes_.end()) {
		return;
	}

	UIScene& scene = it->second;

	for (auto& element : scene.elements) {
		if (!element.visible) {
			continue;
		}

		if (!element.sprite) {
			continue;
		}

		DrawUI(element);
	}
}


void UIeditor::SetPressed(const std::string& sceneId, const std::string& elementName, bool isPressed)
{
	auto it = scenes_.find(sceneId);
	if (it == scenes_.end()) {
		return;
	}

	for (auto& element : it->second.elements) {
		if (element.name == elementName) {
			if (!element.pressAnimEnabled) {
				return;
			}

			element.isPressed = isPressed;
			element.targetScale = isPressed ? element.pressScale : 1.0f;
			return;
		}
	}
}
void UIeditor::SetCount(const std::string& sceneId, const std::string& elementName, int count)
{
	auto it = scenes_.find(sceneId);

	if (it == scenes_.end()) {
		return;
	}

	for (auto& element : it->second.elements) {

		if (element.name == elementName) {

			element.count = count;

			return;
		}
	}
}
#ifdef _DEBUG
void UIeditor::DebugImGui() {

	if (!debugOpen_) {
		return;
	}

	if (ImGui::Begin("UI Manager", &debugOpen_)) {
		ImGui::Checkbox("Visible", &visible_);

		ImGui::Separator();

		char sceneBuf[128];
		strcpy_s(sceneBuf, currentSceneId_.c_str());

		if (ImGui::InputText("Scene ID", sceneBuf, sizeof(sceneBuf))) {
			SetScene(sceneBuf);
		}



		if (ImGui::BeginCombo("Scene Select", currentSceneId_.c_str())) {
			for (auto& [id, scene] : scenes_) {
				bool selected = currentSceneId_ == id;

				if (ImGui::Selectable(id.c_str(), selected)) {
					SetScene(id);
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Separator();

		if (ImGui::Button("Add UI")) {
			AddElement();
		}

		ImGui::SameLine();

		if (ImGui::Button("Scan Resources/UI")) {
			ScanUIFolder();
		}

		ImGui::Separator();

		UIScene& scene = scenes_[currentSceneId_];

		ImGui::Text("Elements");

		for (int i = 0; i < static_cast<int>(scene.elements.size()); i++) {
			bool selected = selectedElementIndex_ == i;

			std::string label = scene.elements[i].name;

			if (label.empty()) {
				label = "NoName";
			}

			label += "##Element" + std::to_string(i);

			if (ImGui::Selectable(label.c_str(), selected)) {
				selectedElementIndex_ = i;
			}
		}

		ImGui::Separator();

		if (selectedElementIndex_ >= 0 &&
			selectedElementIndex_ < static_cast<int>(scene.elements.size())) {

			UIElement& element = scene.elements[selectedElementIndex_];

			char nameBuf[128];
			strcpy_s(nameBuf, element.name.c_str());

			if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
				element.name = nameBuf;

				if (element.name.empty()) {
					element.name = "NewUI";
				}
			}

			ImGui::Checkbox("Element Visible", &element.visible);

			ImGui::DragFloat2("Position", &element.position.x, 1.0f);
			ImGui::DragFloat2("Size", &element.size.x, 1.0f, 1.0f, 4096.0f);

			const char* typeItems[] = {
				"Image",
				"Draw Count"
			};

			int typeIndex =
				static_cast<int>(element.type);

			if (ImGui::Combo(
				"Type",
				&typeIndex,
				typeItems,
				IM_ARRAYSIZE(typeItems)))
			{
				element.type =
					static_cast<UIElementType>(typeIndex);
			}

			if (element.type == UIElementType::Rect) {

				ImGui::DragFloat2(
					"Digit Texture Origin",
					&element.digitTextureOrigin.x,
					1.0f,
					0.0f,
					4096.0f
				);

				ImGui::DragFloat2(
					"Digit Texture Size",
					&element.digitTextureSize.x,
					1.0f,
					1.0f,
					4096.0f
				);

				ImGui::Text(
					"Preview Count: %d",
					element.count
				);
			}

			ImGui::Checkbox("Press Animation", &element.pressAnimEnabled);
			ImGui::DragFloat("Press Duration", &element.pressAnimDuration, 0.01f, 0.01f, 1.0f);
			ImGui::DragFloat("Press Scale", &element.pressScale, 0.01f, 0.5f, 1.0f);

			if (ImGui::BeginCombo("Texture", element.texturePath.c_str())) {
				for (const auto& texture : uiTextureFiles_) {
					bool selected = element.texturePath == texture;

					if (ImGui::Selectable(texture.c_str(), selected)) {
						element.texturePath = texture;

						element.sprite = std::make_unique<Sprite>();
						element.sprite->Initialize(spriteCommon_, element.texturePath);
					}
				}

				ImGui::EndCombo();
			}

			if (ImGui::Button("Delete Selected UI")) {
				DeleteSelectedElement();
			}
		}

		ImGui::Separator();

		if (ImGui::Button("Save")) {
			Save("Resources/UI/ui_layout.json");
		}

		ImGui::SameLine();

		if (ImGui::Button("Load")) {
			Load("Resources/UI/ui_layout.json");
		}
	}

	ImGui::End();
}
#endif

void UIeditor::ScanUIFolder() {
	uiTextureFiles_.clear();

	const std::string folder = "Resources/UI/";

	if (!std::filesystem::exists(folder)) {
		return;
	}

	for (const auto& entry : std::filesystem::directory_iterator(folder)) {
		if (!entry.is_regular_file()) {
			continue;
		}

		std::string ext = entry.path().extension().string();

		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
			uiTextureFiles_.push_back(entry.path().string());
		}
	}
}

void UIeditor::AddElement() {
	if (!scenes_.contains(currentSceneId_)) {
		UIScene scene;
		scene.id = currentSceneId_;
		scenes_[currentSceneId_] = std::move(scene);
	}

	UIElement element;

	element.name = "NewUI";
	element.position = { 640.0f, 360.0f };
	element.size = { 128.0f, 128.0f };
	element.visible = true;

	if (!uiTextureFiles_.empty()) {
		element.texturePath = uiTextureFiles_[0];

		element.sprite = std::make_unique<Sprite>();
		element.sprite->Initialize(spriteCommon_, element.texturePath);
	}

	UIScene& scene = scenes_[currentSceneId_];
	scene.elements.push_back(std::move(element));

	selectedElementIndex_ = static_cast<int>(scene.elements.size()) - 1;
}

void UIeditor::DeleteSelectedElement() {
	auto it = scenes_.find(currentSceneId_);

	if (it == scenes_.end()) {
		return;
	}

	auto& elements = it->second.elements;

	if (selectedElementIndex_ < 0 ||
		selectedElementIndex_ >= static_cast<int>(elements.size())) {
		return;
	}

	elements.erase(elements.begin() + selectedElementIndex_);

	selectedElementIndex_ = -1;
}

void UIeditor::Save(const std::string& filePath) {
	json root;
	root["scenes"] = json::array();

	for (auto& [id, scene] : scenes_) {
		json sceneJson;
		sceneJson["id"] = id;
		sceneJson["elements"] = json::array();

		for (auto& element : scene.elements) {
			json elementJson;

			elementJson["name"] = element.name;
			elementJson["texture"] = element.texturePath;
			elementJson["position"] = {
				element.position.x,
				element.position.y
			};
			elementJson["size"] = {
				element.size.x,
				element.size.y
			};
			// アニメーション設定も保存
			elementJson["pressAnimEnabled"] = element.pressAnimEnabled;
			elementJson["pressAnimDuration"] = element.pressAnimDuration;
			elementJson["pressScale"] = element.pressScale;
			elementJson["visible"] = element.visible;
			elementJson["type"] =
				static_cast<int>(element.type);

			elementJson["digitTextureOrigin"] = {
				element.digitTextureOrigin.x,
				element.digitTextureOrigin.y
			};

			elementJson["digitTextureSize"] = {
				element.digitTextureSize.x,
				element.digitTextureSize.y
			};

			sceneJson["elements"].push_back(elementJson);
		}

		root["scenes"].push_back(sceneJson);
	}

	std::ofstream ofs(filePath);

	if (!ofs.is_open()) {
		return;
	}

	ofs << root.dump(4);
}

void UIeditor::Load(const std::string& filePath) {
	std::ifstream ifs(filePath);

	if (!ifs.is_open()) {
		return;
	}

	json root;
	ifs >> root;

	scenes_.clear();

	for (auto& sceneJson : root["scenes"]) {
		UIScene scene;
		scene.id = sceneJson["id"].get<std::string>();

		for (auto& elementJson : sceneJson["elements"]) {
			UIElement element;

			element.name = elementJson["name"].get<std::string>();
			element.texturePath = elementJson["texture"].get<std::string>();

			element.position.x = elementJson["position"][0].get<float>();
			element.position.y = elementJson["position"][1].get<float>();

			element.size.x = elementJson["size"][0].get<float>();
			element.size.y = elementJson["size"][1].get<float>();

			element.visible = elementJson["visible"].get<bool>();

			if (elementJson.contains("type")) {

				element.type =
					static_cast<UIElementType>(
						elementJson["type"].get<int>()
						);
			}
			if (elementJson.contains(
				"digitTextureOrigin"))
			{
				element.digitTextureOrigin.x =
					elementJson["digitTextureOrigin"][0]
					.get<float>();

				element.digitTextureOrigin.y =
					elementJson["digitTextureOrigin"][1]
					.get<float>();
			}
			if (elementJson.contains(
				"digitTextureSize"))
			{
				element.digitTextureSize.x =
					elementJson["digitTextureSize"][0]
					.get<float>();

				element.digitTextureSize.y =
					elementJson["digitTextureSize"][1]
					.get<float>();
			}

			if (!element.texturePath.empty()) {
				element.sprite = std::make_unique<Sprite>();
				element.sprite->Initialize(spriteCommon_, element.texturePath);
			}

			if (elementJson.contains("pressAnimEnabled")) {
				element.pressAnimEnabled = elementJson["pressAnimEnabled"].get<bool>();
			}

			if (elementJson.contains("pressAnimDuration")) {
				element.pressAnimDuration = elementJson["pressAnimDuration"].get<float>();
			}

			if (elementJson.contains("pressScale")) {
				element.pressScale = elementJson["pressScale"].get<float>();
			}

			scene.elements.push_back(std::move(element));
		}

		scenes_[scene.id] = std::move(scene);
	}

	if (!scenes_.contains(currentSceneId_)) {
		currentSceneId_ = "GamePlay";
	}

	if (!scenes_.contains(currentSceneId_)) {
		AddSceneIfMissing(currentSceneId_);
	}
}

void UIeditor::AddSceneIfMissing(const std::string& sceneId)
{
	if (scenes_.contains(sceneId)) {
		return;
	}

	// シーンが存在しない場合は新規作成
	UIScene scene;
	scene.id = sceneId;
	scenes_[scene.id] = std::move(scene);
}

void UIeditor::PlayPressAnimation(const std::string& sceneId, const std::string& elementName)
{
	auto it = scenes_.find(sceneId);
	if (it == scenes_.end()) {
		return;
	}

	for (auto& element : it->second.elements) {
		if (element.name == elementName) {
			if (!element.pressAnimEnabled) {
				return;
			}

			// 押されたら必ずアニメ開始
			element.pressAnimTime = element.pressAnimDuration;
			return;
		}
	}

}

void UIeditor::DrawUI(UIElement& element)
{

	float scale = 1.0f;

	if (element.pressAnimTime > 0.0f) {
		element.pressAnimTime -= 1.0f / 60.0f;

		if (element.pressAnimTime < 0.0f) {
			element.pressAnimTime = 0.0f;
		}

		float t = element.pressAnimTime / element.pressAnimDuration;

		// 押した瞬間小さく、時間で元に戻る
		scale = 1.0f - (1.0f - element.pressScale) * t;
	}

	Vector2 drawSize = {
		element.size.x * scale,
		element.size.y * scale
	};

	// 中心基準で縮むように位置補正
	Vector2 drawPos = {
		element.position.x + (element.size.x - drawSize.x) * 0.5f,
		element.position.y + (element.size.y - drawSize.y) * 0.5f
	};

	if (element.type == UIElementType::Rect) {

		int digit =
			std::clamp(element.count, 0, 9);

		Vector2 textureLeftTop = {

			element.digitTextureOrigin.x +
			element.digitTextureSize.x *
			static_cast<float>(digit),

			element.digitTextureOrigin.y
		};

		element.sprite->SetTextureLeftTop(
			textureLeftTop
		);

		element.sprite->SetTextureSize(
			element.digitTextureSize
		);
	}

	element.sprite->SetPosition(drawPos);
	element.sprite->SetSize(drawSize);

	element.sprite->Update();
	element.sprite->Draw();

}

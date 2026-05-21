#define NOMINMAX

#include "UIeditor.h"

#include <cassert>
#include <filesystem>
#include <fstream>

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

    {
        UIScene scene;
        scene.id = "Title";
        scenes_[scene.id] = std::move(scene);
    }

    {
        UIScene scene;
        scene.id = "GamePlay";
        scenes_[scene.id] = std::move(scene);
    }

    currentSceneId_ = "GamePlay";

    Load("Resources/UI/ui_layout.json");
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

        element.sprite->SetPosition(element.position);
        element.sprite->SetSize(element.size);
        element.sprite->Update();
        element.sprite->Draw();
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
            elementJson["visible"] = element.visible;

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

            if (!element.texturePath.empty()) {
                element.sprite = std::make_unique<Sprite>();
                element.sprite->Initialize(spriteCommon_, element.texturePath);
            }

            scene.elements.push_back(std::move(element));
        }

        scenes_[scene.id] = std::move(scene);
    }

    if (!scenes_.contains(currentSceneId_)) {
        SetScene("GamePlay");
    }
}
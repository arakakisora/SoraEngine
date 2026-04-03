#define NOMINMAX 
#include "ParticleEditor.h"
#include "ParticleMnager.h"
#include "ParticleBehaviorFactory.h"
#include <algorithm> // 追加
#include <filesystem>
#include "imgui.h"
#ifdef USE_IMGUI

namespace fs = std::filesystem;

void ParticleEditor::DrawImguiEditor()
{
	if (!manager_) {
		return;
	}

	auto& groups = manager_->particleGroups; // friend class

	ImGui::Begin("Particle Editor");

	// 最初だけテクスチャ一覧を読む
	if (textureFilePaths_.empty()) {
		LoadTexturesFromDirectory("Resources/ParticleTexture");
	}

	//新規作成UI
	CreateEffectIMGui();

	if (ImGui::Button("Save To Json")) {
		manager_->SaveToJson("Resources/Data/Particles.json");
	}

	if (groups.empty()) {
		ImGui::Text("No particle groups.");
		ImGui::End();
		return;
	}
	
	// 名前一覧:編集UI
	std::vector<std::string> names;
	names.reserve(groups.size());
	for (const auto& [name, group] : groups) {
		names.push_back(name);
	}

	if (currentIndex_ >= (int)names.size()) {
		currentIndex_ = 0;
	}

	// コンボ用の配列
	std::vector<const char*> items;
	items.reserve(names.size());
	for (const auto& name : names) {
		items.push_back(name.c_str());
	}

	ImGui::Combo("Effect", &currentIndex_, items.data(), (int)items.size());

	const std::string& currentName = names[currentIndex_];


	BasicIMGui(currentName);//基本設定
	VertexTypeIMGui(currentName);//頂点タイプ選択
	ImGui::SeparatorText("Texture");//テクスチャ選択
	TextureSelectIMGui(currentName);//テクスチャ選択
	BehaviorIMGui(currentName);

	ImGui::End();

}

void ParticleEditor::BasicIMGui(const std::string& currentName)
{
	auto& groups = manager_->particleGroups;
	auto& currentGroup = groups.at(currentName);

	//基本設定
	ImGui::Text("ID: %s", currentName.c_str());//名前表示

	//デフォルト　カウント・ライフタイム
	int count = static_cast<int>(currentGroup.defaultCount);
	if (ImGui::DragInt("Count", &count, 1, 1, 1000)) {
		groups.at(currentName).defaultCount = (uint32_t)std::max(count, 1);
	}
	ImGui::DragFloat("Lifetime", &groups.at(currentName).defaultLifetime, 0.1f, 0.1f, 10.0f);
}

void ParticleEditor::VertexTypeIMGui(const std::string& currentName)
{
	auto& groups = manager_->particleGroups;
	auto& currentGroup = groups.at(currentName);
	ImGui::SeparatorText("Mesh");

	static const char* shapeItems[] = {
		"Ring",
		"Cylinder",
		"Quad",
		"Triangle",
	};

	int shapeIndex = 0;
	switch (currentGroup.verticesType) {
	case VerticesType::Ring:     shapeIndex = 0; break;
	case VerticesType::Cylinder: shapeIndex = 1; break;
	case VerticesType::Quad:     shapeIndex = 2; break;
	case VerticesType::Triangle: shapeIndex = 3; break;
	}

	if (ImGui::Combo("Shape", &shapeIndex, shapeItems, IM_ARRAYSIZE(shapeItems))) {
		VerticesType newType = VerticesType::Quad;
		switch (shapeIndex) {
		case 0: newType = VerticesType::Ring;     break;
		case 1: newType = VerticesType::Cylinder; break;
		case 2: newType = VerticesType::Quad;     break;
		case 3: newType = VerticesType::Triangle; break;
		}
		manager_->SetGroupVerticesType(currentName, newType);
	}

}

void ParticleEditor::TextureSelectIMGui(const std::string& currentName)
{
	auto& groups = manager_->particleGroups;
	auto& currentGroup = groups.at(currentName);

	if (textureFilePaths_.empty()) {
		ImGui::Text("No textures in %s", textureDirectory_.c_str());
	} else {

		// 今のグループが使っているテクスチャが何番目かを探す
		int texIndex = 0;
		std::string currentFileName =
			fs::path(currentGroup.materialdata.textureFilePath).filename().string();

		for (int i = 0; i < (int)textureFilePaths_.size(); ++i) {
			if (textureFilePaths_[i] == currentFileName) {
				texIndex = i;
				break;
			}
		}

		// ImGui 用 const char* 配列
		std::vector<const char*> texItems;
		texItems.reserve(textureFilePaths_.size());
		for (auto& s : textureFilePaths_) {
			texItems.push_back(s.c_str());
		}

		if (ImGui::Combo("Texture", &texIndex,
			texItems.data(), (int)texItems.size())) {

			const std::string& fileName = textureFilePaths_[texIndex];
			std::string fullPathStr = (fs::path(textureDirectory_) / fileName).string();


			manager_->SetGroupTexture(currentName, fullPathStr);
		}

		ImGui::Text("Path: %s", currentGroup.materialdata.textureFilePath.c_str());
	}

}

void ParticleEditor::LoadTexturesFromDirectory(const std::string& directory)
{

	textureDirectory_ = directory;
	textureFilePaths_.clear();

	// フォルダ内を走査
	for (const auto& entry : fs::directory_iterator(directory)) {
		if (!entry.is_regular_file()) {
			continue;
		}

		auto path = entry.path();
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		// png だけ使う（必要なら jpg 等も追加）
		if (ext == ".png") {
			textureFilePaths_.push_back(path.filename().string()); // "circle.png"
		}
	}

	std::sort(textureFilePaths_.begin(), textureFilePaths_.end());



}

void ParticleEditor::BehaviorIMGui(const std::string& currentName)
{
	auto& groups = manager_->particleGroups;
	auto& currentGroup = groups.at(currentName);

	if (currentGroup.behavior) {
		ImGui::SeparatorText("Behavior Params");
		currentGroup.behavior->DrawImgui(currentName.c_str());

		if (ImGui::Button("Test Emit (Camera)")) {
			// カメラ位置でテスト発生
			manager_->EmitAtCamera(currentName);
		}
	}
}

void ParticleEditor::CreateEffectIMGui()
{
	ImGui::SeparatorText("Create New Effect");

	// 名前入力
	ImGui::InputText("New Name", newEffectName_, IM_ARRAYSIZE(newEffectName_));

	// Behavior 一覧
	const auto& behaviorNames = ParticleBehaviorFactory::GetBehaviorNames();
	std::vector<const char*> behaviorItems;
	behaviorItems.reserve(behaviorNames.size());
	for (const auto& name : behaviorNames) {
		behaviorItems.push_back(name.c_str());
	}

	if (!behaviorItems.empty()) {
		if (newBehaviorIndex_ >= (int)behaviorItems.size()) {
			newBehaviorIndex_ = 0;
		}
		ImGui::Combo("Behavior", &newBehaviorIndex_,
			behaviorItems.data(), (int)behaviorItems.size());
	}

	// Mesh 一覧
	static const char* meshItems[] = {
		"Ring",
		"Cylinder",
		"Quad",
		"Triangle",
	};

	ImGui::Combo("New Mesh", &newMeshIndex_, meshItems, IM_ARRAYSIZE(meshItems));

	// Texture 一覧
	std::vector<const char*> textureItems;
	textureItems.reserve(textureFilePaths_.size());
	for (const auto& path : textureFilePaths_) {
		textureItems.push_back(path.c_str());
	}

	if (!textureItems.empty()) {
		if (newTextureIndex_ >= (int)textureItems.size()) {
			newTextureIndex_ = 0;
		}
		ImGui::Combo("New Texture", &newTextureIndex_,
			textureItems.data(), (int)textureItems.size());
	}

	// Create ボタン
	if (ImGui::Button("Create Effect")) {

		std::string effectName = newEffectName_;
		if (effectName.empty()) {
			return;
		}

		// 同名があるなら作らない
		if (manager_->particleGroups.contains(effectName)) {
			return;
		}

		// Behavior 生成
		if (behaviorNames.empty()) {
			return;
		}
		const std::string& behaviorType = behaviorNames[newBehaviorIndex_];
		auto behavior = ParticleBehaviorFactory::Create(behaviorType);
		if (!behavior) {
			return;
		}

		// Mesh 変換
		VerticesType meshType = VerticesType::Quad;
		switch (newMeshIndex_) {
		case 0: meshType = VerticesType::Ring;     break;
		case 1: meshType = VerticesType::Cylinder; break;
		case 2: meshType = VerticesType::Quad;     break;
		case 3: meshType = VerticesType::Triangle; break;
		}

		// Texture フルパス
		std::string texturePath;
		if (!textureFilePaths_.empty()) {
			texturePath = (fs::path(textureDirectory_) / textureFilePaths_[newTextureIndex_]).string();
		} else {
			texturePath = "Resources/ParticleTexture/default.png";
		}

		// ParticleGroup 作成
		manager_->CreateParticleGroup(effectName, texturePath, meshType, std::move(behavior));

		
	}

	if (ImGui::Button("Preview Emit")) {

		const std::string previewName = "__preview_effect__";

		// Behavior 生成
		if (behaviorNames.empty()) {
			return;
		}
		const std::string& behaviorType = behaviorNames[newBehaviorIndex_];
		auto behavior = ParticleBehaviorFactory::Create(behaviorType);
		if (!behavior) {
			return;
		}

		// Mesh 変換
		VerticesType meshType = VerticesType::Quad;
		switch (newMeshIndex_) {
		case 0: meshType = VerticesType::Ring;     break;
		case 1: meshType = VerticesType::Cylinder; break;
		case 2: meshType = VerticesType::Quad;     break;
		case 3: meshType = VerticesType::Triangle; break;
		}

		// Texture
		std::string texturePath;
		if (!textureFilePaths_.empty()) {
			texturePath = (fs::path(textureDirectory_) / textureFilePaths_[newTextureIndex_]).string();
		} else {
			texturePath = "Resources/ParticleTexture/default.png";
		}

		// 初回だけ作成
		if (!manager_->particleGroups.contains(previewName)) {
			manager_->CreateParticleGroup(previewName, texturePath, meshType, std::move(behavior));
		} else {
			manager_->SetBehavior(previewName, std::move(behavior));
			manager_->SetGroupVerticesType(previewName, meshType);
			manager_->SetGroupTexture(previewName, texturePath);
		}

		manager_->EmitAtCamera(previewName);
	}

}

#endif // USE_IMGUI
#include "StageEditor.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI
#include <fstream>
#include <iostream>
#include <filesystem>
#include "MapChipDatabase.h"

namespace fs = std::filesystem;

void StageEditor::Run() {
	// 初期化
	if (stageData_.GetWidth() == 0 || stageData_.GetHeight() == 0) {
		// グリッドの初期化
		constexpr int kGridWidth = 50;
		constexpr int kGridHeight = 25;
		//指定サイズで初期化
		stageData_.Resize(kGridWidth, kGridHeight);
		stageData_.Clear();
	}
	// ストローク訪問管理の初期化
	if (strokeVisited_.empty()) {
		strokeVisited_.resize(
			stageData_.GetHeight(),
			std::vector<bool>(stageData_.GetWidth(), false)
		);
	}

	RenderUI();
}

void StageEditor::RenderUI() {
#ifdef USE_IMGUI

	if (fs::exists("Resources/Mapdata") && fs::is_directory("Resources/Mapdata")) {
		std::vector<std::string> newList;
		for (const auto& entry : fs::directory_iterator("Resources/Mapdata")) {
			if (entry.path().extension() == ".csv") {
				newList.push_back(entry.path().filename().string());
			}
		}
		std::sort(newList.begin(), newList.end());
		if (newList != availableStages) {
			availableStages = std::move(newList);
			if (!availableStages.empty()) {
				if (selectedStageIndex >= availableStages.size()) selectedStageIndex = 0;
				strncpy_s(fileNameBuffer, sizeof(fileNameBuffer), availableStages[selectedStageIndex].c_str(), _TRUNCATE);
				fileNameBuffer[sizeof(fileNameBuffer) - 1] = '\0';
			} else {
				selectedStageIndex = 0;
				fileNameBuffer[0] = '\0';
			}
		}
	}

	ImGui::Begin("Stage Editor");
	ImGuiIO& io = ImGui::GetIO();

	// Ctrl+Z / Ctrl+Y
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) { Undo(); }
	if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Y) || (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z)))) { Redo(); }

	ImGui::Checkbox("Show Stage Window", &showStagewindow_);

	ImGui::InputText("FileName", fileNameBuffer, IM_ARRAYSIZE(fileNameBuffer));

	// 発射制限
	int shotLimit = stageData_.GetShotLimit();

	if (ImGui::InputInt("Shot Limit", &shotLimit)) {

		if (shotLimit < 0) {
			shotLimit = 0;
		}

		stageData_.SetShotLimit(shotLimit);
	}

	if (ImGui::Button("New")) {
		stageData_.Clear();// 新規作成
		undoStack_.clear();// Undo/Redo履歴をクリア
		redoStack_.clear();// Undo/Redo履歴をクリア
	}

	// セーブ
	if (ImGui::Button("Save CSV")) {
		// ファイルパスの生成
		std::string fullPath = "Resources/Mapdata/" + std::string(fileNameBuffer);
		SaveCSV(fullPath);
	}

	// ステージリスト（ドロップダウン） — 常に表示。空なら "No Stages"
	const char* comboPreview = availableStages.empty() ? "No Stages" : availableStages[selectedStageIndex].c_str();
	if (ImGui::BeginCombo("Stage List", comboPreview)) {
		if (availableStages.empty()) {
			ImGui::Selectable("No stages", false, ImGuiSelectableFlags_Disabled);
		} else {
			for (int i = 0; i < static_cast<int>(availableStages.size()); ++i) {
				const bool isSelected = (i == selectedStageIndex);
				if (ImGui::Selectable(availableStages[i].c_str(), isSelected)) {
					selectedStageIndex = i;
					strncpy_s(fileNameBuffer, sizeof(fileNameBuffer), availableStages[i].c_str(), _TRUNCATE);
					fileNameBuffer[sizeof(fileNameBuffer) - 1] = '\0'; // null終端
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}

	// ロード（選択中のファイル）
	if (ImGui::Button("Load Selected Stage")) {
		if (!availableStages.empty()) {
			// 選択中のステージをロード
			std::string selectedPath = "Resources/Mapdata/" + availableStages[selectedStageIndex];
			LoadCSV(selectedPath);
		}
	}

	if (ImGui::Button("Reload Map")) {
		isReloadRequested_ = true;
	}

	for (const auto& info : MapChipDatabase::GetInstance()->GetAll()) {
		ImGui::RadioButton(info.label.c_str(), &selectedType_, static_cast<int>(info.id));
	}

	if (static_cast<MapChipType>(selectedType_) == MapChipType::Portal) {
		ImGui::Separator();
		ImGui::Text("Portal Settings");

		ImGui::InputInt("Portal Link ID", &selectedPortalLinkId_);

		const char* dirItems[] = { "right", "left", "up", "down" };
		ImGui::Combo("Portal Direction", &selectedPortalDir_, dirItems, IM_ARRAYSIZE(dirItems));
	}

	ImGui::End();

	ImGui::Begin("Stage");

	const float kCellSize = 15.0f;
	const int rows = static_cast<int>(stageData_.GetHeight());
	const int cols = static_cast<int>(stageData_.GetWidth());

	ImGui::BeginChild("StageCanvasChild", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	ImVec2 scroll = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());

	// キャンバス全体サイズ
	ImVec2 canvasSize(cols * kCellSize, rows * kCellSize);

	// 1個だけ入力用の透明ボタンを置く
	ImGui::InvisibleButton("StageCanvasButton", canvasSize,
		ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

	bool isHovered = ImGui::IsItemHovered();
	bool isActive = ImGui::IsItemActive();

	ImVec2 mousePos = ImGui::GetIO().MousePos;

	// 背景
	dl->AddRectFilled(
		canvasPos,
		ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
		IM_COL32(40, 40, 40, 255));

	// 見えてる範囲だけ描画する
	ImVec2 winPos = ImGui::GetWindowPos();
	ImVec2 winSize = ImGui::GetWindowSize();

	float visibleMinX = scroll.x;
	float visibleMinY = scroll.y;
	float visibleMaxX = scroll.x + winSize.x;
	float visibleMaxY = scroll.y + winSize.y;

	int startX = std::max(0, static_cast<int>(visibleMinX / kCellSize));
	int startY = std::max(0, static_cast<int>(visibleMinY / kCellSize));
	int endX = std::min(cols, static_cast<int>(visibleMaxX / kCellSize) + 2);
	int endY = std::min(rows, static_cast<int>(visibleMaxY / kCellSize) + 2);

	// マウス座標→セル座標
	int hoverX = -1;
	int hoverY = -1;
	if (isHovered) {
		hoverX = static_cast<int>((mousePos.x - canvasPos.x) / kCellSize);
		hoverY = static_cast<int>((mousePos.y - canvasPos.y) / kCellSize);
		if (hoverX < 0 || hoverX >= cols || hoverY < 0 || hoverY >= rows) {
			hoverX = -1;
			hoverY = -1;
		}
	}

	// ストローク開始/終了
	if (isHovered) {
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			BeginStroke();
			isEditingDrag_ = true;
			dragButton_ = ImGui::IsMouseClicked(ImGuiMouseButton_Left) ? ImGuiMouseButton_Left : ImGuiMouseButton_Right;
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && hoverX >= 0 && hoverY >= 0) {
			stageData_.GetType(hoverX, hoverY);
		}
	}

	if (isEditingDrag_) {
		if ((dragButton_ == ImGuiMouseButton_Left && ImGui::IsMouseDown(ImGuiMouseButton_Left)) ||
			(dragButton_ == ImGuiMouseButton_Right && ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
			if (hoverX >= 0 && hoverY >= 0) {
				if (dragButton_ == ImGuiMouseButton_Left) {
					ApplyCellWithUndo(hoverX, hoverY, selectedType_);
				}
				if (dragButton_ == ImGuiMouseButton_Right) {
					ApplyCellWithUndo(hoverX, hoverY, 0);
				}
			}
		}

		if ((dragButton_ == ImGuiMouseButton_Left && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) ||
			(dragButton_ == ImGuiMouseButton_Right && ImGui::IsMouseReleased(ImGuiMouseButton_Right))) {
			isEditingDrag_ = false;
			dragButton_ = -1;
			EndStroke();
		}
	}

	// セル描画
	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			MapChipType typeId = stageData_.GetType(x, y);
			const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);
			
			ImVec4 colorV = ImVec4(1, 1, 1, 1);
			if (info) {
				colorV = ImVec4(info->color.x, info->color.y, info->color.z, info->color.w);
			}

			ImU32 fillColor = ImGui::ColorConvertFloat4ToU32(colorV);

			ImVec2 p0(canvasPos.x + x * kCellSize, canvasPos.y + y * kCellSize);
			ImVec2 p1(p0.x + kCellSize, p0.y + kCellSize);

			dl->AddRectFilled(p0, p1, fillColor);
			dl->AddRect(p0, p1, IM_COL32(25, 25, 25, 255));

			if (info && info->collision != "none") {
				dl->AddRect(p0, p1, IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
			}

			if (info && info->hitPoints > 0) {
				dl->AddLine(ImVec2(p0.x, p1.y), ImVec2(p1.x, p0.y), IM_COL32(0, 0, 0, 255), 1.0f);
			}

			if (info && !info->spawn.empty() && info->spawn != "none") {
				ImVec2 c((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
				dl->AddCircleFilled(c, 3.0f, IM_COL32(0, 0, 0, 255));
			}
		}
	}

	// ホバー中セルを強調
	if (hoverX >= 0 && hoverY >= 0) {
		ImVec2 p0(canvasPos.x + hoverX * kCellSize, canvasPos.y + hoverY * kCellSize);
		ImVec2 p1(p0.x + kCellSize, p0.y + kCellSize);
		dl->AddRect(p0, p1, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);

		const MapChipInfo* hoverInfo =
			MapChipDatabase::GetInstance()->GetById(stageData_.GetType(hoverX, hoverY));

		if (hoverInfo) {
			ImGui::BeginTooltip();
			ImGui::Text("id:%d  %s", hoverInfo->id, hoverInfo->label.c_str());
			ImGui::Text("collision: %s", hoverInfo->collision.c_str());
			ImGui::Text("hp: %d", hoverInfo->hitPoints);
			ImGui::Text("spawn: %s", hoverInfo->spawn.c_str());
			ImGui::EndTooltip();
		}
	}

	ImGui::EndChild();
	ImGui::End();

#endif // USE_IMGUI
}

void StageEditor::SaveCSV(const std::string& filename) {
	fs::create_directories("Resources/Mapdata");

	std::ofstream file(filename);

	// 発射制限を保存
	file << "#shotLimit,"
		<< stageData_.GetShotLimit()
		<< "\n";

	for (uint32_t y = 0; y < stageData_.GetHeight(); ++y) {
		for (uint32_t x = 0; x < stageData_.GetWidth(); ++x) {
			const StageCell& cell = stageData_.At(x, y);

			file << static_cast<int>(cell.type);

			// ポータルなら 3:0:right みたいに保存したい場合
			if (cell.type == MapChipType::Portal) {
				file << ":" << cell.linkId << ":" << DirectionToString(cell.direction);
			}

			if (x + 1 < stageData_.GetWidth()) {
				file << ",";
			}
		}
		file << "\n";
	}
}

void StageEditor::LoadCSV(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		return;
	}

	stageData_.Clear();

	std::string line;
	uint32_t y = 0;

	while (std::getline(file, line)) {

		// 発射制限
		if (line.rfind("#shotLimit,", 0) == 0) {

			std::string valueText =
				line.substr(std::string("#shotLimit,").size());

			stageData_.SetShotLimit(
				std::stoi(valueText)
			);

			continue;
		}

		std::stringstream lineStream(line);
		std::string cellText;
		uint32_t x = 0;

		while (std::getline(lineStream, cellText, ',')) {
			if (x >= stageData_.GetWidth() || y >= stageData_.GetHeight()) {
				++x;
				continue;
			}

			std::stringstream cellParser(cellText);

			std::string typeText;
			std::getline(cellParser, typeText, ':');

			MapChipType type = static_cast<MapChipType>(std::stoi(typeText));
			stageData_.SetType(x, y, type);

			if (type == MapChipType::Portal) {
				std::string linkText;
				std::string dirText;

				if (std::getline(cellParser, linkText, ':') &&
					std::getline(cellParser, dirText, ':')) {

					StageCell& cell = stageData_.At(x, y);
					cell.linkId = std::stoi(linkText);
					cell.direction = DirFromString(dirText);
				}
			}

			++x;
		}

		++y;
	}

	undoStack_.clear();
	redoStack_.clear();
}

void StageEditor::BeginStroke()
{
	// ストローク開始
	isStrokeActive_ = true;// ストロークアクティブ化
	currentStroke_.clear();// 現在のストローククリア
	// すでに記録したセルの重複防止用配列をクリア
	for (auto& row : strokeVisited_) {
		// セル訪問情報をクリア
		std::fill(row.begin(), row.end(), false);
	}

}

void StageEditor::EndStroke()
{
	if (!isStrokeActive_) return;
	isStrokeActive_ = false;

	if (!currentStroke_.empty()) {
		undoStack_.push_back(currentStroke_);
		redoStack_.clear(); // 新規編集が入ったらRedoは消す
	}
	currentStroke_.clear();

}

void StageEditor::ApplyCellWithUndo(int x, int y, int newType)
{
	if (x < 0 || y < 0) return;
	if (x >= static_cast<int>(stageData_.GetWidth())) return;
	if (y >= static_cast<int>(stageData_.GetHeight())) return;

	int before = static_cast<int>(stageData_.GetType(x, y));
	if (before == newType) return;

	if (isStrokeActive_ && !strokeVisited_[y][x]) {
		strokeVisited_[y][x] = true;
		currentStroke_.push_back(CellEdit{ x, y, before, newType });
	}

	stageData_.SetType(x, y, static_cast<MapChipType>(newType));

	if (static_cast<MapChipType>(newType) == MapChipType::Portal) {
		StageCell& cell = stageData_.At(x, y);
		cell.linkId = selectedPortalLinkId_;
		cell.direction = GetSelectedPortalDirection();
	}
}

void StageEditor::Undo()
{
	if (undoStack_.empty()) { return; }

	Stroke s = undoStack_.back();
	undoStack_.pop_back();


	// 逆順で戻す
	for (auto it = s.rbegin(); it != s.rend(); ++it) {
		stageData_.SetType(
			static_cast<uint32_t>(it->x),
			static_cast<uint32_t>(it->y), 
			static_cast<MapChipType>(it->before)
		);
	}

	redoStack_.push_back(std::move(s));
}

void StageEditor::Redo()
{
	
	if (redoStack_.empty()) { return; }// リドゥスタックが空なら何もしない
	Stroke s = redoStack_.back();
	redoStack_.pop_back();
	// 順番に適用する
	for (auto& e : s) {
		stageData_.SetType(
			static_cast<uint32_t>(e.x),
			static_cast<uint32_t>(e.y),
			static_cast<MapChipType>(e.after)
		);
	}

	undoStack_.push_back(std::move(s));
}

std::string StageEditor::DirectionToString(const Vector3& dir) const
{
	if (dir.x > 0.5f) return "right";
	if (dir.x < -0.5f) return "left";
	if (dir.y > 0.5f) return "up";
	if (dir.y < -0.5f) return "down";

	return "right";
}

Vector3 StageEditor::DirFromString(const std::string& dir) const
{
	if (dir == "right") return { 1.0f, 0.0f, 0.0f };
	if (dir == "left")  return { -1.0f, 0.0f, 0.0f };
	if (dir == "up")    return { 0.0f, 1.0f, 0.0f };
	if (dir == "down")  return { 0.0f, -1.0f, 0.0f };

	return { 1.0f, 0.0f, 0.0f };
}

Vector3 StageEditor::GetSelectedPortalDirection() const
{
	switch (selectedPortalDir_) {
	case 0:
		return { 1.0f, 0.0f, 0.0f };   // right
	case 1:
		return { -1.0f, 0.0f, 0.0f };  // left
	case 2:
		return { 0.0f, 1.0f, 0.0f };   // up
	case 3:
		return { 0.0f, -1.0f, 0.0f };  // down
	default:
		return { 1.0f, 0.0f, 0.0f };
	}
}

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
	if (grid_.empty()) {
		// グリッドの初期化
		constexpr int kGridWidth = 50;
		constexpr int kGridHeight = 25;
		// 2Dグリッドを指定サイズで初期化
		grid_.resize(kGridHeight, std::vector<GridCell>(kGridWidth));
	}
	// ストローク訪問管理の初期化
	if (strokeVisited_.empty()) {
		// グリッドと同じサイズで初期化
		strokeVisited_.resize(grid_.size(), std::vector<bool>(grid_[0].size(), false));
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


	// 新規作成
	ImGui::InputText("FileName", fileNameBuffer, IM_ARRAYSIZE(fileNameBuffer));
	if (ImGui::Button("New")) {
		for (auto& row : grid_) {
			for (auto& cell : row) {
				cell.type = 0; // 空白に初期化
			}
		}
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
		ImGui::RadioButton(info.label.c_str(), &selectedType_, info.id);
	}

	ImGui::End();

	ImGui::Begin("Stage");
	//bool canEdit = !ImGui::IsAnyItemActive();

	// ストローク開始/終了（Stageウィンドウ上での左/右操作）
	if (ImGui::IsWindowHovered()) {
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			BeginStroke();
		}
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
			EndStroke();
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	// マップ描画エリア
	for (int y = 0; y < static_cast<int>(grid_.size()); ++y) {
		for (int x = 0; x < static_cast<int>(grid_[y].size()); ++x) {

			// セルのタイプ（＝ JSON で定義した id）
			int typeId = grid_[y][x].type;

			// DB から色を取得
			const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);

			ImVec4 color(1, 1, 1, 1);
			if (info) {
				color = ImVec4(
					info->color.x,
					info->color.y,
					info->color.z,
					info->color.w
				);
			}

			constexpr float kCellSize = 12.0f;
			// 普通のクリックでも反応させる（既存動作）
			if (ImGui::ColorButton(
				("##" + std::to_string(x) + "_" + std::to_string(y)).c_str(),
				color, 0, ImVec2(kCellSize, kCellSize))) {
				ApplyCellWithUndo(x, y, selectedType_);
				
			}

			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImVec2 p0 = ImGui::GetItemRectMin();
			ImVec2 p1 = ImGui::GetItemRectMax();

			// タイル情報
			const MapChipInfo* cellInfo = MapChipDatabase::GetInstance()->GetById(grid_[y][x].type);

			// 1) 当たり判定があるなら枠表示
			if (cellInfo && cellInfo->collision != "none") {
				dl->AddRect(p0, p1, IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
			}

			// 2) 壊せる（HP>0）なら「斜線」1本（まずは簡易でOK）
			if (cellInfo && cellInfo->hitPoints > 0) {
				dl->AddLine(ImVec2(p0.x, p1.y), ImVec2(p1.x, p0.y), IM_COL32(0, 0, 0, 255), 1.0f);
			}

			// 3) スポーン系なら小さい点（player/goal/enemyなど）
			if (cellInfo && !cellInfo->spawn.empty() && cellInfo->spawn != "none") {
				ImVec2 c((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
				dl->AddCircleFilled(c, 2.0f, IM_COL32(0, 0, 0, 255));
			}
			// ツールチップ表示
			if (ImGui::IsItemHovered() && cellInfo) {
				ImGui::BeginTooltip();// ツールチップ開始
				ImGui::Text("id:%d  %s", cellInfo->id, cellInfo->label.c_str());// ラベル表示
				ImGui::Text("collision: %s", cellInfo->collision.c_str());// 当たり判定表示
				ImGui::Text("hp: %d", cellInfo->hitPoints);// HP表示
				ImGui::Text("spawn: %s", cellInfo->spawn.c_str());// スポーン情報表示	
				ImGui::EndTooltip();
			}

			
			

			// マウス操作処理
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem |
				ImGuiHoveredFlags_RectOnly)) {

				// セル上で押し始めたら編集ドラッグ開始
				if (!isEditingDrag_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					isEditingDrag_ = true;
					dragButton_ = ImGuiMouseButton_Left;
					BeginStroke();
				}
				if (!isEditingDrag_ && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					isEditingDrag_ = true;
					dragButton_ = ImGuiMouseButton_Right;
					BeginStroke();
				}

				// 編集ドラッグ中だけ塗る
				if (isEditingDrag_) {
					if (dragButton_ == ImGuiMouseButton_Left && io.MouseDown[ImGuiMouseButton_Left]) {
						ApplyCellWithUndo(x, y, selectedType_);
					}
					if (dragButton_ == ImGuiMouseButton_Right && io.MouseDown[ImGuiMouseButton_Right]) {
						ApplyCellWithUndo(x, y, 0);
					}
				}

				// スポイトはそのまま
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
					selectedType_ = grid_[y][x].type;
				}
			}

			ImGui::SameLine();
		}
		ImGui::NewLine();
	}
	// どこで離しても編集終了
	if (isEditingDrag_) {
		if ((dragButton_ == ImGuiMouseButton_Left && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) ||
			(dragButton_ == ImGuiMouseButton_Right && ImGui::IsMouseReleased(ImGuiMouseButton_Right))) {
			isEditingDrag_ = false;
			dragButton_ = -1;
			EndStroke();
		}
	}
	ImGui::PopStyleVar();
	ImGui::End();

#endif // USE_IMGUI
}

void StageEditor::SaveCSV(const std::string& filename) {
	fs::create_directories("Resources/Mapdata"); // フォルダがなければ作成
	std::ofstream file(filename);
	for (const auto& row : grid_) {
		for (size_t x = 0; x < row.size(); ++x) {
			file << row[x].type;
			if (x + 1 < row.size()) file << ",";
		}
		file << "\n";
	}
}

void StageEditor::LoadCSV(const std::string& filename) {
	// ファイルを開く
	std::ifstream file(filename);
	std::string line;
	int y = 0;
	// 1行ずつ読み込み
	//ホットリロード
	while (std::getline(file, line)) {
		int x = 0;
		size_t start = 0;
		while (start < line.size()) {
			size_t end = line.find(',', start);
			if (end == std::string::npos) end = line.size();
			int val = std::stoi(line.substr(start, end - start));
			if (y < grid_.size() && x < grid_[y].size()) {
				grid_[y][x].type = val;
			}
			start = end + 1;
			++x;
		}
		++y;
	}
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
	int& cell = grid_[y][x].type;
	if (cell == newType) return;

	// ストローク中、同じセルを二重記録しない
	if (isStrokeActive_ && !strokeVisited_[y][x]) {
		strokeVisited_[y][x] = true;
		currentStroke_.push_back(CellEdit{ x, y, cell, newType });
	}
	cell = newType;
}

void StageEditor::Undo()
{
	if (undoStack_.empty()) { return; }
	Stroke s = undoStack_.back();
	undoStack_.pop_back();

	// 逆順で戻す
	for (auto it = s.rbegin(); it != s.rend(); ++it) {
		grid_[it->y][it->x].type = it->before;
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
		grid_[e.y][e.x].type = e.after;
	}
	undoStack_.push_back(std::move(s));
}

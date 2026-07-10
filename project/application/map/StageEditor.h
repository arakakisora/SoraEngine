
#pragma once
#include <vector>
#include <string>
#include "MapChipDatabase.h"
#include "StageData.h"


/// <summary>
/// セル編集履歴構造体
/// </summary>
struct CellEdit {
	int x, y;
	int before, after;
};

/// <summary>
/// ステージエディタークラス
/// </summary>
class StageEditor {
public:
	// ファイル名バッファ
	char fileNameBuffer[64] = "stage1.csv";
	std::vector<std::string> availableStages;
	int selectedStageIndex = 0;

	/// <summary>
	/// エディターの実行
	/// </summary>
	void Run();
	/// <summary>
	/// リロードが要求されたかどうかを取得します
	/// </summary>
	/// <returns></returns>
	bool GetReloadRequested() const { return isReloadRequested_; }
	/// <summary>
	/// リロード要求を設定します
	/// </summary>
	/// <param name="value"></param>
	void SetReloadRequested(bool value) { isReloadRequested_ = value; }
	/// <summary>
	/// ファイル名を取得します
	/// </summary>
	const char* GetFileName() const { return fileNameBuffer; }

	StageData& GetStageData() { return stageData_; }
	const StageData& GetStageData() const { return stageData_; }
private:
	/// <summary>
	/// UIの描画
	/// </summary>
	void RenderUI();
	/// <summary>
	/// CSVの保存と読み込み
	/// </summary>
	void SaveCSV(const std::string& filename);
	/// <summary>
	/// CSVの読み込み
	/// </summary>
	void LoadCSV(const std::string& filename);

	/// ======アンドゥ・リドゥ機能======//
	/// <summary>
	/// ストロークの開始と終了
	/// </summary>
	void BeginStroke();
	/// <summary>
	/// ストロークの終了
	/// </summary>
	void EndStroke();
	/// <summary>
	/// セルの変更をUndo/Redo用に記録しつつ適用する
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="newType"></param>
	void ApplyCellWithUndo(int x, int y, int newType);
	/// <summary>
	/// Undo操作
	/// </summary>
	void Undo();
	/// <summary>
	/// Redo操作
	/// </summary>
	void Redo();

	std::string DirectionToString(const Vector3& dir) const;
	Vector3 DirFromString(const std::string& dir) const;
	Vector3 GetSelectedPortalDirection() const;

private:
	
	StageData stageData_;
	int selectedType_ = 1;// 選択中のマップチップタイプ
	bool isReloadRequested_ = false;// リロード要求フラグ
	bool isEditingDrag_ = false;
	int dragButton_ = -1; // 0=left, 1=right


	// Undo/Redo 用データ構造
	using Stroke = std::vector<CellEdit>;
	std::vector<Stroke> undoStack_;// 元に戻すスタック
	std::vector<Stroke> redoStack_;// やり直すスタック
	Stroke currentStroke_;// 現在のストローク
	bool isStrokeActive_ = false;// ストロークがアクティブかどうか
	// すでに記録したセルの重複防止
	std::vector<std::vector<bool>> strokeVisited_;
	bool showStagewindow_ = true;

	int selectedPortalLinkId_ = 0;
	int selectedPortalDir_ = 0;
	// 0:right, 1:left, 2:up, 3:down

};

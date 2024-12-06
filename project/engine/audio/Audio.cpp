#include "Audio.h"
#include <wrl.h>

Audio* Audio::instance_ = nullptr;

Audio* Audio::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new Audio();
	}
	return instance_;
}

void Audio::Initialize()
{
	

	
	//XAudioの生成
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	//マスターボイスの生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));


}

void Audio::Finalize()
{

	//XAudio2の解放
	xAudio2.Reset();
	delete instance_;

}

SoundData Audio::SoundLoadWave(const char* filename)
{

	//HRESULT hr;

	//ファイルの読み込み
	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	//ファイルオープン失敗を検出する
	assert(file.is_open());

	//wavデータの読み込み
	//RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	//対ぷふぁwaveかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}
	//フォーマットチャンクの読み込み
	FormatChunk format = {};
	//チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	//チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	//Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	//JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		//読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		//再読取り
		file.read((char*)&data, sizeof(data));
	}

	//LISTチャンクを検出した場合
	if (strncmp(data.id, "LIST", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	//dataチャンクのデータぶ（波形データ）の読み込み
	char* pbuffer = new char[data.size];
	file.read(pbuffer, data.size);

	//	waveファイルを閉じる
	file.close();

	SoundData soundData = {  };
	soundData.wfex = format.fmt;
	soundData.PBuffer = reinterpret_cast<BYTE*>(pbuffer);
	soundData.bufferSize = data.size;

	return soundData;

}

void Audio::SoundUnload(SoundData* soundData)
{
	
	if (pSourceVoice) {
		pSourceVoice->Stop();
		pSourceVoice->DestroyVoice();
		pSourceVoice = nullptr;
	}

	//buffaのメモリ解放
	delete[] soundData->PBuffer;
	soundData->PBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};

}

void Audio::SoundPlayWave(const SoundData& soundData)
{
	HRESULT hr;

	//波形フォーマット元にSourcwvoiceの生成
	pSourceVoice = nullptr;
	hr = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(hr));

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.PBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	//波形データ再生
	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	hr = pSourceVoice->Start();

}



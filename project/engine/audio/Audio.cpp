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
	// XAudio2の初期化
    HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(hr));
	// マスターボイスの生成
    hr = xAudio2->CreateMasteringVoice(&masterVoice);
    assert(SUCCEEDED(hr));
}

void Audio::Finalize()
{
    // すべての再生中の音を停止
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->Stop();
            voice->DestroyVoice();
        }
    }
    activeVoices.clear();

    xAudio2.Reset();
    delete instance_;
}

SoundData Audio::SoundLoadWave(const char* filename)
{
	// ファイルを開く
    std::ifstream file(filename, std::ios_base::binary);
    assert(file.is_open());
	// RIFFヘッダーを読む
    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    assert(strncmp(riff.chunk.id, "RIFF", 4) == 0);
    assert(strncmp(riff.type, "WAVE", 4) == 0);
	// fmtチャンクを読む
    FormatChunk format = {};
    file.read((char*)&format, sizeof(ChunkHeader));
    assert(strncmp(format.chunk.id, "fmt ", 4) == 0);
    assert(format.chunk.size <= sizeof(format.fmt));
    file.read((char*)&format.fmt, format.chunk.size);
	// dataチャンクを探す
    ChunkHeader data;
    file.read((char*)&data, sizeof(data));
	// "JUNK"チャンクや"LIST"チャンクをスキップする
    if (strncmp(data.id, "JUNK", 4) == 0 || strncmp(data.id, "LIST", 4) == 0) {
        file.seekg(data.size, std::ios_base::cur);
        file.read((char*)&data, sizeof(data));
    }
	//  "data"チャンクであることを確認
    assert(strncmp(data.id, "data", 4) == 0);
	// 波形データを読む
    char* pbuffer = new char[data.size];
    file.read(pbuffer, data.size);
    file.close();
	// SoundData構造体に格納して返す
    SoundData soundData = {};
    soundData.wfex = format.fmt;
    soundData.PBuffer = reinterpret_cast<BYTE*>(pbuffer);
    soundData.bufferSize = data.size;

    return soundData;
}

void Audio::SoundUnload(SoundData* soundData)
{
	// 再生中の音を停止
    StopSpecificAudio(soundData);
    delete[] soundData->PBuffer;
    soundData->PBuffer = nullptr;
    soundData->bufferSize = 0;
    soundData->wfex = {};
}

void Audio::SoundPlayWave(const SoundData& soundData)
{
    HRESULT hr;
	// すでに再生中の場合は停止してから再生し直す
    StopSpecificAudio(const_cast<SoundData*>(&soundData));
	// ソースボイスの生成
    IXAudio2SourceVoice* newVoice = nullptr;
    hr = xAudio2->CreateSourceVoice(&newVoice, &soundData.wfex);
    assert(SUCCEEDED(hr));
	// バッファの送信と再生開始
    XAUDIO2_BUFFER buf{};
    buf.pAudioData = soundData.PBuffer;
    buf.AudioBytes = soundData.bufferSize;
    buf.Flags = XAUDIO2_END_OF_STREAM;
	// SubmitSourceBufferとStartの結果は特に使わないが、念のためチェックしておく
    hr = newVoice->SubmitSourceBuffer(&buf);
    hr = newVoice->Start();
	// 再生中の音として登録
    activeVoices[const_cast<SoundData*>(&soundData)] = newVoice;
}

void Audio::StopAudio()
{
	// すべての再生中の音を停止
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->Stop();
            voice->DestroyVoice();
        }
    }
    activeVoices.clear();
}

void Audio::StopSpecificAudio(SoundData* soundData)
{
	// 指定した音を停止
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            voice->Stop();
            voice->DestroyVoice();
        }
        activeVoices.erase(it);
    }
}

void Audio::PauseAudio()
{
	// すべての再生中の音を一時停止
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->Stop();
        }
    }
}

void Audio::PauseSpecificAudio(SoundData* soundData)
{
	// 指定した音を一時停止
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            voice->Stop();
        }
    }
}

void Audio::ResumeAudio()
{
	// すべての再生中の音を再開
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->Start();
        }
    }
}

void Audio::ResumeSpecificAudio(SoundData* soundData)
{
	// 指定した音を再開
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            voice->Start();
        }
    }
}

void Audio::SetPlaybackSpeed(float speed)
{
	// すべての再生中の音の速度変更
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            HRESULT hr = voice->SetFrequencyRatio(speed);
            assert(SUCCEEDED(hr));
        }
    }
}

void Audio::SetPlaybackSpeed(SoundData* soundData, float speed)
{
	// 指定した音の速度変更
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            HRESULT hr = voice->SetFrequencyRatio(speed);
            assert(SUCCEEDED(hr));
        }
    }
}

bool Audio::IsSoundPlaying() const
{
	// 何か音が鳴っているか確認
    for (const auto& [soundData, voice] : activeVoices) {
        if (voice) {
            XAUDIO2_VOICE_STATE state;
            voice->GetState(&state);
            if (state.BuffersQueued > 0) {
                return true;
            }
        }
    }
    return false;
}



bool Audio::IsSoundPlaying(SoundData* soundData) const
{
	// 指定した音が鳴っているか確認
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            XAUDIO2_VOICE_STATE state;
            voice->GetState(&state);
            return state.BuffersQueued > 0;
        }
    }
    return false;
}

void Audio::SetVolume(float volume)
{
	// すべての再生中の音の音量変更
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->SetVolume(volume);
        }
    }
}

void Audio::SetVolume(SoundData* soundData, float volume)
{
	// 指定した音の音量変更
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            voice->SetVolume(volume);
        }
    }
}

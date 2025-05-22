#pragma once
#include "DirectXCommon.h"
#include "RenderingData.h"
class LightManager
{

	public:
	//ディレクションライト
	void SetDirectionalLight(const DirectionalLight& directionalLight) { *directionalLightData = directionalLight; }
	DirectionalLight GetDirectionalLight() { return *directionalLightData; }
	//ディレクションライトの向き
	void SetDirectionalLightDirection(const Vector3& direction) { directionalLightData->direction = direction; }
	//ディレクションライトの色
	void SetDirectionalLightColor(const Vector4& color) { directionalLightData->color = color; }
	//ディレクションライトの強さ
	void SetDirectionalLightIntensity(float intensity) { directionalLightData->intensity = intensity; }
	//ライトオンオフ
	void SetDirectionalLightEnable(bool enable) { directionalLightData->enable = enable; }

	//ポイントライト
	void SetPointLight(const PointLight& pointLight) { *pointLightData = pointLight; }
	PointLight GetPointLight() { return *pointLightData; }
	//ポイントライトの位置
	void SetPointLightPosition(const Vector3& position) { pointLightData->position = position; }
	//ポイントライトの色
	void SetPointLightColor(const Vector4& color) { pointLightData->color = color; }
	//ポイントライトの強さ
	void SetPointLightIntensity(float intensity) { pointLightData->intensity = intensity; }
	//ポイントライトの半径
	void SetPointLightRadius(float radius) { pointLightData->radius = radius; }
	float GetPointLightRadius() { return pointLightData->radius; }
	//ポイントライトの減衰率
	void SetPointLightDecay(float decay) { pointLightData->decay = decay; }
	float GetPointLightDecay() { return pointLightData->decay; }
	//ポイントライトのオンオフ
	void SetPointLightEnable(bool enable) { pointLightData->enable = enable; }


	//スポットライト
	void SetSpotLight(const SpotLight& spotLight) { *spotLightData = spotLight; }
	SpotLight GetSpotLight() { return *spotLightData; }
	//スポットライトの位置
	void SetSpotLightPosition(const Vector3& position) { spotLightData->position = position; }
	//スポットライトの向き
	void SetSpotLightDirection(const Vector3& direction) { spotLightData->direction = direction; }
	//スポットライトの色
	void SetSpotLightColor(const Vector4& color) { spotLightData->color = color; }
	//スポットライトの強さ
	void SetSpotLightIntensity(float intensity) { spotLightData->intensity = intensity; }
	//スポットライトの距離
	void SetSpotLightDistance(float distance) { spotLightData->distance = distance; }
	//スポットライトの減衰率
	void SetSpotLightDecay(float decay) { spotLightData->decay = decay; }
	//スポットライトのコーンの角度
	void SetSpotLightConsAngle(float consAngle) { spotLightData->consAngle = consAngle; }

	void SetSpotLightCosFalloffstrt(float cosFalloffstrt) { spotLightData->cosFalloffstrt = cosFalloffstrt; }
	//スポットライトのオンオフ
	void SetSpotLightEnable(bool enable) { spotLightData->enable = enable; }




	//ライトのオンオフ
	void SetLighting(bool enable) { enableLighting = enable; }

private:



    DirectXCommon* dxCommon_ = nullptr;

    // 平行光源
    DirectionalLight directionalLight_;
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
    DirectionalLight* directionalLightData = nullptr;

    // 複数のポイントライト
    std::vector<PointLight> pointLights_;
    Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;
    PointLight* pointLightData = nullptr;

    // 複数のスポットライト
    std::vector<SpotLight> spotLights_;
    Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_;
    SpotLight* spotLightData = nullptr;


	//ライトのオンオフ
	bool enableLighting = true;

};


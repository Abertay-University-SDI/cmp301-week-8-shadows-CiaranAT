// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void updateVariables();
	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;
	CubeMesh* cube;
	SphereMesh* sphere;
	SphereMesh* lightSphere;

	Light* lightArray[2];
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMap;

	OrthoMesh* shadowMapMesh;

	XMFLOAT3 lightPosSlider;
	XMFLOAT3 lightDirSlider;
	float rotation;

	int sceneWidth = 100;
	int sceneHeight = 100;
};

#endif
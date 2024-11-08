// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	lightSphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	shadowMapMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, -screenWidth / 2.7, screenHeight / 2.7);
	shadowMapMesh2 = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, screenWidth / 2.7, screenHeight / 2.7);

	// Variables for defining shadow map
	int shadowmapWidth = 1024;
	int shadowmapHeight = 1024;
	sceneWidth = 100;
	sceneHeight = 100;
	float shadowResMult = 10;

	// Configure directional light
	// Initialise light
	for (int i = 0; i < 2; i++) //generate shadow maps and lights
	{
		shadowMap[i] = new ShadowMap(renderer->getDevice(), shadowmapWidth * shadowResMult, shadowmapHeight * shadowResMult);
		lightArray[i] = new Light();
	}

	lightArray[0]->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	lightArray[0]->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	lightArray[0]->setDirection(0.0f, -0.7f, 0.7f);
	lightDirSlider = lightArray[0]->getDirection();
	lightArray[0]->setPosition(0.f, 0.f, -10.f);
	lightPosSlider = lightArray[0]->getPosition();

	lightArray[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	lightArray[1]->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	lightArray[1]->setDiffuseColour(0.0f, 0.0f, 1.0f, 1.0f);
	lightArray[1]->setDirection(0.0f, -0.7f, -0.7f);
	lightArray[1]->setPosition(0.f, 0.f, 40.f);

	lightArray[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{

	// Perform depth pass
	depthPass();

	updateVariables();

	// Render scene
	finalPass();

	return true;
}

void App1::updateVariables()
{
	rotation += timer->getTime();

	lightArray[0]->setPosition(lightPosSlider.x, lightPosSlider.y, lightPosSlider.z);
	lightArray[0]->setDirection(lightDirSlider.x, lightDirSlider.y, lightDirSlider.z);
	lightArray[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	//light->generateProjectionMatrix(4.f, 100.f);
}

void App1::depthPass()
{
	for (int i = 0; i < 2; i++) {

		// Set the render target to be the render to texture.
		shadowMap[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		// get the world, view, and projection matrices from the camera and d3d objects.
		lightArray[i]->generateViewMatrix();
		XMMATRIX lightViewMatrix = lightArray[i]->getViewMatrix();
		XMMATRIX lightProjectionMatrix = lightArray[i]->getOrthoMatrix();
		XMMATRIX worldMatrix = renderer->getWorldMatrix();

		worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
		// Render floor
		mesh->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

		worldMatrix = XMMatrixTranslation(-10.f, 5.f, 20.f);

		// Render cube 
		cube->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

		worldMatrix = XMMatrixTranslation(10.f, 5.f, 20.f);
		// Render sphere  
		sphere->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixRotationY(rotation) * XMMatrixTranslation(0.f, 7.f, 5.f);
		XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
		worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
		// Render model
		model->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

		worldMatrix = renderer->getWorldMatrix();

		// Set back buffer as render target and reset view port.
		renderer->setBackBufferRenderTarget();
		renderer->resetViewport();
	}
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	ID3D11ShaderResourceView* shadowMapPassTexture[2] = { shadowMap[0]->getDepthMapSRV(), shadowMap[0]->getDepthMapSRV() };

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMapPassTexture, lightArray);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = XMMatrixTranslation(-10.f, 5.f, 20.f);
	// Render cube 
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMapPassTexture, lightArray);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	worldMatrix = XMMatrixTranslation(10.f, 5.f, 20.f);
	// Render sphere  
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMapPassTexture, lightArray);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	worldMatrix = XMMatrixTranslation(lightPosSlider.x, lightPosSlider.y, lightPosSlider.z);
	// Render sphere  
	lightSphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMapPassTexture, lightArray);
	shadowShader->render(renderer->getDeviceContext(), lightSphere->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixRotationY(rotation) * XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMapPassTexture, lightArray);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();

	// RENDER THE RENDER TEXTURE SCENE
	// Requires 2D rendering and an ortho mesh.
	renderer->setZBuffer(false);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	worldMatrix = renderer->getWorldMatrix();

	shadowMapMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowMap[0]->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), shadowMapMesh->getIndexCount());

	shadowMapMesh2->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowMap[1]->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), shadowMapMesh2->getIndexCount());

	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	if (ImGui::CollapsingHeader("Light 1 Pos", ImGuiTreeNodeFlags_CollapsingHeader)) {
		ImGui::SliderFloat("Light 1 X", &lightPosSlider.x, -100, 100);
		ImGui::SliderFloat("Light 1 Y", &lightPosSlider.y, 0, 50);
		ImGui::SliderFloat("Light 1 Z", &lightPosSlider.z, -100, 100);
	}

	if (ImGui::CollapsingHeader("Light 1 Dir", ImGuiTreeNodeFlags_CollapsingHeader)) {
		ImGui::SliderFloat("Light Direction X", &lightDirSlider.x, -1, 1);
		ImGui::SliderFloat("Light Direction Y", &lightDirSlider.y, -1, 1);
		ImGui::SliderFloat("Light Direction Z", &lightDirSlider.z, -1, 1);
	}

	//ImGui::SliderFloat3("Light pos", (&lightPosSlider.x, &lightPosSlider.y, &lightPosSlider.z), -50, 50);


	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


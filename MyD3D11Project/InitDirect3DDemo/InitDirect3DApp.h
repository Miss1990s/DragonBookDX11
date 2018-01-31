#ifndef INITDIRECT3DAPP_H
#define INITDIRECT3DAPP_H

#include "D3DApp.h"

class InitDirect3DApp : public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInstance);
	~InitDirect3DApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
};

#endif//INITDIRECT3DAPP_H

// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <mutex>

#include "scorch/Client.h"
#include "scorch/navigation/Navigation.h"

static std::once_flag initializedFlag;
static HMODULE gModule = NULL;

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = NULL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static WNDPROC oWndProc = NULL;

LRESULT CALLBACK hkWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) > 0)
		return 1L;
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

int exitThread(HMODULE hModule)
{
	Navigation::Shutdown();
	Client::Shutdown();
	FreeLibraryAndExitThread(hModule, TRUE);
	return TRUE;
}

long __stdcall hkPresent11(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	std::call_once(initializedFlag, [pSwapChain]() { 
		DXGI_SWAP_CHAIN_DESC desc;
		pSwapChain->GetDesc(&desc);

		ID3D11Device* device = nullptr;
		pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);

		ID3D11DeviceContext* context = nullptr;
		device->GetImmediateContext(&context);

		ImGui::CreateContext();

		oWndProc = (WNDPROC)::SetWindowLongPtr(desc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)hkWindowProc);
		ImGui_ImplWin32_Init(desc.OutputWindow);
		ImGui_ImplDX11_Init(device, context);

		Client::Initialize();
	});

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Console");
	{
		if (ImGui::Button("Detach")) 
		{
			ImGui::End();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			// Restore WndProc
			DXGI_SWAP_CHAIN_DESC desc;
			pSwapChain->GetDesc(&desc);
			(WNDPROC)::SetWindowLongPtr(desc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);

			// Shutdown ImGui
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();

			// Kiero unbind...
			kiero::unbind(8);

			// Spawn thread and release library
			CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)exitThread, gModule, 0, nullptr);
			return oPresent(pSwapChain, SyncInterval, Flags);
		}

		Client::Render();
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

#pragma warning(push)
#pragma warning(disable:26812)
int entryThread(HMODULE hModule)
{
	gModule = hModule;
	if (kiero::init(kiero::RenderType::Auto) == kiero::Status::Success &&
		kiero::bind(8, (void**)&oPresent, hkPresent11) == kiero::Status::Success)
	{
		return TRUE;
	}

	return FALSE;
}
#pragma warning(pop)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)entryThread, hModule, 0, NULL);
		break;
	}

	return TRUE;
}


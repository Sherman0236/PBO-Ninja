#pragma once
#include <iostream>
#include <d3d11.h>
#include <dxgi.h>
#include <detours/detours.h>
#include "imgui-1.89.5/imgui_single_file.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Render
{
    // typedef for the original present function
    typedef HRESULT(WINAPI* fnPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

    static fnPresent s_fnPresent = NULL;
	static ID3D11Device* s_device = NULL;
	static IDXGISwapChain* s_swapChain = NULL;
    static ID3D11DeviceContext* s_deviceContext = nullptr;
    static ID3D11RenderTargetView* s_renderTargetView = nullptr;
    static HWND s_window = NULL;
    static WNDPROC s_originalWndProc = NULL;

    /// <summary>
    /// Gets the swapchain and hooks the present function
    /// </summary>
    bool Initialize();

    /// <summary>
    /// Gets the swapchain by creating a D3D11 device
    /// </summary>
    bool GetSwapChain();

    /// <summary>
    /// Renders the GUI every frame
    /// </summary>
    HRESULT WINAPI hkD3D11Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);

    /// <summary>
    /// Handles window procedures
    /// </summary>
    LRESULT __stdcall hkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
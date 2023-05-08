#include "Render.hpp"
#include <mutex>
#include "../GUI.hpp"
#include "imgui-1.89.5/imgui_impl_dx11.h"
#include "imgui-1.89.5/imgui_impl_win32.h"
#include "roboto-medium.hpp"

bool Render::Initialize()
{
    if (s_swapChain != NULL)
    {
        return true;
    }

    if (!GetSwapChain() || s_swapChain == nullptr)
    {
        return false;
    }

    // present is the 8th function inside the swapchain's virtual table
    auto swapchainVTable = *reinterpret_cast<uint64_t**>(s_swapChain);
    s_fnPresent = reinterpret_cast<fnPresent>(swapchainVTable[8]);

    // hook present
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(LPVOID&)s_fnPresent, hkD3D11Present);
    DetourTransactionCommit();
    return true;
}

bool Render::GetSwapChain()
{
    // define the swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = 0;
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = GetForegroundWindow();
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    // create the D3D11 device and swap chain
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &s_swapChain, &s_device, NULL, NULL);
    return !FAILED(hr) && s_swapChain != nullptr;
}

HRESULT WINAPI Render::hkD3D11Present(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{
    // only initialize once 
    static std::once_flag initFlag;
    std::call_once(initFlag, [&]
    {
        // get the device from the swapchain
        if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&s_device)))
        {
            throw std::exception("failed to get device from swapchain");
        }

        s_device->GetImmediateContext(&s_deviceContext);

        // get the swapchain description
        DXGI_SWAP_CHAIN_DESC sd;
        swapChain->GetDesc(&sd);
        s_window = sd.OutputWindow;

        // create a render target view using a view description to correct for colors
        ID3D11Texture2D* renderTarget = nullptr;
        if (SUCCEEDED(swapChain->GetBuffer(0, __uuidof(renderTarget), reinterpret_cast<PVOID*>(&renderTarget))))
        {
            D3D11_RENDER_TARGET_VIEW_DESC rendertTargetViewDesc;
            ZeroMemory(&rendertTargetViewDesc, sizeof(rendertTargetViewDesc));
            rendertTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            rendertTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            if (FAILED(s_device->CreateRenderTargetView(renderTarget, &rendertTargetViewDesc, &s_renderTargetView)))
            {
                throw std::exception("failed to create render target view");
            }
            renderTarget->Release();
        }

        // configure ImGui
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.IniFilename = nullptr;
        auto font = io.Fonts->AddFontFromMemoryTTF(&roboto_medium, sizeof(roboto_medium), 16.f);
        if (font == nullptr)
        {
            throw std::exception("failed to load font from memory");
        }

        // initialize ImGui
        ImGui_ImplWin32_Init(s_window);
        ImGui_ImplDX11_Init(s_device, s_deviceContext);
        ImGui_ImplDX11_CreateDeviceObjects();

        // hook the original window procedure
        s_originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(s_window, GWLP_WNDPROC, reinterpret_cast<uintptr_t>(hkWndProc)));
    });

    // toggle the gui
    if (GetAsyncKeyState(VK_INSERT) & 1)
    {
        GUI::s_isOpen = !GUI::s_isOpen;
    }

    // create a new ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // draw the gui
    if (GUI::s_isOpen)
    {
        GUI::OnFrame();
    }

    // end and render the ImGui frame
    ImGui::EndFrame();
    ImGui::Render();
    s_deviceContext->OMSetRenderTargets(1, &s_renderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // call the original present
    return s_fnPresent(swapChain, syncInterval, flags);
}

LRESULT __stdcall Render::hkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // pass the window message to ImGui
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
    {
        return true;
    }

    // pass the message to the original window procedure
    return CallWindowProc(s_originalWndProc, hwnd, uMsg, wParam, lParam);
}
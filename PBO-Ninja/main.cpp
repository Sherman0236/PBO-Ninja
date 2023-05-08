#include <windows.h>
#include <thread>
#include "Render/Render.hpp"
#include "Global.hpp"
#include "Signature.hpp"

/// <summary>
/// Initialize the rendering and global variables
/// </summary>
void Initialize()
{
	// search for GFileBanks
	auto fileBanks = Signature::FindPointer("DayZ_x64.exe", "4C 8B 3D ? ? ? ? 44 8B E0 4A 8B 1C 3E 48");
	if (!fileBanks.has_value())
	{
		Utils::ShowMessageBox("Error", "Failed to find GFileBanks", MB_ICONERROR);
		return;
	}

	// store the address of GFileBanks
	Global::s_fileBanks = reinterpret_cast<Array<QFBank*>*>(*fileBanks);

	// initialize the gui
	if (!Render::Initialize())
	{
		Utils::ShowMessageBox("Error", "Failed to initialize rendering", MB_ICONERROR);
		return;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID pReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// calling AllocConsole from the entrypoint causes an exception to be thrown
		// a thread is created to circumvent this and increase the speed at which execution returns to the injector
		std::thread initThread(Initialize);
		initThread.detach();
	}

	return TRUE;
}
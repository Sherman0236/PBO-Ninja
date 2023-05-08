#pragma once
#include "Global.hpp"

namespace GUI
{
	/// <summary>
	/// Contains basic information about a file's contents
	/// </summary>
	struct FileContentsInfo
	{
		std::string m_fileName = {};

		// the file's unencrypted bytes
		std::vector<uint8_t> m_fileBytes = {};
		
		// if false, the struct is scheduled to be removed from the open files list
		bool m_isOpen = false;

		// cannot use std::once_flag since this struct needs to be copyable
		bool m_windowSizeSet = false;

		FileContentsInfo(std::string fileName, std::vector<uint8_t> bytes) 
			: m_fileName(fileName), m_fileBytes(bytes), m_isOpen(true) { }
	};

	extern bool s_isOpen;
	extern bool s_isPatcherOpen;
	extern bool s_isExecutorOpen;
	extern int s_activeBankIndex;
	extern std::string s_fileSearchFilter;
	extern std::vector<FileContentsInfo> s_openFiles;
	extern std::vector<std::string> s_readableExtensions;

	/// <summary>
	/// Called every frame when the gui is open
	/// </summary>
	void OnFrame();

	void DrawFileContents();

	void DrawMainWindow();
};


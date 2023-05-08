#include "GUI.hpp"
#include <fstream>
#include <ranges>
#include "Render/imgui-1.89.5/imgui_single_file.h"

namespace fs = std::filesystem;

// initialization of static variables
bool GUI::s_isOpen = false;
int GUI::s_activeBankIndex = 0;
std::string GUI::s_fileSearchFilter = "";
std::vector<GUI::FileContentsInfo> GUI::s_openFiles = {};
std::vector <std::string> GUI::s_readableExtensions = {".xml", ".h", ".c", ".hpp", ".cpp", ".txt"};

void GUI::OnFrame()
{
	DrawMainWindow();

	DrawFileContents();
}

void GUI::DrawMainWindow()
{
	// get the list of file banks
	auto banks = Global::s_fileBanks->GetContents();

	// set the window size only once
	static std::once_flag sizeFlag;
	std::call_once(sizeFlag, [] { ImGui::SetNextWindowSize(ImVec2(1200, 600)); });

	if (ImGui::Begin("PBO Ninja", nullptr))
	{
		ImGui::Columns(2);

		// create a left pane that lists the file banks
		if (ImGui::BeginListBox("##LeftPane", ImGui::GetContentRegionAvail()))
		{
			for (int i = 0; i < banks.size(); i++)
			{
				auto& fileBank = banks.at(i);

				if (ImGui::Selectable(fileBank->GetDisplayName().c_str(), s_activeBankIndex == i))
				{
					s_activeBankIndex = i;
				}
			}
			ImGui::EndListBox();
		}

		ImGui::NextColumn();

		// create a right pane that allows viewing/manipulating specific file banks
		if (ImGui::BeginChild("##RightPane"))
		{
			auto selectedBank = banks.at(s_activeBankIndex);

			ImGui::SeparatorText(selectedBank->GetDisplayName().c_str());

			auto prefixString = std::string(selectedBank->prefix()->contents());
			ImGui::SetNextItemWidth(425.f);
			ImGui::InputText("Prefix", (char*)prefixString.c_str(), prefixString.size(), ImGuiInputTextFlags_ReadOnly);

			auto openNameString = std::string(selectedBank->openName()->contents());
			ImGui::SetNextItemWidth(425.f);
			ImGui::InputText("Open Name", (char*)openNameString.c_str(), openNameString.size(), ImGuiInputTextFlags_ReadOnly);

			// display the file bank's various booleans
			ImGui::Checkbox("Locked", selectedBank->locked());
			ImGui::Checkbox("Lockable", selectedBank->lockable());
			ImGui::Checkbox("Verify Signature", selectedBank->verifySignature());
			ImGui::Checkbox("Loaded", selectedBank->loaded());
			ImGui::Checkbox("Error", selectedBank->error());

			// hide the bank
			if (ImGui::Button("Remove"))
			{
				Global::s_fileBanks->Remove(s_activeBankIndex);
			}

			ImGui::SameLine();

			// dump the unencrypted file bank bytes to disk
			if (ImGui::Button("Dump"))
			{
				auto bytes = selectedBank->GetBytes();

				auto fileName = std::format("_{}", selectedBank->GetFileName());
				fileName = fs::path(fileName).replace_extension(".pbo").string();

				std::ofstream outFile(fileName, std::ios::binary);
				outFile.write((char*)bytes.data(), bytes.size());

				auto message = std::format("Dumped 0x{:X} bytes to {} in your game directory\n", bytes.size(), fileName);
				Utils::ShowMessageBox("Success", message);
			}

			// display the file bank's hash
			auto hashString = selectedBank->GetHash();
			ImGui::SeparatorText("Hash");
			ImGui::SetNextItemWidth(425.f);
			ImGui::InputText("##Hash", (char*)hashString.c_str(), hashString.size(), ImGuiInputTextFlags_ReadOnly);

			// display the individual files contained in the file bank
			ImGui::SeparatorText("Files");

			ImGui::SetNextItemWidth(425.f);
			ImGui::InputText("Filter", &s_fileSearchFilter);

			if (ImGui::BeginListBox("##Files", ImGui::GetContentRegionAvail()))
			{
				auto filteredFiles = selectedBank->GetFiles();
				
				// filter the bank's files if the file filter is not empty
				if (!s_fileSearchFilter.empty())
				{
					auto filteredRange = filteredFiles | std::views::filter([](QFBank::FileInfoO info)
					{
						return std::string(info.m_name->contents()).find(s_fileSearchFilter) != std::string::npos;
					});

					filteredFiles = std::vector<QFBank::FileInfoO>(filteredRange.begin(), filteredRange.end());
				}

				for (auto& fileInfo : filteredFiles)
				{
					if (ImGui::Selectable(fileInfo.m_name->contents()))
					{
						// only display human readable files in the file viewer window
						auto fileExtension = fs::path(fileInfo.m_name->contents()).extension().string();
						if (std::find(s_readableExtensions.begin(), s_readableExtensions.end(),
							fileExtension) == s_readableExtensions.end())
						{
							auto message = std::format("The contents of {} files are not human readable\n", fileExtension);
							Utils::ShowMessageBox("Warning", message, MB_ICONWARNING);
							continue;
						}

						std::string fileName = std::format("{}\\{}", selectedBank->GetFileName(),  fileInfo.m_name->contents());

						// check if the file has already been opened
						if (std::find_if(s_openFiles.begin(), s_openFiles.end(), [&](FileContentsInfo file) {
							return file.m_fileName == fileName; }) != s_openFiles.end())
						{
							Utils::ShowMessageBox("Warning", "File already open", MB_ICONWARNING);
							continue;
						}

						// parse the file's bytes from the file bank
						auto fileBytes = selectedBank->GetBytes(fileInfo.m_startOffset, fileInfo.m_length);
						fileBytes.push_back(0); // append a null character to ensure the data stops

						// push the file to the open files vector
						s_openFiles.push_back(FileContentsInfo(fileName, fileBytes));
					}
				}

				ImGui::EndListBox();
			}

			ImGui::EndChild();
		}
	}

	ImGui::End();
}

void GUI::DrawFileContents()
{
	// iterate over all open files
	for (auto it = s_openFiles.begin(); it != s_openFiles.end();)
	{
		auto& contentsInfo = *it;

		// remove any closed files
		if (!contentsInfo.m_isOpen)
		{
			it = s_openFiles.erase(it);
			continue;
		}

		// setup the window size for the first time
		if (!contentsInfo.m_windowSizeSet)
		{
			ImGui::SetNextWindowSize(ImVec2(800, 800));
			contentsInfo.m_windowSizeSet = true;
		}

		// display the file contents
		if (ImGui::Begin(contentsInfo.m_fileName.c_str(), &contentsInfo.m_isOpen))
		{
			ImGui::InputTextMultiline("##FileData", (char*)contentsInfo.m_fileBytes.data(),
				contentsInfo.m_fileBytes.size(), ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_ReadOnly);
		}

		ImGui::End();
		it++;
	}
}
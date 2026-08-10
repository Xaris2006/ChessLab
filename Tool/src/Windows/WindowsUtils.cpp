#include "WindowsUtils.h"


#include <Windows.h>
#include <commdlg.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Walnut/Application.h"

#include <iostream>

static std::string WCharToString(const wchar_t* w)
{
	if (!w) return {};

	int size = WideCharToMultiByte(
		CP_UTF8, 0,
		w, -1,
		nullptr, 0,
		nullptr, nullptr
	);

	std::string str(size, 0);

	WideCharToMultiByte(
		CP_UTF8, 0,
		w, -1,
		&str[0], size,
		nullptr, nullptr
	);

	// Remove Windows null terminator
	if (!str.empty() && str.back() == '\0')
		str.pop_back();

	return str;
}

namespace Windows::Utils
{
	std::string OpenFile(const wchar_t* Filter)
	{
		OPENFILENAME ofn;
		WCHAR szFile[260] = { 0 };
		WCHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(Walnut::Application::Get().GetWindowHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectory(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = Filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		std::string filepath = "";

		if (GetOpenFileName(&ofn) == TRUE)
			filepath = WCharToString(ofn.lpstrFile);
		return filepath;
	}

	std::string SaveFile(const wchar_t* Filter)
	{
		OPENFILENAME ofn;
		WCHAR szFile[260] = { 0 };
		WCHAR currentDir[256] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(Walnut::Application::Get().GetWindowHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		if (GetCurrentDirectory(256, currentDir))
			ofn.lpstrInitialDir = currentDir;

		ofn.lpstrFilter = Filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = L"pgn";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileName(&ofn) == TRUE)
			return WCharToString(ofn.lpstrFile);

		return std::string();
	}

}
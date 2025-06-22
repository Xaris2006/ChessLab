#include "HelpPanel.h"

#include "imgui.h"

#include "Walnut/Application.h"
#include "Walnut/UI/UI.h"

namespace Panels
{
	HelpPanel::HelpPanel()
	{

	}

	void HelpPanel::OnImGuiRender()
	{
		ImGui::Begin("Help");

		ImVec4 bcolor = { 0, 0.66, 0.95, 1 };
		ImVec4 gcolor = { 0.38, 0.67, 0, 1 };
		
		auto helpFunc = [bcolor, gcolor](const std::string& name, const std::string& desc)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30);

				ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
				
				ImGui::Text(name.c_str());

				ImGui::PopStyleColor();
				
				ImGui::SameLine();

				ImGui::PushStyleColor(ImGuiCol_Text, gcolor);

				ImGui::Text(desc.c_str());

				ImGui::PopStyleColor();
			};

		ImGui::BeginTabBar("Help Sections");

		if (ImGui::BeginTabItem("Chess Window"))
		{
			ImGui::NewLine();

			ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

			bool openTree = false;

			openTree = ImGui::TreeNodeEx("Key Shortcuts", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
			
			ImGui::PopFont();
			ImGui::PopStyleColor();

			if(openTree)
			{				
				ImGui::Separator();

				helpFunc("Ctrl + O",			 "Opens a explore window that can choose a file to open.");
				helpFunc("Ctrl + N",			 "Open an empty chess file.");
				helpFunc("Ctrl + S",			 "Save the current chess file.");
				helpFunc("Ctrl + Shilf + S",	 "Save as the current chess file.");
				helpFunc("Ctrl + B",			 "Copy the current position of your board.");
				helpFunc("Ctrl + F",			 "Flip your board.");
				helpFunc("Ctrl + E",			 "Open a window where you can edit your current position.");
				helpFunc("Left Arrow",			 "Go to previous move.");
				helpFunc("Right Arrow",			 "Go to next move.");
				helpFunc("Ctrl + Up Arrow",		 "Open the default chess engine.");
				helpFunc("Ctrl + Down Arrow",	 "Close the chess engine.");

				helpFunc("R key Down + Right Click (or Ctrl + R + Left Click)", "Highlight a block on your board with red.");
				helpFunc("G key Down + Right Click (or Ctrl + G + Left Click)", "Highlight a block on your board with green.");
				helpFunc("B key Down + Right Click (or Ctrl + B + Left Click)", "Highlight a block on your board with blue.");

				helpFunc("R key Down + Right Down and Released (or the same but Left and Ctrl)", "Draw a red arrow on your board from the first click to the release of the mouse button.");
				helpFunc("G key Down + Right Down and Released (or the same but Left and Ctrl)", "Draw a green arrow on your board from the first click to the release of the mouse button.");
				helpFunc("B key Down + Right Down and Released (or the same but Left and Ctrl)", "Draw a blue arrow on your board from the first click to the release of the mouse button.");

				ImGui::Separator();

				ImGui::TreePop();
			}

			openTree = false;

			ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

			openTree = ImGui::TreeNodeEx("Content Browser", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);

			ImGui::PopFont();
			ImGui::PopStyleColor();

			if (openTree)
			{
				ImGui::Separator();

				helpFunc("Double Click", "Open a chess file or a directory.");
				helpFunc("Click on <-",  "Go back a directory.");
				helpFunc("Right Click",  "Open a menu with useful options.");
				helpFunc("Drag",		 "Drag a file to board or to opening book panel to open it.");

				ImGui::Separator();

				ImGui::TreePop();
			}
			
			openTree = false;

			ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

			openTree = ImGui::TreeNodeEx("Moves", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);

			ImGui::PopFont();
			ImGui::PopStyleColor();

			if (openTree)
			{
				ImGui::Separator();

				helpFunc("Raw tab",				  "Just present moves a simble format.");
				helpFunc("Training tab",		  "Present only the previous move.");
				helpFunc("Left Click on a move",  "Go to selected position.");
				helpFunc("Right Click on a move", "Open a menu with useful options.");
				helpFunc("Notes",                 "To read or write notes for a move, click view->notes that will open the note panel.");

				ImGui::Separator();

				ImGui::TreePop();
			}

			openTree = false;

			ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

			openTree = ImGui::TreeNodeEx("Database", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);

			ImGui::PopFont();
			ImGui::PopStyleColor();

			if (openTree)
			{
				ImGui::Separator();

				helpFunc("Double Click",	   "Open the selected chess game.");
				helpFunc("New Game Button",	   "Creates a new game in file.");
				helpFunc("Delete Game Button", "Marks the selected game as deleted. To delete the marked games you should close the file and then delete the games throuth the file menu by right clicking the file on main browser.");

				ImGui::Separator();

				ImGui::TreePop();
			}
			
			openTree = false;

			ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

			openTree = ImGui::TreeNodeEx("Opening Book", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);

			ImGui::PopFont();
			ImGui::PopStyleColor();

			if (openTree)
			{
				ImGui::Separator();

				helpFunc("Create Opening Book file", "Select a chess file from ... or drag it from content browser.");
				helpFunc("Open Opening Book file",	 "Select a Chess Opening Book file from ... or drag it from content browser.");
				helpFunc("Green",					 "Persentage of the games that white won.");
				helpFunc("Blue",					 "Persentage of the games that the result was draw.");
				helpFunc("Red",						 "Persentage of the games that black won.");

				ImGui::Separator();

				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Browser"))
		{
			//ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			//ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));
			//
			//bool openTree = false;
			//
			//openTree = ImGui::TreeNodeEx("Content Browser", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
			//
			//ImGui::PopFont();
			//ImGui::PopStyleColor();

			if (true)
			{
				ImGui::Separator();

				helpFunc("Double Click", "Open a chess file or a directory.");
				helpFunc("Click on <-", "Go back a directory.");
				helpFunc("Right Click", "Open a menu with useful options.");

				ImGui::Separator();

				//ImGui::TreePop();
			}

			ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

			auto openTree = ImGui::TreeNodeEx("Merge Chess Files", ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);

			ImGui::PopFont();
			ImGui::PopStyleColor();

			if (openTree)
			{
				ImGui::Separator();

				helpFunc("Click merge on the option menu", "It openes Merge Window.");
				helpFunc("Drag file and Drop to Merge Window", "Add the file to the file that everything will be merged into.");
				helpFunc("Drag file and Drop to file", "Merges the first file into the second one.");

				ImGui::Separator();

				ImGui::TreePop();
			}

			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Profile"))
		{
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Web"))
		{
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Tools"))
		{
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();
	}
}
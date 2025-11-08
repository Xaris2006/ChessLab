-- premake5.lua
workspace "ChessLab"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "ChessLab-Board"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus",  "/utf-8", "/Zc:char8_t-"  }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Vendor"
   include "vendor/ChessAPI"
   include "vendor/Web"
   include "vendor/xxHash"
group ""

include "Build-Walnut-External.lua"
include "ChessLab-Board/Build-ChessLab-Board.lua"
include "Tool/Build-Tool-App.lua"
include "ChessLab-Lobby/Build-ChessLab-Lobby.lua"
include "Updater/premake5.lua"
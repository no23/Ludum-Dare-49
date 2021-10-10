local fmodIncPath = "C:\\Program Files (x86)\\FMOD SoundSystem\\FMOD Studio API Windows\\api\\core\\inc"
local fmodLibPath = "C:\\Program Files (x86)\\FMOD SoundSystem\\FMOD Studio API Windows\\api\\core\\lib\\x64"
local fmodLib = "fmod_vc.lib"
local assetDir = "..\\Assets"

workspace "Unstable"
	configurations { "Debug", "Release" }
	startproject "Unstable"

staticruntime "on"

filter "configurations:Debug"
	architecture "x86_64"
	defines { "DEBUG", "WINDOWS_X64" }
	symbols "On"

filter "configurations:Release"
	architecture "x86_64"
	defines { "WINDOWS_X64" }

project "Unstable"
	kind "WindowedApp"
	targetdir "Build/output/%{cfg.buildcfg}"
	debugdir "build/output/%{cfg.buildcfg}"
	location "Unstable"
	defines { "SFML_STATIC" }
	cppdialect "C++17"

	files { 
		"Unstable/src/*.h", 
		"Unstable/src/*.c", 
		"Unstable/src/*.hpp", 
		"Unstable/src/*.cpp",

		"Unstable/external/remotery/lib/Remotery.c",
		"Unstable/external/remotery/lib/Remotery.h",

		"Unstable/external/fastnoise/src/fastnoise.h",

		"Unstable/external/fastnoise/src/fastnoise.h",

		"Unstable/external/scheme/src/*.h",
		"Unstable/external/scheme/src/*.c",

		"Unstable/external/Flecs-2.4.7/src/*.c",
		"Unstable/external/Flecs-2.4.7/src/*.h",
	}

	vpaths {
		["Headers/*"] = { "Unstable/src/**.h", "Unstable/src/**.hpp" },
		["Source/*"] = { "Unstable/src/**.c", "Unstable/src/**.cpp" },
	}

	includedirs {
		"Unstable/external/fastnoise/src",
		"Unstable/external/scheme/src",
		"Unstable/external/remotery/lib",
		"Unstable/External/SFML-2.5.1/include",
		"Unstable/external/Flecs-2.4.7/src",
		"Unstable/external/Box2D-2.4.1/include",
		"$(WindowsSDK_IncludePath)",
		fmodIncPath
	}

	libdirs {
		"Unstable/External/SFML-2.5.1/extlibs/libs-msvc/x64",
		fmodLibPath
	}

	links {
		"winmm.lib",
		"opengl32.lib",
		"SFML",
		"lzma",
		"Box2D",
		"freetype.lib",
		fmodLib
	}

	postbuildcommands {
		"copy \"" .. fmodLibPath .. "\\fmod.dll\" \"%{cfg.targetdir}\\fmod.dll\"",
		"robocopy /E " .. assetDir .." %{cfg.targetdir}\\Assets",
		"if %errorlevel% leq 1 exit 0 else exit %errorlevel%" --robocopy returns 1 if it copied anything
	}

group "External"
project "lzma"
	kind "StaticLib"
	language "C"
	compileas "C"
	targetdir "build/lib/%{cfg.buildcfg}"
	buildoptions "/std:c17"
	location "Unstable/External/lzma/"

	files { "Unstable/External/lzma/C/*.h", "Unstable/External/lzma/C/*.c" }
	vpaths {
		["Headers/*"] = "Unstable/External/lzma/C/**.h",
		["Source/*"] = "Unstable/External/lzma/C/**.c"
	}

project "SFML"
	kind "StaticLib"
	targetdir "build/lib/%{cfg.buildcfg}"
	location "Unstable/External/SFML-2.5.1/"
	defines { "WIN32", "_WINDOWS", "SFML_STATIC", "STBI_FAILURE_USERMSG" }

	includedirs {
		"Unstable/External/SFML-2.5.1/src",
		"Unstable/External/SFML-2.5.1/include",

		"Unstable/External/SFML-2.5.1/extlibs/headers/stb_image",
		"Unstable/External/SFML-2.5.1/extlibs/headers/freetype2"
	}

	files {
		"Unstable/External/SFML-2.5.1/include/SFML/Config.hpp",
		"Unstable/External/SFML-2.5.1/include/SFML/OpenGL.hpp",
		"Unstable/External/SFML-2.5.1/include/SFML/GpuPreference.hpp",
		"Unstable/External/SFML-2.5.1/include/SFML/System.hpp",
		"Unstable/External/SFML-2.5.1/include/SFML/Main.hpp",
		"Unstable/External/SFML-2.5.1/include/SFML/Graphics.hpp",
		"Unstable/External/SFML-2.5.1/include/SFML/Window.hpp",

		--Gaphics
		"Unstable/External/SFML-2.5.1/include/SFML/Graphics/**.hpp", 
		"Unstable/External/SFML-2.5.1/include/SFML/Graphics/**.inl",
		"Unstable/External/SFML-2.5.1/src/SFML/Graphics/**.hpp",
		"Unstable/External/SFML-2.5.1/src/SFML/Graphics/**.cpp",

		--Main
		"Unstable/External/SFML-2.5.1/src/SFML/Main/MainWin32.cpp", --Main

		--System
		"Unstable/External/SFML-2.5.1/include/SFML/System/**.hpp", 
		"Unstable/External/SFML-2.5.1/include/SFML/System/**.inl", 
		"Unstable/External/SFML-2.5.1/src/SFML/System/*.hpp", 
		"Unstable/External/SFML-2.5.1/src/SFML/System/Win32/**.hpp",
		"Unstable/External/SFML-2.5.1/src/SFML/System/*.cpp", 
		"Unstable/External/SFML-2.5.1/src/SFML/System/Win32/**.cpp",

		--Window
		"Unstable/External/SFML-2.5.1/include/SFML/Window/**.hpp", 
		"Unstable/External/SFML-2.5.1/src/SFML/Window/*.hpp", 
		"Unstable/External/SFML-2.5.1/src/SFML/Window/*.cpp", 
		"Unstable/External/SFML-2.5.1/src/SFML/Window/Win32/**.hpp", 
		"Unstable/External/SFML-2.5.1/src/SFML/Window/Win32/**.cpp"
	}

	removefiles {
		"Unstable/External/SFML-2.5.1/src/SFML/Window/EGL*.hpp", 
		"Unstable/External/SFML-2.5.1/src/SFML/Window/EGL*.cpp"
	}

project "Box2D"
	kind "StaticLib"
	targetdir "build/lib/%{cfg.buildcfg}"
	location "Unstable/External/Box2D-2.4.1/"
	defines { "WIN32", "_WINDOWS" }

	includedirs{
		"Unstable/External/Box2D-2.4.1/src",
		"Unstable/External/Box2D-2.4.1/include",
	}

	files{
		"Unstable/External/Box2D-2.4.1/src/**.cpp",
		"Unstable/External/Box2D-2.4.1/src/**.h",
		"Unstable/External/Box2D-2.4.1/include/**.h",
	}
workspaceName = "ART"
baseName = path.getbasename(path.getdirectory(os.getcwd()));

if (os.isdir('build_files') == false) then
    os.mkdir('build_files')
end

workspace(workspaceName)
location "../"
configurations { "Debug", "Release", "Test" }
platforms { "x64", "ARM64" }

warnings "Extra"

defaultplatform("x64")

filter { "toolset:gcc or toolset:clang" }
    buildoptions {
        "-Werror",
        "-Wredundant-decls",
        "-Wcast-align",
        "-Wconversion",
        "-Wno-sign-conversion",
        "-Wdouble-promotion",
        "-Wno-unused-parameter",
        "-Wno-unused-function",
        "-Wuninitialized",
        "-pedantic",
    }

    -- filter { "configurations:Test" }
    --     buildoptions {
    --         "--coverage"
    --     }

filter {}

filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"
    optimize "Off"

filter "configurations:Release"
    defines { "NDEBUG" }
    symbols "Off"
    optimize "On"

filter { "platforms:x64" }
architecture "x86_64"

filter { "platforms:Arm64" }
architecture "ARM64"

filter {}

targetdir "bin/%{cfg.buildcfg}/"

startproject(workspaceName)

project(workspaceName)
kind "ConsoleApp"
location "./"
targetdir "../bin/%{cfg.buildcfg}"

filter {}
    files
    {
        path.getdirectory(os.getcwd()) .. "/external/**.cpp",
        path.getdirectory(os.getcwd()) .. "/external/**.h",
        path.getdirectory(os.getcwd()) .. "/external/**.hpp",
        path.getdirectory(os.getcwd()) .. "/lib/**.cpp",
        path.getdirectory(os.getcwd()) .. "/lib/**.h",
        path.getdirectory(os.getcwd()) .. "/include/**.h",
    }

    includedirs
    {
        path.getdirectory(os.getcwd()) .. "/external",
        path.getdirectory(os.getcwd()) .. "/lib",
        path.getdirectory(os.getcwd()) .. "/include",
    }

filter {"configurations:Debug or Release"}
    files
    {
        path.getdirectory(os.getcwd()) .. "/src/**.cpp",
        path.getdirectory(os.getcwd()) .. "/src/**.h"
    }

    includedirs
    {
        path.getdirectory(os.getcwd()) .. "/src"
    }

filter {}

filter {"configurations:Test"}
    files
    {
        path.getdirectory(os.getcwd()) .. "/tests/**.cpp",
        path.getdirectory(os.getcwd()) .. "/tests/**.h"
    }

    includedirs
    {
        path.getdirectory(os.getcwd()) .. "/tests"
    }

filter {}

cdialect "C17"
cppdialect "C++17"

filter {}

-- postbuildcommands
-- {
--     "{COPYDIR} %[../assets] %[../bin/%{cfg.buildcfg}/assets]"
-- }

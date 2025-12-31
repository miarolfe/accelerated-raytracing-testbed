workspaceName = "ART"
baseName = path.getbasename(path.getdirectory(os.getcwd()));

if (os.isdir('build_files') == false) then
    os.mkdir('build_files')
end

workspace(workspaceName)
location "../"
configurations { "Debug", "Release" }
platforms { "x64", "ARM64" }

defaultplatform("x64")

filter "configurations:Debug"
defines { "DEBUG" }
symbols "On"

filter "configurations:Release"
defines { "NDEBUG" }
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
location "build_files/"
targetdir "../bin/%{cfg.buildcfg}"

filter {}

vpaths
{
    ["Header Files/*"] = { "../lib/**.h", "../include/**.h", "../src/**.h" },
    ["Source Files/*"] = { "../lib/**.cpp", "src/**.cpp" },
}

files
{
    "../lib/**.cpp",
    "../lib/**.h",
    "../include/**.h",
    "../src/**.cpp",
    "../src/**.h"
}

filter {}

includedirs { "../src" }
includedirs { "../include" }

cdialect "C17"
cppdialect "C++17"

filter {}

-- postbuildcommands
-- {
--     "{COPYDIR} %[../assets] %[../bin/%{cfg.buildcfg}/assets]"
-- }

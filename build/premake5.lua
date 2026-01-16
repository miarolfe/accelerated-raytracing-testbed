workspaceName = "ART"
baseName = path.getbasename(path.getdirectory(os.getcwd()))

if not os.isdir("build_files") then
    os.mkdir("build_files")
end

workspace(workspaceName)
location "../"
configurations {
    "Debug_Headless",
    "Debug_GUI",
    "Release_Headless",
    "Release_GUI",
    "Test"
}

filter "configurations:*_GUI"
    defines { "ART_GUI" }

filter "configurations:*_Headless"
    defines { "ART_HEADLESS" }

filter {}

platforms { "x64", "ARM64" }

defaultplatform "x64"

filter { "toolset:gcc or toolset:clang" }
    buildoptions {
        "-Werror",
        "-Wredundant-decls",
        "-Wcast-align",
        "-Wno-sign-conversion",
        "-Wdouble-promotion",
        "-Wno-unused-parameter",
        "-Wno-unused-function",
        "-Wuninitialized",
        "-pedantic",
        "-fopenmp"
    }

    linkoptions {
        "-fopenmp"
    }

filter {}

filter { "toolset:msc" }
    buildoptions {
        "/W4",
        "/WX",
        "/permissive-",
        "/openmp",
        "/MP",
    }

    linkoptions {
        "/openmp",
        "/MP",
    }

filter {}

filter "configurations:Debug_*"
    defines { "DEBUG" }
    symbols "On"
    optimize "Off"
    runtime "Debug"

filter "configurations:Release_*"
    defines { "NDEBUG" }
    symbols "Off"
    optimize "On"
    runtime "Release"

filter "configurations:Test"
    runtime "Debug"

filter {}

filter "platforms:x64"
    architecture "x86_64"

filter "platforms:ARM64"
    architecture "ARM64"

filter {}

startproject(workspaceName)

project(workspaceName)
    kind "ConsoleApp"
    location "./"
    targetdir "../bin/%{cfg.buildcfg}"

    files {
        path.getdirectory(os.getcwd()) .. "/external/**.cpp",
        path.getdirectory(os.getcwd()) .. "/external/**.h",
        path.getdirectory(os.getcwd()) .. "/external/**.hpp",
        path.getdirectory(os.getcwd()) .. "/lib/**.cpp",
        path.getdirectory(os.getcwd()) .. "/lib/**.h",
        path.getdirectory(os.getcwd()) .. "/include/**.h",
    }

filter { "files:**/stb_image_impl.cpp", "toolset:msc" }
    disablewarnings { "4100", "4244", "4267", "4389", "4505" }

filter { "files:**/stb_image_write_impl.cpp", "toolset:msc" }
    disablewarnings { "4100", "4244", "4267", "4389", "4505" }

filter { "files:**/stb_image_impl.cpp", "toolset:gcc or toolset:clang" }
    buildoptions { "-w", "-Wno-error" }

filter { "files:**/stb_image_write_impl.cpp", "toolset:gcc or toolset:clang" }
    buildoptions { "-w", "-Wno-error" }

filter {}

externalincludedirs {
    path.getdirectory(os.getcwd()) .. "/external",
    path.getdirectory(os.getcwd()) .. "/external/SDL3/include",
    path.getdirectory(os.getcwd()) .. "/lib",
    path.getdirectory(os.getcwd()) .. "/include",
}

filter { "system:windows", "platforms:x64", "configurations:*_GUI" }
    libdirs { path.getdirectory(os.getcwd()) .. "/external/SDL3/lib/windows/x64" }
    links { "SDL3" }
    postbuildcommands {
        '{COPYFILE} "%{wks.location}/external/SDL3/lib/windows/x64/SDL3.dll" "%{cfg.targetdir}"'
    }

filter { "system:windows", "platforms:ARM64", "configurations:*_GUI" }
    libdirs { path.getdirectory(os.getcwd()) .. "/external/SDL3/lib/windows/arm64" }
    links { "SDL3" }
    postbuildcommands {
        '{COPYFILE} "%{wks.location}/external/SDL3/lib/windows/arm64/SDL3.dll" "%{cfg.targetdir}"'
    }

filter { "system:linux", "configurations:*_GUI" }
    links { "SDL3" }

filter {}

filter { "configurations:Debug_* or Release_*" }
    files {
        path.getdirectory(os.getcwd()) .. "/src/**.cpp",
        path.getdirectory(os.getcwd()) .. "/src/**.h",
    }

    externalincludedirs {
        path.getdirectory(os.getcwd()) .. "/src",
        path.getdirectory(os.getcwd()) .. "/src/common",
        path.getdirectory(os.getcwd()) .. "/src/gui",
        path.getdirectory(os.getcwd()) .. "/src/headless",
    }

filter {}

filter { "configurations:Test" }
    files {
        path.getdirectory(os.getcwd()) .. "/tests/**.cpp",
        path.getdirectory(os.getcwd()) .. "/tests/**.h",
    }

    externalincludedirs {
        path.getdirectory(os.getcwd()) .. "/tests"
    }

filter {}

cdialect "C17"
cppdialect "C++17"

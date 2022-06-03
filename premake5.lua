workspace "In.spire"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    outputdir = "%{cfg.buildcfg}_%{cfg.system}_%{cfg.architecture}"

    project "In.spire"
        location "In.spire"
        kind "SharedLib"
        language "C++"

        targetdir ("_bin/" .. outputdir .. "/%{prj.name}")
        objdir ("_obj/" .. outputdir .. "/%{prj.name}")
        
        pchheader "inpch.h"
        pchsource "In.spire/src/inpch.cpp"

        files
        {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.cpp"
        }

        includedirs
        {
            "thirdparty/spdlog/include",
            "thirdparty/asio/asio/include"
        }

        filter "system:windows"
            cppdialect "C++17"
            staticruntime "On"
            systemversion "latest"
            buildoptions "/MDd"

            defines
            {
                "PLATFORM_WINDOWS",
                "DLL_BUILD"
            }

            postbuildcommands
            {
                ("{COPY} %{cfg.buildtarget.relpath} ../_bin/" .. outputdir .. "/SandboxServer"),
                ("{COPY} %{cfg.buildtarget.relpath} ../_bin/" .. outputdir .. "/SandboxClient")
            }

        filter "configurations:Debug"
            defines "IS_DEBUG"
            symbols "On"

        filter "configurations:Release"
            defines "IS_RELEASE"
            optimize "On"

        filter "configurations:Dist"
            defines "IS_DIST"
            optimize "On"

project "SandboxServer"
    location "SandboxServer"
    kind "ConsoleApp"
    language "C++"

    targetdir ("_bin/" .. outputdir .. "/%{prj.name}")
    objdir ("_obj/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "thirdparty/asio/asio/include",
        "In.spire/src"
    }

    links
    {
        "In.spire"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"
        buildoptions "/MDd"

    filter "configurations:Debug"
        defines "IS_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "IS_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "IS_DIST"
        optimize "On"
        
project "SandboxClient"
    location "SandboxClient"
    kind "ConsoleApp"
    language "C++"

    targetdir ("_bin/" .. outputdir .. "/%{prj.name}")
    objdir ("_obj/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "thirdparty/asio/asio/include",
        "In.spire/src"
    }

    links
    {
        "In.spire"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"
        buildoptions "/MDd"

    filter "configurations:Debug"
        defines "IS_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "IS_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "IS_DIST"
        optimize "On"
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
        kind "ConsoleApp"
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
            "%{prj.name}/thirdparty/spdlog/include"
        }

        filter "system:windows"
            cppdialect "C++17"
            staticruntime "On"
            systemversion "latest"

        filter "configurations:Debug"
            defines "IS_DEBUG"
            symbols "On"

        filter "configurations:Release"
            defines "IS_RELEASE"
            optimize "On"

        filter "configurations:Dist"
            defines "IS_DIST"
            optimize "On"
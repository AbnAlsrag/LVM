workspace "LVM"
    configurations { "Debug", "Dev", "Release" }

    architecture "x64"
    startproject "LVM"
    compileas "C"

    
    output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    
    -- //LVM// --
    project "LVM"
        warnings "Extra"
        location "LVM"
        kind "ConsoleApp"
        language "C"
        
        targetdir ("bin/" .. output_dir .. "/%{prj.name}")
        objdir ("bin-int/" .. output_dir .. "/%{prj.name}")
    
        files {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.c",
        }

        includedirs {
            "LVM/src"
        }
        
        filter "system:windows"
            systemversion "latest"
            
            defines {
                "LVM_PLATFORM_WINDOWS"
            }
 
        filter "configurations:Debug"
            defines { "LVM_Debug" }
            boundscheck "On"
            symbols "On"
            
        filter "configurations:Dev"
            defines { "LVM_Dev" }
            boundscheck "Off"
            optimize "Debug"

        filter "configurations:Release"
            defines { "LVM_Release" }
            boundscheck "Off"
            optimize "Full"

    -- //LDB// --
    project "LDB"
        warnings "Extra"
        location "LDB"
        kind "ConsoleApp"
        language "C"
        
        targetdir ("bin/" .. output_dir .. "/%{prj.name}")
        objdir ("bin-int/" .. output_dir .. "/%{prj.name}")
    
        files {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.c",
        }

        includedirs {
            "LDB/src"
        }
        
        filter "system:windows"
            systemversion "latest"
            
            defines {
                "LVM_PLATFORM_WINDOWS"
            }
 
        filter "configurations:Debug"
            defines { "LVM_Debug" }
            boundscheck "On"
            symbols "On"
            
        filter "configurations:Dev"
            defines { "LVM_Dev" }
            boundscheck "Off"
            optimize "Debug"

        filter "configurations:Release"
            defines { "LVM_Release" }
            boundscheck "Off"
            optimize "Full"
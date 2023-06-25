workspace "LVM"
    configurations { "Debug", "Dev", "Release" }

    architecture "x64"
    startproject "LVME"
    compileas "C"
    
    output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    
    -- //LVM// --
    project "LVM"
        warnings "Default"
        location "LVM"
        kind "None"
        language "C"
        
        files {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.c",
        }

    -- //LVME// --
    project "LVME"
        warnings "Default"
        location "LVME"
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
        warnings "Default"
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
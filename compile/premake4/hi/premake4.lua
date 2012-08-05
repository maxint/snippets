solution "MySolution"
    configurations { "Debug", "Release" }

    project "MyApp"
        kind "ConsoleApp"
        language "C++"
        files { "**.h", "**.cpp" }

        configuration "Debug"
            defines { "DEBUG" }
            flags { "Symbols" }

        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }


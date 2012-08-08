-- libfixmath
-- Cross Platform Fixed Point Maths Library 
-- http://code.google.com/p/libfixmath/
solution "libfixmath"
    configurations { "Debug", "Release" }
    location "_build"

    configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize" }

    configuration {}

    project "libfixmath"
        kind "StaticLib"
        language "C++"
        files { "src/**.h", "src/**.cpp", "src/**.c" }

    project "test"
        kind "ConsoleApp"
        language "C++"
        files { "test/**.h", "test/**.c*" }
        includedirs { "src" }
        links { "libfixmath" }

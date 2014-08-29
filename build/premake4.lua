function setTargetObjDir(outDir)
    for _, cfg in ipairs(configurations()) do
        for _, plat in ipairs(platforms()) do
            local action = _ACTION or ""
            
            local prj = project()
            
            --"_debug_win32_vs2008"
            local suffix = "_" .. cfg .. "_" .. plat .. "_" .. action
            
            targetPath = outDir
            
            suffix = string.lower(suffix)

            local obj_path = "../intermediate/" .. cfg .. "/" .. action .. "/" .. prj.name
            
            obj_path = string.lower(obj_path)
            
            configuration {cfg, plat}
                targetdir(targetPath)
                objdir(obj_path)
                targetsuffix(suffix)
        end
    end
end

function linkLib(libBaseName)
    for _, cfg in ipairs(configurations()) do
        for _, plat in ipairs(platforms()) do
            local action = _ACTION or ""
            
            local prj = project()
            
            local cfgName = cfg
            
            --"_debug_win32_vs2008"
            local suffix = "_" .. cfgName .. "_" .. plat .. "_" .. action
            
            libFullName = libBaseName .. string.lower(suffix)
            
            configuration {cfg, plat}
                links(libFullName)
        end
    end
end

solution "test"
    configurations { "debug", "release" }
    platforms { "x32", "x64" }

    location ("./" .. (_ACTION or ""))
    language "C++"
    flags { "ExtraWarnings" }
    
    configuration "debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    configuration "release"
        defines { "NDEBUG" }
        flags { "Optimize" }

    configuration "vs*"
        defines { "_CRT_SECURE_NO_WARNINGS" }
        
    configuration "gmake"
        buildoptions "-msse4.2 -Wall -Wextra"

    project "gtest"
        kind "StaticLib"
        
        defines { "GTEST_HAS_PTHREAD=0" }

        files { 
            "../thirdparty/gtest/src/gtest-all.cc",
            "../thirdparty/gtest/src/**.h",
        }

        includedirs {
            "../thirdparty/gtest/",
            "../thirdparty/gtest/include",
        }

        setTargetObjDir("../thirdparty/lib")

    project "unittest"
        kind "ConsoleApp"
        
        if _ACTION == "gmake" then
            buildoptions "-Werror -Weffc++ -Wswitch-default"
        end

        files { 
            "../include/**.h",
            "../test/unittest/**.cpp",
            "../test/unittest/**.h",
        }
        
        includedirs {
            "../include/",
            "../thirdparty/gtest/include/",
        }

        libdirs "../thirdparty/lib"

        setTargetObjDir("../bin")

        linkLib "gtest"
        links "gtest"
        
    project "perftest"
        kind "ConsoleApp"
        
        files { 
            "../include/**.h",
            "../test/perftest/**.cpp",
            "../test/perftest/**.c",
            "../test/perftest/**.h",
        }
        
        includedirs {
            "../include/",
            "../thirdparty/gtest/include/",
            "../thirdparty/",
            "../thirdparty/jsoncpp/include/",
            "../thirdparty/libjson/",
            "../thirdparty/yajl/include/",
        }

        libdirs "../thirdparty/lib"

        setTargetObjDir("../bin")

        linkLib "gtest"
        links "gtest"

solution "example"
    configurations { "debug", "release" }
    platforms { "x32", "x64" }
    location ("./" .. (_ACTION or ""))
    language "C++"
    flags { "ExtraWarnings" }
    includedirs "../include/"

    configuration "debug"
        defines { "DEBUG" }
        flags { "Symbols" }

    configuration "release"
        defines { "NDEBUG" }
        flags { "Optimize", "EnableSSE2" }

    configuration "vs*"
        defines { "_CRT_SECURE_NO_WARNINGS" }

    configuration "gmake"
        buildoptions "-Werror -Wall -Wextra -Weffc++ -Wswitch-default"

    local examplepaths = os.matchdirs("../example/*")
    for _, examplepath in ipairs(examplepaths) do
        project(path.getname(examplepath))
            kind "ConsoleApp"
            files(examplepath .. "/*")
            setTargetObjDir("../bin")
    end

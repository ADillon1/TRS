local solutionName = "TRS"

-- Directory vars
local dirWorking = "../"
local dirBinary = "../bin"
local dirSource = "../src/"
local dirBuild = "../build/"
local dirObjectFiles = "../temp/"

local flagsRelease = {
  "Symbols",
  "FatalWarnings",
  "Optimize",
  "FloatFast",
  "OptimizeSpeed",
  "NoMinimalRebuild",
  "NoEditAndContinue",
  "C++11"
}

local flagsDebug = {
  "Symbols",
  "FatalWarnings",
  "NoMinimalRebuild",
  "FloatFast",
  "NoEditAndContinue",
  "NoIncrementalLink",
  "C++11"
}

-- Preprocessor defines
local definesDebug = {
  "_CRT_SECURE_NO_WARNINGS",
  "DEBUG"
}

local definesRelease = {
  "AK_OPTIMIZED",
  "_CRT_SECURE_NO_WARNINGS",
  "NDEBUG"
}

-- Clean Function
newaction {
  trigger     = "clean",
  description = "Removes the build directory.",
  execute     = function ()
    os.rmdir(dirBuild)
    os.rmdir(dirObjectFiles)
    os.rmdir(dirBinary)
    print("done.")
  end
}

if (_ACTION ~= nil) then
  solution(solutionName)
  location(dirBuild .. _ACTION)
  configurations {"Debug", "Release"}

  project("unit-tests")
    language("C++")
    kind("ConsoleApp")
    objdir(dirObjectFiles)
    targetdir(dirBinary)
    debugdir(dirBinary)

    files { dirSource .. "**.h", 
            dirSource .. "**.hpp",
            dirSource .. "**.c",
            dirSource .. "**.cpp" 
    }

    configuration "Debug"
      defines { definesDebug }
      flags { flagsDebug }
    
    configuration "Release"
      defines { definesRelease }
      flags { flagsRelease }

end
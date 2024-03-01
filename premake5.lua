-- premake5.lua
workspace "CpuRaytracer"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "CpuRaytracer"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "CpuRaytracerApp"
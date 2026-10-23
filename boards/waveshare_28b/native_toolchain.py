"""Use the same installed Espressif compiler if its Windows alias launcher fails.

The official unified toolchain selects the S3 with xtensa_esp32s3.so.
https://github.com/espressif/xtensa-dynconfig#using
No SDK archives or installed launchers are modified.
"""
Import("env")
import os
from pathlib import Path
import subprocess

if os.name == "nt":
    package = Path(env.PioPlatform().get_package_dir("toolchain-xtensa-esp-elf"))
    alias = package / "bin/xtensa-esp32s3-elf-g++.exe"
    probe = subprocess.run([str(alias), "--version"], capture_output=True, timeout=15)
    if probe.returncode:
        plugin = package / "lib/xtensa_esp32s3.so"
        assert plugin.is_file(), "ESP32-S3 dynconfig plugin missing"
        def tool(name):
            path = package / ("bin/xtensa-esp-elf-" + name + ".exe")
            assert path.is_file(), str(path)
            return path.as_posix()
        # GCC's multilib table matches this basename literally. An absolute
        # -mdynconfig path selects the CPU plugin but misses the S3 runtime libs.
        dynflag = "-mdynconfig=" + plugin.name
        probe_env = dict(os.environ, XTENSA_GNU_CONFIG=plugin.as_posix())
        multilib = subprocess.run([tool("gcc"), dynflag, "-fno-rtti", "-print-multi-directory"],
                                  capture_output=True, text=True, check=True, env=probe_env)
        assert multilib.stdout.strip() == "esp32s3/no-rtti", "Wrong Xtensa runtime library selection"
        def select_tools(build_env, node=None):
            build_env.Replace(CC=tool("gcc"), CXX=tool("g++"), AS=tool("as"),
                              AR=tool("gcc-ar"), RANLIB=tool("gcc-ranlib"), SIZETOOL=tool("size"))
            build_env["ENV"]["XTENSA_GNU_CONFIG"] = plugin.as_posix()
            build_env.AppendUnique(CCFLAGS=[dynflag], LINKFLAGS=[dynflag])
            return node
        # Pre: configure each source environment, including library/variant clones.
        # Post: configure the final link environment after framework setup.
        if not env.get("WAVESHARE_NATIVE_TOOLCHAIN_MIDDLEWARE"):
            env.AddBuildMiddleware(select_tools)
            env["WAVESHARE_NATIVE_TOOLCHAIN_MIDDLEWARE"] = True
        select_tools(env)
        print("[waveshare_28b] Windows alias launcher failed; same unified GCC with explicit ESP32-S3 dynconfig selected.")

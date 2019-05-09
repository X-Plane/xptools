#!/usr/bin/env python3

from distutils.core import setup, Extension

taxi_module = Extension("xplane_taxi_sign",
                        define_macros = [('DEV', '0'), ('REL', '1')],
                        extra_compile_args=['-std=c++11', '-stdlib=libc++', '-mmacosx-version-min=10.7'],
                        include_dirs = ["../Utils", "../WEDCore/"],
                        sources = ["SignParser_Python_Module.cpp", "../Utils/AssertUtils.cpp", "../WEDCore/WED_Sign_Parser.cpp", "../WEDCore/WED_Taxi_Sign.cpp", "SignParser_Canonicalize.cpp"])

setup(name="xplane_taxi_sign",
    version="0.2",
    ext_modules=[taxi_module])


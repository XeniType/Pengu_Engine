@echo off
cd %~dp0..\..

conan install -of build/deps/Release ^ --profile:host=default ^ --profile:build=default_build ^ --build=missing -s build_type=Release -s compiler=msvc -s compiler.version=193 -s compiler.runtime=dynamic -s compiler.cppstd=17 --build=missing src/win64/build
conan install -of build/deps/RelWithDebInfo ^ --profile:host=default ^ --profile:build=default_build -s build_type=RelWithDebInfo -s compiler=msvc -s compiler.version=193 -s compiler.runtime=dynamic -s compiler.cppstd=17 --build=missing src/win64/build
conan install -of build/deps/Debug ^ --profile:host=default ^ --profile:build=default_build -s build_type=Debug -s compiler=msvc -s compiler.version=193 -s compiler.runtime=dynamic -s compiler.cppstd=17 --build=missing src/win64/build
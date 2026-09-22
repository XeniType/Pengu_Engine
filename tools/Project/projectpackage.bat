@echo off
setlocal enabledelayedexpansion
echo =======================================
echo   Pengu Engine - Stage Builder
echo =======================================
echo.

Rem === Destination directory ===
set "DEST=C:\Users\kaeli\Documents\GitHub\Pengu_Engine"

Rem === Source directory (repo root, one level above this script) ===
set "SRC=%~dp0/../.."

Rem === Create folder structure ===
if not exist "%DEST%" (
    echo [+] Creating destination directory...
    mkdir "%DEST%"
)
cd /d "%DEST%"
for %%D in (
    tools
    tools\Project
    tools\Python
    include
    src
    src\win64
    src\win64\build
    lib
    lib\win64
    lib\win64\Release
    lib\win64\Debug
    lib\win64\RelWithDebInfo
    examples
    docs
) do (
    if not exist "%%D" (
        echo [+] Creating %%D...
        mkdir "%%D"
    )
)
if not exist README.MD (
    echo [+] Creating empty README.MD...
    copy /y NUL README.MD > NUL
)
echo.
echo --- Copying files ---
echo.

Rem === Copy Include ===
echo [~] Copying include headers...
xcopy /y /i /e /q "%SRC%\include" include\
if errorlevel 1 ( echo [!] WARNING: Failed to copy include headers ) else ( echo [OK] Include headers copied )

Rem === Copy Libs ===
echo [~] Copying libraries...
for %%C in (Release Debug RelWithDebInfo) do (
    copy /y "%SRC%\build\%%C\Pengu_Engine.lib" "lib\win64\%%C\Pengu_Engine.lib" > NUL
    if errorlevel 1 ( echo [!] WARNING: %%C lib not found, skipping... ) else ( echo [OK] %%C lib copied )
)

Rem === Copy Tools ===
echo [~] Copying tools...
copy /y "%SRC%\tools\Project\gendeps.bat" tools\Project
copy /y "%SRC%\tools\Project\genproject.bat" tools\Project
copy /y "%SRC%\tools\premake5.exe" tools
copy /y "%SRC%\tools\Project\runpm5.bat" tools\Project
if errorlevel 1 ( echo [!] WARNING: Failed to copy tools ) else ( echo [OK] Tools copied )

Rem === Copy Examples ===
echo [~] Copying examples...
xcopy /y /i /e /q "%SRC%\examples" examples\
if errorlevel 1 ( echo [!] WARNING: Failed to copy examples ) else ( echo [OK] Examples copied )

Rem === Copy Premake script ===
echo [~] Copying premake5.lua...
copy /y "%SRC%\stage_premake5.lua" premake5.lua > NUL
if errorlevel 1 ( echo [!] WARNING: stage_premake5.lua not found ) else ( echo [OK] premake5.lua copied )

Rem === Copy Premake script ===
echo [~] Copying requirements.txt...
copy /y "%SRC%\tools\Python\requirements.txt" tools\Python\requirements.txt > NUL
if errorlevel 1 ( echo [!] WARNING: requirements.txt not found ) else ( echo [OK] requirements.txt copied )

Rem === Copy ConanFile ===
copy /y "%SRC%\src\win64\build\conanfile.txt" "src\win64\build\conanfile.txt" > NUL
if errorlevel 1 ( echo [!] WARNING: conanfile.txt not found ) else ( echo [OK] conanfile.txt copied )

Rem === Copy Resources ===
echo [~] Copying resources...
xcopy /y /i /e /q "%SRC%\assets" assets\
if errorlevel 1 ( echo [!] WARNING: Failed to copy Assets ) else ( echo [OK] Assets copied )

Rem === Copy Docs ===
echo [~] Copying Docs...
copy /y "%SRC%\doc\UserMan\README.md" README.md
copy /y "%SRC%\doc\UserMan\EngineGuide.md" docs\
copy /y "%SRC%\doc\UserMan\ConanInstall.md" docs\
copy /y "%SRC%\doc\Diagrams\pengu_engine_architecture.html" docs\
copy /y "%SRC%\doc\Diagrams\pengu_namespace_diagram.html" docs\
if errorlevel 1 ( echo [!] WARNING: Failed to copy docs ) else ( echo [OK] Docs copied )

echo.
echo =======================================
echo   Stage build complete!
echo =======================================
echo.
endlocal
pause
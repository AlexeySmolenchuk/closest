set ARNOLD_ROOT=C:\solidangle\SDK_6.2.1.0
set HFS=C:\Program Files\Side Effects Software\Houdini 18.5.563
set MSVCDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.16.27023

set NAME="closest"

"%HFS%\bin\hcustom" -e -i ./build src/%NAME%.cpp -L %ARNOLD_ROOT%/lib -l ai.lib -I %ARNOLD_ROOT%/include -l libGEO.lib -l libUT.lib

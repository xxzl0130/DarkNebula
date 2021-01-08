SET INSTALL_PREFIX="C:\ProgramData\DarkNebula"

if exist %INSTALL_PREFIX% goto 1
mkdir %INSTALL_PREFIX%
if exist %INSTALL_PREFIX%\include goto 1
mkdir %INSTALL_PREFIX%\include
if exist %INSTALL_PREFIX%\include\DarkNebula goto 1
mkdir %INSTALL_PREFIX%\include\DarkNebula
if exist %INSTALL_PREFIX%\lib goto 1
mkdir %INSTALL_PREFIX%\lib
if exist %INSTALL_PREFIX%\bin goto 1
mkdir %INSTALL_PREFIX%\bin

:1
copy ..\DarkNebula\*.h %INSTALL_PREFIX%\include\DarkNebula\
copy ..\x64\Release\*.lib %INSTALL_PREFIX%\lib\
copy ..\x64\Release\*.dll %INSTALL_PREFIX%\bin\
copy ..\x64\Debug\*.lib %INSTALL_PREFIX%\lib\
copy ..\x64\Debug\DarkNebula*.pdb %INSTALL_PREFIX%\bin\
copy ..\x64\Debug\*.dll %INSTALL_PREFIX%\bin\
copy ..\DarkNebulaSharp\bin\Release\netstandard2.0\*.dll %INSTALL_PREFIX%\bin\
copy ..\DarkNebulaSharp\bin\Release\netstandard2.0\DarkNebula*.pdb %INSTALL_PREFIX%\bin\
rem bat file directory: %~dp0

%~dp0\dfu-programmer.exe %1 erase
%~dp0\dfu-programmer.exe %*
%~dp0\dfu-programmer.exe %1 reset


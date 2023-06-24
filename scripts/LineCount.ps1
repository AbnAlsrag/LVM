cd ..\
(gci -include *.cpp,*.h -recurse | select-string .).Count
pause
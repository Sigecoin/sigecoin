if not exist .build mkdir .build
cd ./.build

rem set QT5_BASE_DIR=C:\Qt\5.5.1\qtbase

@set params=
rem @set params=%params% -DCMAKE_BUILD_TYPE=

cmake -G "Visual Studio 14 2015" %params% ../

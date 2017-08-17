if not exist .build mkdir .build
cd ./.build

set QT5_BASE_DIR=D:\Qt\5.5.1\qtbase
set PROTOBUF_ROOT=E:\SDK\protobuf-2.6.1

@set params=
rem @set params=%params% -DCMAKE_BUILD_TYPE=

cmake -G "Visual Studio 14 2015" %params% ../

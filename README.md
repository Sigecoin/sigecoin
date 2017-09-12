Sigecoin
========

Sigecoin is designed to be a low latency, high throughput cryptocurrency https://sigecoin.net


Build instructions for Windows
MT debug/release, full node, GUI wallet, tests
----------------------------------------------

* Requires MSVC Visual Studio installed not less 2013

* Requires CMake installed (3.8 recommended)

* Requires QT5 open source build MT(d):

  --> get one from archives in https://download.qt.io/archive/qt/5.5/5.5.1/single, unpack it;

  --> prepare OpenSSL precompiled static libraries from here: https://www.npcglib.org/~stathis/blog/precompiled-openssl/ accordingly with version of your Visual Studio;
      1) set an environment variable OPENSSL_ROOT=<openssl root folder> and OPENSSL_ROOT_DIR=<openssl root folder>
      2) MT libraries can be renamed/copied with different names from cryptoMT.lib to crypt32.lib and from libsslMT.lib to libeay32.lib - it may resolve names conflicts in future applications linkage
      
  --> prepare QT build with multi-thread runtimes, by default MD(d), but we need MT(d)

      1) find configuration file ..\5.5.1\qtbase\mkspecs\common\msvc-desktop.conf;
      2) replace all -MD and -MDd on -MT and -MTd;
         eg it must looks like: QMAKE_CFLAGS_RELEASE = -O2 -MT, QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += -O2 -MT -Zi, QMAKE_CFLAGS_DEBUG = -Zi -MTd
      3) find configuration project ..\5.5.1\qt.pro
      4) do comments for all submodules:
         # addModule(qtandroidextras, qtbase)
         # addModule(qtmacextras, qtbase) etc., 
         only base QT module must remains uncomment:   addModule(qtbase)
      5) run MSVC environments from "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat" or from own VS location
      6) set environment variable like QT QTDIR=C:\QT\5.5.1\qtbase
      7) run configuration command in folder on up level from QTDIR (eg $QTDIR\.. or C:\QT\5.5.1) as following;
         configure -prefix %QTDIR% -debug-and-release -static -confirm-license -opensource -platform win32-msvc2015 -make tools -make libs -no-cetest -openssl-linked OPENSSL_LIBS="-lssleay32 -lcrypt32" -nomake tests -nomake examples -no-angle -opengl desktop -D "_BIND_TO_CURRENT_VCLIBS_VERSION=1" -I %OPENSSL_ROOT_DIR%\include
      8) nmake
      9) all QT static libraries compiled here:  %QTDIR%/lib

* Requires Boost open source build MT(d) not earlier than 1.58:

      1) get one from archives from here: https://sourceforge.net/projects/boost/files/boost/1.63.0
      2) run boost_1_63_0\bootstrap.bat and wait for bjam.exe built
      3) set environment like BOOST_ROOT=C:\boost_1_63_0 
      4) run from command line:
         bjam.exe --prefix=%BOOST_ROOT% --build-type-compact -link=static runtime-link=static --with-atomic --with-regex --with-log --with-thread --with-system --with-date_time --with-filesystem --with-program_options --with-signals --with-serialization --with-chrono --with-test --with-context --with-locale --with-coroutine --with-iostreams --with-random
      5) all boost libraries wil be located here %BOOST_ROOT%/stage/lib

* Get Google Protobuf sources (any version) with VS projects for example from here: https://github.com/Microsoft/vsminecraft/tree/master/dependencies/protobuf-2.6.1 (vsprojects folder contains prjs for msvc build)
  Build it debug and release libprotobuf with MT(d) option (switch it in project properties C/C++->Code Generation->Runtime Library)

* Make CMake projects by ..\sigecoin\vs14_32_mt.cmd for VS 2014-2015;
  ...or replace string inside cmd if your VS has later/earler versions accordingly with CMake compilator requirements

* all projects will be generated inside sigecoin/.build folder, and can be opened and compiled by Visual Studio

* Note: current version of node works as local single node with genesis block creation when the following parameters will be specified in C:\Users\<User>\AppData\Roaming\Sigecoin\sigecoin.conf 

rpcuser=
rpcpassword=
rpcallowip=127.0.0.1/0.0.0.0
rpcport=8563
onlynet=ipv4
listen=1
server=1
bind=127.0.0.1
discover=1
dns=1
dnsseed=1
forcednsseed=1
seednode=127.0.0.1
prune=15000

* all command line arguments and RPC commands are similar to Bitcoin Core apps: bitcoind, bitcoin-cli, bitcoin-tx --> sigenode, sigenode-cli, sigenode-tx

* Sigecoin QT wallet has exact functionality as Bitcoin QT Wallet
  Also starts full genesis node on first start if blocks not found and parameters above are specified

* Client node can be started on local machine for another location, for example C:\Client\Sigecoin with following parameters:
  sigenode.exe -datadir=C:\Client\Sigecoin\data\ -conf=C:\Client\Sigecoin\data\sigecoin.conf

and parameters in conf:

rpcuser=
rpcpassword=
rpcallowip=127.0.0.1/0.0.0.0
rpcport=8562                 # RPC port should differs from first node in case when both running on single machine
bind=192.168.1.100           # IP of genesis node in internal routing
onlynet=ipv4
listen=1
server=1
addnode=127.0.0.1



Build instructions for Ubuntu (12.0+)
Without GUI, fullnode, cli, tx, tests.
-----------------------------------------

* Requires CMake installed (3.8 recommended)

* sudo apt-get install build-essential autotools-dev autoconf pkg-config libssl-dev
  sudo apt-get install libboost-all-dev
  sudo apt-get install libevent-dev

* Specify BOOST_ROOT environment variable in case of manual build of BOOST libraries (not earlier 1.58)
* ./unix_build.sh
  cd .build - change directory
  make

* bench, sigenode, sigenode-cli, sigenode-tx, tests executables and static libraries should be compiled in .build directory

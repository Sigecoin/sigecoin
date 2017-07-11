 cmake_minimum_required (VERSION 2.8.11)

macro(configure_msvc_runtime)
  if(MSVC)
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    # Set compiler options.
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${MSVC_RUNTIME} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

project (sige)

set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "0")

set (SIGE_GUI OFF CACHE BOOL "")
set (SIGE_TEST ON CACHE BOOL "")
set (MSVC_RUNTIME "static")


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if (WIN32)
        configure_msvc_runtime()
	set (PLATFORM_COMPILE_FLAGS "-DBOOST_SPIRIT_THREADSAFE -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN /wd4244")
	set (BLAKE2_IMPLEMENTATION "blake2/blake2b.c")
else (WIN32)
	set (PLATFORM_COMPILE_FLAGS "-DBOOST_SPIRIT_THREADSAFE -Werror=switch -fPIC")
	IF (CMAKE_SYSTEM_PROCESSOR MATCHES "^(i.86|x86(_64)?)$")
		set (PLATFORM_COMPILE_FLAGS "${PLATFORM_COMPILE_FLAGS} -msse4")
		set (BLAKE2_IMPLEMENTATION "blake2/blake2b.c")
	else()
		set (BLAKE2_IMPLEMENTATION "blake2/blake2b-ref.c")
	endif()
endif (WIN32)

if (WIN32)
	set (PLATFORM_C_FLAGS "/std=c11")
else (WIN32)
	set (PLATFORM_C_FLAGS "-std=c11")
endif (WIN32)

if (WIN32)
	set (PLATFORM_CXX_FLAGS "/bigobj")
else (WIN32)
	set (PLATFORM_CXX_FLAGS "-std=c++11")
endif (WIN32)

if (WIN32)
	set (PLATFORM_GUI_TARGET_PROPERTIES WIN32)
else (WIN32)
	set (PLATFORM_GUI_TARGET_PROPERTIES "")
endif (WIN32)

if (APPLE)
	set (PLATFORM_LINK_FLAGS "-framework Foundation -framework OpenCL")
elseif (WIN32)
	set (PLATFORM_LINK_FLAGS "")
        set (PLATFORM_SPECIFIC_LIBS WS2_32.lib)
else ()
	set (PLATFORM_LINK_FLAGS "-static-libgcc -static-libstdc++")
endif (APPLE)

find_package(OpenSSL)
if (OPENSSL_FOUND)
	include_directories(${OPENSSL_INCLUDE_DIR})
	set(CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
endif (OPENSSL_FOUND)

if (SIGE_GUI)
	if (WIN32)
		set (PLATFORM_QT_PACKAGES WinExtras)
	else (WIN32)
		set (PLATFORM_QT_PACKAGES)
	endif (WIN32)

	find_package (Qt5 COMPONENTS Core Gui Widgets Test ${PLATFORM_QT_PACKAGES})
	include_directories (${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Test_INCLUDE_DIRS})
endif (SIGE_GUI)

include_directories (${CMAKE_SOURCE_DIR})

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost COMPONENTS 
         date_time 
         system
         log
         log_setup
         thread
         program_options
         regex
         chrono
         atomic
         unit_test_framework
         REQUIRED)

include_directories (${Boost_INCLUDE_DIR})

add_subdirectory (secp256k1)
add_subdirectory (leveldb)
add_subdirectory (libevent)

add_library (lmdb
	liblmdb/lmdb.h
	liblmdb/mdb.c
	liblmdb/midl.c)

add_library (blake2
	blake2/blake2-config.h
	blake2/blake2-impl.h
	blake2/blake2.h
	${BLAKE2_IMPLEMENTATION})

add_library (univalue
        univalue/univalue.cpp
        univalue/univalue_escapes.h
        univalue/univalue_read.cpp
        univalue/univalue_utffilter.h
        univalue/univalue_write.cpp
        univalue/include/univalue.h
        )

set (WINSOCK2_LIBRARY Ws2_32 CACHE STRING "")

add_library (crypto
        sige/crypto/aes.cpp
        sige/crypto/aes.h
        sige/crypto/common.h
        sige/crypto/hmac_sha256.cpp
        sige/crypto/hmac_sha256.h
        sige/crypto/hmac_sha512.cpp
        sige/crypto/hmac_sha512.h
        sige/crypto/ripemd160.cpp
        sige/crypto/ripemd160.h
        sige/crypto/sha1.cpp
        sige/crypto/sha1.h
        sige/crypto/sha256.cpp
        sige/crypto/sha256.h
        sige/crypto/sha512.cpp
        sige/crypto/sha512.h
        sige/crypto/ctaes/bench.c
        sige/crypto/ctaes/ctaes.c
        sige/crypto/ctaes/ctaes.h
        )
set_target_properties(crypto PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS}")
target_include_directories(crypto PRIVATE ${sige_SOURCE_DIR}/sige ${sige_SOURCE_DIR}/sige/src ${sige_SOURCE_DIR}/sige/crypto)

add_library (wallet
        sige/wallet/accounting_entry.cpp
        sige/wallet/accounting_entry.h
        sige/wallet/coincontrol.h
        sige/wallet/crypter.cpp
        sige/wallet/crypter.h
        sige/wallet/db.cpp
        sige/wallet/db.h
        sige/wallet/keypool.cpp
        sige/wallet/keypool.h
        sige/wallet/merkletx.cpp
        sige/wallet/merkletx.h
        sige/wallet/reservekey.cpp
        sige/wallet/reservekey.h
        sige/wallet/rpcdump.cpp
        sige/wallet/rpcwallet.cpp
        sige/wallet/rpcwallet.h
        sige/wallet/wallet.cpp
        sige/wallet/wallet.h
        sige/wallet/wallet_account.cpp
        sige/wallet/wallet_account.h
        sige/wallet/wallet_db.cpp
        sige/wallet/wallet_db.h
        sige/wallet/wallet_key.cpp
        sige/wallet/wallet_key.h
        sige/wallet/wallet_output.cpp
        sige/wallet/wallet_output.h
        sige/wallet/wallet_tx.cpp
        sige/wallet/wallet_tx.h
        sige/wallet/wallet_util.cpp
        sige/wallet/wallet_util.h
        )
set_target_properties(wallet PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS}")
target_include_directories(wallet PRIVATE ${sige_SOURCE_DIR}/sige ${sige_SOURCE_DIR}/sige/src ${sige_SOURCE_DIR}/sige/wallet ${leveldb_SOURCE_DIR} ${leveldb_SOURCE_DIR}/include ${sige_SOURCE_DIR}/univalue)


set (SRC_CORE
        sige/src/compat/byteswap.h
        sige/src/compat/endian.h
        sige/src/compat/glibc_compat.cpp
        sige/src/compat/glibc_sanity.cpp
        sige/src/compat/glibcxx_sanity.cpp
        sige/src/compat/sanity.h
        sige/src/compat/strnlen.cpp 
        sige/src/addrdb.cpp
        sige/src/addrdb.h
        sige/src/addrman.cpp
        sige/src/addrman.h
        sige/src/amount.cpp
        sige/src/amount.h
        sige/src/baseuint.cpp
        sige/src/baseuint.h
        sige/src/base58.cpp
        sige/src/base58.h
        sige/src/base58string.cpp
        sige/src/base58string.h
        sige/src/blockencodings.cpp
        sige/src/blockencodings.h
        sige/src/bloom.cpp
        sige/src/bloom.h
        sige/src/chain.cpp
        sige/src/chain.h
        sige/src/chainparams.cpp
        sige/src/chainparams.h
        sige/src/chainparams_1_mainnet.cpp
        sige/src/chainparams_2_testnet.cpp
        sige/src/chainparams_3_regtest.cpp
        sige/src/chainparamsbase.cpp
        sige/src/chainparamsbase.h
        sige/src/chainparamsseeds.h
        sige/src/chainparamsutil.cpp
        sige/src/chainparamsutil.h
        sige/src/checkpoints.cpp
        sige/src/checkpoints.h
        sige/src/checkqueue.h
        sige/src/clientversion.cpp
        sige/src/clientversion.h
        sige/src/coins.cpp
        sige/src/coins.h
        sige/src/compat.h
        sige/src/compressor.cpp
        sige/src/compressor.h
        sige/src/core_io.h
        sige/src/core_memusage.h
        sige/src/core_read.cpp
        sige/src/core_write.cpp
        sige/src/cuckoocache.h
        sige/src/dbwrapper.cpp
        sige/src/dbwrapper.h
        sige/src/extkey.cpp
        sige/src/extkey.h
        sige/src/extpubkey.cpp
        sige/src/extpubkey.h
        sige/src/hash.cpp
        sige/src/hash.h
        sige/src/httprpc.cpp
        sige/src/httprpc.h
        sige/src/httpserver.cpp
        sige/src/httpserver.h
        sige/src/indirectmap.h
        sige/src/init.cpp
        sige/src/init.h
        sige/src/key.cpp
        sige/src/key.h
        sige/src/keystore.cpp
        sige/src/keystore.h
        sige/src/limitedmap.h
        sige/src/memusage.h
        sige/src/merkleblock.cpp
        sige/src/merkleblock.h
        sige/src/miner.cpp
        sige/src/miner.h
        sige/src/net.cpp
        sige/src/net.h
        sige/src/netaddress.cpp
        sige/src/netaddress.h
        sige/src/netbase.cpp
        sige/src/netbase.h
        sige/src/netmessagemaker.h
        sige/src/netprocessing.cpp
        sige/src/netprocessing.h
        sige/src/noui.cpp
        sige/src/noui.h
        sige/src/pow.cpp
        sige/src/pow.h
        sige/src/prevector.h
        sige/src/protocol.cpp
        sige/src/protocol.h
        sige/src/pubkey.cpp
        sige/src/pubkey.h
        sige/src/pubkeyutil.cpp
        sige/src/pubkeyutil.h
        sige/src/random.cpp
        sige/src/random.h
        sige/src/rest.cpp
        sige/src/reverselock.h
        sige/src/scheduler.cpp
        sige/src/scheduler.h
        sige/src/serialize.h
        sige/src/sigaddress.cpp
        sige/src/sigaddress.h
        sige/src/sigkeybase.cpp
        sige/src/sigkeybase.h
        sige/src/streams.h
        sige/src/sync.cpp
        sige/src/sync.h
        sige/src/threadinterrupt.cpp
        sige/src/threadinterrupt.h
        sige/src/threadsafety.h
        sige/src/timedata.cpp
        sige/src/timedata.h
        sige/src/tinyformat.h
        sige/src/torcontrol.cpp
        sige/src/torcontrol.h
        sige/src/txdb.cpp
        sige/src/txdb.h
        sige/src/txdestination.cpp
        sige/src/txdestination.h
        sige/src/txmempool.cpp
        sige/src/txmempool.h
        sige/src/uint256.cpp
        sige/src/uint256.h
        sige/src/uinterface.cpp
        sige/src/uinterface.h
        sige/src/undo.h
        sige/src/util.cpp
        sige/src/util.h
        sige/src/utilmoneystr.cpp
        sige/src/utilmoneystr.h
        sige/src/utilstrencodings.cpp
        sige/src/utilstrencodings.h
        sige/src/utiltime.cpp
        sige/src/utiltime.h
        sige/src/validation.cpp
        sige/src/validation.h
        sige/src/validationinterface.cpp
        sige/src/validationinterface.h
        sige/src/version.h
        sige/src/versionbits.cpp
        sige/src/versionbits.h
        sige/src/warnings.cpp
        sige/src/warnings.h
        sige/src/wintype.h 
        )

set (SRC_CONSENSUS
        sige/src/consensus/consensus.h
        sige/src/consensus/merkle.cpp
        sige/src/consensus/merkle.h
        sige/src/consensus/params.h
        sige/src/consensus/validation.h 
        )

set (SRC_POLICY
        sige/src/policy/fees.cpp
        sige/src/policy/fees.h
        sige/src/policy/policy.cpp
        sige/src/policy/policy.h
        sige/src/policy/rbf.cpp
        sige/src/policy/rbf.h 
        )

set (SRC_PRIMITIVES
        sige/src/primitives/block.cpp
        sige/src/primitives/block.h
        sige/src/primitives/transaction.cpp
        sige/src/primitives/transaction.h 
        )

set (SRC_RPC
        sige/src/rpc/blockchain.cpp
        sige/src/rpc/client.cpp
        sige/src/rpc/client.h
        sige/src/rpc/mining.cpp
        sige/src/rpc/misc.cpp
        sige/src/rpc/rpcnet.cpp
        sige/src/rpc/proto.cpp
        sige/src/rpc/proto.h
        sige/src/rpc/rawtransaction.cpp
        sige/src/rpc/register.h
        sige/src/rpc/server.cpp
        sige/src/rpc/server.h 
        )

set (SRC_SCRIPT
        sige/src/script/interpreter.cpp
        sige/src/script/interpreter.h
        sige/src/script/ismine.cpp
        sige/src/script/ismine.h
        sige/src/script/script.cpp
        sige/src/script/script.h
        sige/src/script/script_const.cpp
        sige/src/script/script_const.h
        sige/src/script/script_error.cpp
        sige/src/script/script_error.h
        sige/src/script/script_id.cpp
        sige/src/script/script_id.h
        sige/src/script/script_num.cpp
        sige/src/script/script_num.h
        sige/src/script/sigcache.cpp
        sige/src/script/sigcache.h
        sige/src/script/sigcoinconsensus.cpp
        sige/src/script/sigcoinconsensus.h
        sige/src/script/sign.cpp
        sige/src/script/sign.h
        sige/src/script/standard.cpp
        sige/src/script/standard.h
        )

set (SRC_SUPPORT
        sige/src/support/cleanse.cpp
        sige/src/support/cleanse.h
        sige/src/support/events.h
        sige/src/support/lockedpool.cpp
        sige/src/support/lockedpool.h 
        )


source_group("src"        FILES ${SRC_CORE})
source_group("consensus"  FILES ${SRC_CONSENSUS})
source_group("policy"     FILES ${SRC_POLICY})
source_group("primitives" FILES ${SRC_PRIMITIVES})
source_group("rpc"        FILES ${SRC_RPC})
source_group("script"     FILES ${SRC_SCRIPT})
source_group("support"    FILES ${SRC_SUPPORT})



if (ENABLE_ZMQ)
       set (SRC_ZMQ
                sige/src/zmq/zmqabstractnotifier.cpp
                sige/src/zmq/zmqabstractnotifier.h
                sige/src/zmq/zmqconfig.h
                sige/src/zmq/zmqnotificationinterface.cpp
                sige/src/zmq/zmqnotificationinterface.h
                sige/src/zmq/zmqpublishnotifier.cpp
                sige/src/zmq/zmqpublishnotifier.h
        )
        source_group("zmq"        FILES ${SRC_ZMQ})
endif (ENABLE_ZMQ)

add_library (sigecoin 
        ${SRC_CORE}
        ${SRC_CONSENSUS}
        ${SRC_POLICY}
        ${SRC_PRIMITIVES}
        ${SRC_RPC}
        ${SRC_SCRIPT}
        ${SRC_SUPPORT}
        ${SRC_ZMQ}
        )

set_target_properties (sigecoin PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS}")
target_include_directories(sigecoin PUBLIC ${sige_SOURCE_DIR}/sige ${sige_SOURCE_DIR}/sige/src ${sige_SOURCE_DIR}/univalue ${sige_SOURCE_DIR}/blake2 ${secp256k1_SOURCE_DIR} ${leveldb_SOURCE_DIR} ${leveldb_SOURCE_DIR}/include ${libevent_SOURCE_DIR}/include ${libevent_BINARY_DIR}/include)
add_dependencies(sigecoin crypto wallet univalue blake2 secp256k1 leveldb event_core_static event_extra_static event_openssl_static)

add_executable (bench
        sige/bench/base58.cpp
        sige/bench/bench.cpp
        sige/bench/bench.h
        sige/bench/bench_sigcoin.cpp
        sige/bench/ccoins_caching.cpp
        sige/bench/checkblock.cpp
        sige/bench/checkqueue.cpp
        sige/bench/coin_selection.cpp
        sige/bench/crypto_hash.cpp
        sige/bench/examples.cpp
        sige/bench/lockedpool.cpp
        sige/bench/mempool_eviction.cpp
        sige/bench/perf.cpp
        sige/bench/perf.h
        sige/bench/rollingbloom.cpp
        sige/bench/verify_script.cpp 
        )

add_executable (sigenode
        sige/node/sigenode.cpp
	)

add_executable (sigenode-cli
        sige/node/sigenode-cli.cpp
	)

add_executable (sigenode-tx
        sige/node/sigenode-tx.cpp
	)


set_target_properties (blake2 PROPERTIES COMPILE_FLAGS "${PLATFORM_C_FLAGS} ${PLATFORM_COMPILE_FLAGS} -D__SSE2__")
set_target_properties (bench PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS} -DQT_NO_KEYWORDS -DSIGE_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR} -DSIGE_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR} -DBOOST_ASIO_HAS_STD_ARRAY=1 -DSIGE_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH}")
set_target_properties (bench PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")
set_target_properties (sigenode sigenode-cli sigenode-tx PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS} -DQT_NO_KEYWORDS -DSIGE_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR} -DSIGE_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR} -DBOOST_ASIO_HAS_STD_ARRAY=1 -DSIGE_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH}")
set_target_properties (sigenode sigenode-cli sigenode-tx PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")

target_link_libraries (sigenode sigecoin crypto wallet univalue  secp256k1 leveldb lmdb event_core_static event_extra_static event_openssl_static blake2 ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${PLATFORM_LIBS} ${PLATFORM_SPECIFIC_LIBS})
target_link_libraries (sigenode-cli sigecoin crypto wallet univalue secp256k1 event_core_static event_extra_static event_openssl_static blake2 ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${PLATFORM_LIBS} ${PLATFORM_SPECIFIC_LIBS})
target_link_libraries (sigenode-tx sigecoin crypto wallet univalue secp256k1 leveldb event_core_static event_extra_static event_openssl_static blake2 ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${PLATFORM_LIBS} ${PLATFORM_SPECIFIC_LIBS})
target_link_libraries (bench sigecoin crypto wallet secp256k1 leveldb event_core_static event_extra_static event_openssl_static blake2 ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${PLATFORM_LIBS} ${PLATFORM_SPECIFIC_LIBS})

if (SIGE_TEST)
        add_executable (tests
                test/addrman_tests.cpp
                test/allocator_tests.cpp
                test/amount_tests.cpp
                test/arith_uint256_tests.cpp
                test/base32_tests.cpp
                test/base58_tests.cpp
                test/base64_tests.cpp
                test/bip32_tests.cpp
                test/blockencodings_tests.cpp
                test/bloom_tests.cpp
                test/bswap_tests.cpp
                test/coins_tests.cpp
                test/compress_tests.cpp
                test/crypto_tests.cpp
                test/cuckoocache_tests.cpp
                test/dbwrapper_tests.cpp
                test/DoS_tests.cpp
                test/getarg_tests.cpp
                test/hash_tests.cpp
                test/key_tests.cpp
                test/limitedmap_tests.cpp
                test/main_tests.cpp
                test/mempool_tests.cpp
                test/merkle_tests.cpp
                test/miner_tests.cpp
                test/multisige_tests.cpp
                test/net_tests.cpp
                test/netbase_tests.cpp
                test/pmt_tests.cpp
                test/policyestimator_tests.cpp
                test/pow_tests.cpp
                test/prevector_tests.cpp
                test/reverselock_tests.cpp
                test/rpc_tests.cpp
                test/sanity_tests.cpp
                test/scheduler_tests.cpp
                test/script_P2SH_tests.cpp
                test/script_tests.cpp
                test/scriptnum_tests.cpp
                test/scriptnum10.h
                test/serialize_tests.cpp
                test/sighash_tests.cpp
                test/sigopcount_tests.cpp
                test/skiplist_tests.cpp
                test/streams_tests.cpp
                test/test_random.h
                test/test_sigecoin.cpp
                test/test_sigecoin.h
                test/test_sigecoin_fuzzy.cpp
                test/testutil.cpp
                test/testutil.h
                test/timedata_tests.cpp
                test/transaction_tests.cpp
                test/txvalidationcache_tests.cpp
                test/uint256_tests.cpp
                test/univalue_tests.cpp
                test/util_tests.cpp
                test/versionbits_tests.cpp 
                )

	set_target_properties (tests PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS} -DQT_NO_KEYWORDS -DSIGE_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR} -DSIGE_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR} -DBOOST_ASIO_HAS_STD_ARRAY=1 -DSIGE_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH}")
	set_target_properties (tests PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")
	target_link_libraries (tests sigecoin crypto univalue secp256k1 leveldb event_core_static event_extra_static event_openssl_static blake2 ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${PLATFORM_LIBS} ${PLATFORM_SPECIFIC_LIBS})

        add_executable (tests_crypto
                sige/crypto/ctaes/test.c
                )
	set_target_properties (tests_crypto PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS} -DQT_NO_KEYWORDS -DSIGE_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR} -DSIGE_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR} -DBOOST_ASIO_HAS_STD_ARRAY=1 -DSIGE_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH}")
	set_target_properties (tests_crypto PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")
	target_link_libraries (tests_crypto crypto ${PLATFORM_LIBS})

        add_executable (tests_wallet
                sige/wallet/test/accounting_tests.cpp
                sige/wallet/test/crypto_tests.cpp
                sige/wallet/test/wallet_test_fixture.cpp
                sige/wallet/test/wallet_test_fixture.h
                sige/wallet/test/wallet_tests.cpp
                )
	set_target_properties (tests_wallet PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS} -DQT_NO_KEYWORDS -DSIGE_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR} -DSIGE_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR} -DBOOST_ASIO_HAS_STD_ARRAY=1 -DSIGE_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH}")
	set_target_properties (tests_wallet PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")
	target_link_libraries (tests_wallet sigecoin crypto wallet univalue secp256k1 leveldb lmdb event_core_static event_extra_static event_openssl_static blake2 ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${PLATFORM_LIBS} ${PLATFORM_SPECIFIC_LIBS})
endif (SIGE_TEST)


if (SIGE_GUI)
        qt5_add_resources(RES src/qt/*.qrc)

	add_library (qt
                src/qt/*.cpp
		src/qt/*.h)

	add_executable (qt_wallet ${PLATFORM_GUI_TARGET_PROPERTIES}
		${PLATFORM_WALLET_SOURCE}
		src/wallet/*.cpp
        ${RES})

	set_target_properties (qt_wallet PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")
	
	set_target_properties (qt qt_wallet PROPERTIES COMPILE_FLAGS "${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS} -DQT_NO_KEYWORDS -DSIGE_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR} -DSIGE_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR} -DBOOST_ASIO_HAS_STD_ARRAY=1 -DSIGE_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH}")
	set_target_properties (qt PROPERTIES LINK_FLAGS "${PLATFORM_LINK_FLAGS}")
endif (SIGE_GUI)


if (WIN32)
	set (PLATFORM_LIBS Ws2_32 mswsock)
else (WIN32)
	set (PLATFORM_LIBS pthread dl)
endif (WIN32)

if (WIN32)
	set (PLATFORM_WALLET_LIBS Qt5::WinExtras)
else (WIN32)
	set (PLATFORM_WALLET_LIBS)
endif (WIN32)

if (SIGE_GUI)
	target_link_libraries (qt sigecoin qt_wallet lmdb blake2 ${Boost_ATOMIC_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_LOG_SETUP_LIBRARY} ${Boost_THREAD_LIBRARY} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Test ${QT_QTGUI_LIBRARY} ${PLATFORM_LIBS})
endif (SIGE_GUI)

set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
if (SIGE_GUI)
	if (APPLE)
		get_filename_component (Qt5Core_framework_DIR ${Qt5_DIR}/../../QtCore.framework ABSOLUTE)
		get_filename_component (Qt5Gui_framework_DIR ${Qt5_DIR}/../../QtGui.framework ABSOLUTE)
		get_filename_component (Qt5PrintSupport_framework_DIR ${Qt5_DIR}/../../QtPrintSupport.framework ABSOLUTE)
		get_filename_component (Qt5Test_framework_DIR ${Qt5_DIR}/../../QtTest.framework ABSOLUTE)
		get_filename_component (Qt5Widgets_framework_DIR ${Qt5_DIR}/../../QtWidgets.framework ABSOLUTE)
		get_filename_component (Qt5_platforms_DIR ${Qt5_DIR}/../../../plugins/platforms ABSOLUTE)

		file (COPY ${Qt5Core_framework_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/packaging)
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-id" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtCore.framework/Versions/5/QtCore")

		file (COPY ${Qt5Gui_framework_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/packaging)
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-id" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtGui.framework/Versions/5/QtGui")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Core_framework_DIR}/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtGui.framework/Versions/5/QtGui")

		file (COPY ${Qt5PrintSupport_framework_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/packaging)
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-id" "@executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtPrintSupport.framework/Versions/5/QtPrintSupport")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Core_framework_DIR}/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtPrintSupport.framework/Versions/5/QtPrintSupport")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Gui_framework_DIR}/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtPrintSupport.framework/Versions/5/QtPrintSupport")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Widgets_framework_DIR}/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtPrintSupport.framework/Versions/5/QtPrintSupport")

		file (COPY ${Qt5Test_framework_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/packaging)
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-id" "@executable_path/../Frameworks/QtTest.framework/Versions/5/QtTest" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtTest.framework/Versions/5/QtTest")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Core_framework_DIR}/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtTest.framework/Versions/5/QtTest")

		file (COPY ${Qt5Widgets_framework_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/packaging)
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-id" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtWidgets.framework/Versions/5/QtWidgets")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Core_framework_DIR}/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtWidgets.framework/Versions/5/QtWidgets")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Gui_framework_DIR}/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtWidgets.framework/Versions/5/QtWidgets")

		file (COPY ${Qt5_DIR}/../../../plugins/platforms/libqcocoa.dylib DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/packaging)
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Core_framework_DIR}/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" "${CMAKE_CURRENT_BINARY_DIR}/packaging/libqcocoa.dylib")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Gui_framework_DIR}/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" "${CMAKE_CURRENT_BINARY_DIR}/packaging/libqcocoa.dylib")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5PrintSupport_framework_DIR}/Versions/5/QtPrintSupport" "@executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport" "${CMAKE_CURRENT_BINARY_DIR}/packaging/libqcocoa.dylib")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Widgets_framework_DIR}/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets" "${CMAKE_CURRENT_BINARY_DIR}/packaging/libqcocoa.dylib")

		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:qt_wallet> ${CMAKE_CURRENT_BINARY_DIR}/packaging)
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Core_framework_DIR}/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" "${CMAKE_CURRENT_BINARY_DIR}/packaging/qt_wallet")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Gui_framework_DIR}/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui" "${CMAKE_CURRENT_BINARY_DIR}/packaging/qt_wallet")
		add_custom_command (TARGET qt_wallet POST_BUILD COMMAND "install_name_tool" "-change" "${Qt5Widgets_framework_DIR}/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets" "${CMAKE_CURRENT_BINARY_DIR}/packaging/qt_wallet")

		install (FILES ${CMAKE_CURRENT_BINARY_DIR}/packaging/qt_wallet DESTINATION qt_wallet.app/Contents/MacOS PERMISSIONS OWNER_EXECUTE OWNER_READ)
		install (FILES Info.plist DESTINATION qt_wallet.app/Contents)
		install (FILES qt.conf DESTINATION qt_wallet.app/Contents/Resources)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtCore.framework" DESTINATION qt_wallet.app/Contents/Frameworks USE_SOURCE_PERMISSIONS)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtGui.framework" DESTINATION qt_wallet.app/Contents/Frameworks USE_SOURCE_PERMISSIONS)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtPrintSupport.framework" DESTINATION qt_wallet.app/Contents/Frameworks USE_SOURCE_PERMISSIONS)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtTest.framework" DESTINATION qt_wallet.app/Contents/Frameworks USE_SOURCE_PERMISSIONS)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packaging/QtWidgets.framework" DESTINATION qt_wallet.app/Contents/Frameworks USE_SOURCE_PERMISSIONS)
		install (FILES "${CMAKE_CURRENT_BINARY_DIR}/packaging/libqcocoa.dylib" DESTINATION qt_wallet.app/Contents/PlugIns/platforms PERMISSIONS OWNER_EXECUTE OWNER_READ)
		install (FILES qt_wallet.icns DESTINATION qt_wallet.app/Contents/Resources)
	elseif (WIN32)
		if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
			set (WIN_REDIST vc_redist.x64.exe)
		else ()
			set (WIN_REDIST vc_redist.x86.exe)
		endif ()
		set (CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\${WIN_REDIST}\\\" /quiet /norestart'")
		set (CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/qt_wallet.ico)
		set (CPACK_NSIS_DISPLAY_NAME "sige")
		set (CPACK_NSIS_PACKAGE_NAME "sige ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
		set (CPACK_NSIS_URL_INFO_ABOUT "https://sigecoin.net")
		set (CPACK_NSIS_CONTACT "sigecoin@gmail.com")
		set (CPACK_NSIS_MENU_LINKS "qt_wallet.exe" "SGC Wallet" "https://sigecoin.net" "SigeCoin website")
		set (CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
		get_target_property (Qt5WindowsPlugin Qt5::QWindowsIntegrationPlugin LOCATION)
		get_filename_component (Qt5_bin_DIR ${Qt5_DIR}/../../../bin ABSOLUTE)
		install (TARGETS qt_wallet DESTINATION .)
		install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${WIN_REDIST} DESTINATION .)
		install (FILES ${Qt5_bin_DIR}/libGLESv2.dll DESTINATION .)
		install (FILES ${Qt5_bin_DIR}/Qt5Core.dll DESTINATION .)
		install (FILES ${Qt5_bin_DIR}/Qt5Gui.dll DESTINATION .)
		install (FILES ${Qt5_bin_DIR}/Qt5Widgets.dll DESTINATION .)
		install (FILES ${Qt5_bin_DIR}/Qt5WinExtras.dll DESTINATION .)
		install (FILES ${Qt5WindowsPlugin} DESTINATION platforms)
	else ()
		install (TARGETS qt_wallet DESTINATION .)
	endif ()
endif (SIGE_GUI)

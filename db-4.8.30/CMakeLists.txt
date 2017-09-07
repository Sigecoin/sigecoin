cmake_minimum_required    (VERSION 2.8.11)

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

project (libdb)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if (WIN32)
    configure_msvc_runtime()
    set (PLATFORM_COMPILE_FLAGS "-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_LIB /wd4244")
else (WIN32)
    set (PLATFORM_C_FLAGS "-std=c11")
    set (PLATFORM_CXX_FLAGS "-std=c++11")
    set (PLATFORM_COMPILE_FLAGS "-Werror=switch -fPIC")
endif (WIN32)


set (SRC_BTREE
    btree/bt_compact.c
    btree/bt_compare.c
    btree/bt_compress.c
    btree/bt_conv.c
    btree/bt_curadj.c
    btree/bt_cursor.c
    btree/bt_delete.c
    btree/bt_method.c
    btree/bt_open.c
    btree/bt_put.c
    btree/bt_rec.c
    btree/bt_reclaim.c
    btree/bt_recno.c
    btree/bt_rsearch.c
    btree/bt_search.c
    btree/bt_split.c
    btree/bt_stat.c
    btree/bt_upgrade.c
    btree/bt_verify.c
    btree/btree.src
    btree/btree_auto.c
    btree/btree_autop.c 
    )

set (SRC_CLIB
    clib/strsep.c
    )

set (SRC_COMMON
    common/db_byteorder.c
    common/db_compint.c
    common/db_err.c
    common/db_getlong.c
    common/db_idspace.c
    common/db_log2.c
    common/db_shash.c
    common/dbt.c
    common/mkpath.c
    common/openflags.c
    common/os_method.c
    common/util_cache.c
    common/util_log.c
    common/util_sig.c
    common/zerofill.c
    )

set (SRC_CRYPTO
    crypto/aes_method.c
    crypto/crypto.c
    crypto/mersenne/mt19937db.c
    crypto/rijndael/rijndael-alg-fst.c
    crypto/rijndael/rijndael-api-fst.c
    )

set (SRC_CXX
    cxx/cxx_db.cpp
    cxx/cxx_dbc.cpp
    cxx/cxx_dbt.cpp
    cxx/cxx_env.cpp
    cxx/cxx_except.cpp
    cxx/cxx_lock.cpp
    cxx/cxx_logc.cpp
    cxx/cxx_mpool.cpp
    cxx/cxx_multi.cpp
    cxx/cxx_seq.cpp
    cxx/cxx_txn.cpp
    )

set (SRC_DB
    db/crdel_auto.c
    db/crdel_autop.c
    db/crdel_rec.c
    db/db.c
    db/db_am.c
    db/db_auto.c
    db/db_autop.c
    db/db_cam.c
    db/db_cds.c
    db/db_conv.c
    db/db_dispatch.c
    db/db_dup.c
    db/db_iface.c
    db/db_join.c
    db/db_meta.c
    db/db_method.c
    db/db_open.c
    db/db_overflow.c
    db/db_ovfl_vrfy.c
    db/db_pr.c
    db/db_rec.c
    db/db_reclaim.c
    db/db_remove.c
    db/db_rename.c
    db/db_ret.c
    db/db_setid.c
    db/db_setlsn.c
    db/db_sort_multiple.c
    db/db_stati.c
    db/db_truncate.c
    db/db_upg.c
    db/db_upg_opd.c
    db/db_vrfy.c
    db/db_vrfyutil.c
    db/partition.c 
    )

set (SRC_DBM
    dbm/dbm.c
    )

set (SRC_DBREG
    dbreg/dbreg.c
    dbreg/dbreg_auto.c
    dbreg/dbreg_autop.c
    dbreg/dbreg_rec.c
    dbreg/dbreg_stat.c
    dbreg/dbreg_util.c 
    )

set (SRC_ENV
    env/env_alloc.c
    env/env_config.c
    env/env_failchk.c
    env/env_file.c
    env/env_globals.c
    env/env_method.c
    env/env_name.c
    env/env_open.c
    env/env_recover.c
    env/env_region.c
    env/env_register.c
    env/env_sig.c
    env/env_stat.c
    )

set (SRC_FILEOPS
    fileops/fileops_auto.c
    fileops/fileops_autop.c
    fileops/fop_basic.c
    fileops/fop_rec.c
    fileops/fop_util.c
    )

set (SRC_HASH
    hash/hash.c
    hash/hash_auto.c
    hash/hash_autop.c
    hash/hash_conv.c
    hash/hash_dup.c
    hash/hash_func.c
    hash/hash_meta.c
    hash/hash_method.c
    hash/hash_open.c
    hash/hash_page.c
    hash/hash_rec.c
    hash/hash_reclaim.c
    hash/hash_stat.c
    hash/hash_upgrade.c
    hash/hash_verify.c
    )

set (SRC_HMAC
    hmac/hmac.c
    hmac/sha1.c
    )

set (SRC_HSEARCH
    hsearch/hsearch.c
    )

set (SRC_LOCK
    lock/lock.c
    lock/lock_deadlock.c
    lock/lock_failchk.c
    lock/lock_id.c
    lock/lock_list.c
    lock/lock_method.c
    lock/lock_region.c
    lock/lock_stat.c
    lock/lock_timer.c
    lock/lock_util.c
    )

set (SRC_LOG
    log/log.c
    log/log_archive.c
    log/log_compare.c
    log/log_debug.c
    log/log_get.c
    log/log_method.c
    log/log_put.c
    log/log_stat.c
    )

set (SRC_MP
    mp/mp_alloc.c
    mp/mp_bh.c
    mp/mp_fget.c
    mp/mp_fmethod.c
    mp/mp_fopen.c
    mp/mp_fput.c
    mp/mp_fset.c
    mp/mp_method.c
    mp/mp_mvcc.c
    mp/mp_region.c
    mp/mp_register.c
    mp/mp_resize.c
    mp/mp_stat.c
    mp/mp_sync.c
    mp/mp_trickle.c
    )

if (WIN32)
    set (SRC_MUTEX
        mutex/mut_alloc.c
        mutex/mut_failchk.c
        mutex/mut_method.c
        mutex/mut_region.c
        mutex/mut_stat.c
        mutex/mut_win32.c
        )
else (WIN32)
    set (SRC_MUTEX
        mutex/mut_alloc.c
        mutex/mut_failchk.c
        mutex/mut_fcntl.c
        mutex/mut_method.c
        mutex/mut_pthread.c
        mutex/mut_region.c
        mutex/mut_stat.c
        mutex/mut_tas.c
        )
endif (WIN32)


if (WIN32)
    set (SRC_OS
        os/os_abort.c
        os/os_addrinfo.c
        os/os_alloc.c
        os/os_ctime.c
        os/os_pid.c
        os/os_root.c
        os/os_rpath.c
        os/os_stack.c
        os/os_tmpdir.c
        os/os_uid.c
        os_windows/os_abs.c
        os_windows/os_clock.c
        os_windows/os_config.c
        os_windows/os_cpu.c
        os_windows/os_dir.c
        os_windows/os_errno.c
        os_windows/os_fid.c
        os_windows/os_flock.c
        os_windows/os_fsync.c
        os_windows/os_getenv.c
        os_windows/os_handle.c
        os_windows/os_map.c
        os_windows/os_mkdir.c
        os_windows/os_open.c
        os_windows/os_rename.c
        os_windows/os_rw.c
        os_windows/os_seek.c
        os_windows/os_stat.c
        os_windows/os_truncate.c
        os_windows/os_unlink.c
        os_windows/os_yield.c
        build_windows/libdb.def
        build_windows/libdb.rc
        )
else (WIN32)
    set (SRC_OS
        os/os_abort.c
        os/os_abs.c
        os/os_addrinfo.c
        os/os_alloc.c
        os/os_clock.c
        os/os_config.c
        os/os_cpu.c
        os/os_ctime.c
        os/os_dir.c
        os/os_errno.c
        os/os_fid.c
        os/os_flock.c
        os/os_fsync.c
        os/os_getenv.c
        os/os_handle.c
        os/os_map.c
        os/os_mkdir.c
        os/os_open.c
        os/os_pid.c
        os/os_rename.c
        os/os_root.c
        os/os_rpath.c
        os/os_rw.c
        os/os_seek.c
        os/os_stack.c
        os/os_stat.c
        os/os_tmpdir.c
        os/os_truncate.c
        os/os_uid.c
        os/os_unlink.c
        os/os_yield.c
        )
endif (WIN32)


set (SRC_QAM
    qam/qam.c
    qam/qam_auto.c
    qam/qam_autop.c
    qam/qam_conv.c
    qam/qam_files.c
    qam/qam_method.c
    qam/qam_open.c
    qam/qam_rec.c
    qam/qam_stat.c
    qam/qam_upgrade.c
    qam/qam_verify.c
    )

set (SRC_REP
    rep/rep_auto.c
    rep/rep_backup.c
    rep/rep_elect.c
    rep/rep_lease.c
    rep/rep_log.c
    rep/rep_method.c
    rep/rep_record.c
    rep/rep_region.c
    rep/rep_stat.c
    rep/rep_util.c
    rep/rep_verify.c
    )

if (WIN32)
    set (SRC_REPMGR
        repmgr/repmgr_auto.c
        repmgr/repmgr_elect.c
        repmgr/repmgr_method.c
        repmgr/repmgr_msg.c
        repmgr/repmgr_net.c
        repmgr/repmgr_queue.c
        repmgr/repmgr_sel.c
        repmgr/repmgr_stat.c
        repmgr/repmgr_util.c
        repmgr/repmgr_windows.c
        )
else (WIN32)
    set (SRC_REPMGR
        repmgr/repmgr_auto.c
        repmgr/repmgr_elect.c
        repmgr/repmgr_method.c
        repmgr/repmgr_msg.c
        repmgr/repmgr_net.c
        repmgr/repmgr_queue.c
        repmgr/repmgr_sel.c
        repmgr/repmgr_stat.c
        repmgr/repmgr_util.c
        repmgr/repmgr_posix.c
        )
endif (WIN32)

set (SRC_SEQ
    sequence/seq_stat.c
    sequence/sequence.c
    )

set (SRC_TXN
    txn/txn.c
    txn/txn_auto.c
    txn/txn_chkpt.c
    txn/txn_failchk.c
    txn/txn_method.c
    txn/txn_rec.c
    txn/txn_recover.c
    txn/txn_region.c
    txn/txn_stat.c
    txn/txn_util.c
    )


source_group("btree" FILES ${SRC_BTREE})
source_group("clib" FILES ${SRC_CLIB})
source_group("common" FILES ${SRC_COMMON})
source_group("crypto" FILES ${SRC_CRYPTO})
source_group("cxx" FILES ${SRC_CXX})
source_group("db" FILES ${SRC_DB})
source_group("dbm" FILES ${SRC_DBM})
source_group("dbreg" FILES ${SRC_DBREG})
source_group("env" FILES ${SRC_ENV})
source_group("fileops" FILES ${SRC_FILEOPS})
source_group("hash" FILES ${SRC_HASH})
source_group("hmac" FILES ${SRC_HMAC})
source_group("hsearch" FILES ${SRC_HSEARCH})
source_group("lock" FILES ${SRC_LOCK})
source_group("log" FILES ${SRC_LOG})
source_group("mp" FILES ${SRC_MP})
source_group("mutex" FILES ${SRC_MUTEX})
source_group("os" FILES ${SRC_OS})
source_group("qam" FILES ${SRC_QAM})
source_group("rep" FILES ${SRC_REP})
source_group("repmgr" FILES ${SRC_REPMGR})
source_group("sequence" FILES ${SRC_SEQ})
source_group("txn" FILES ${SRC_TXN})

add_library (libdb
    ${SRC_BTREE}
    ${SRC_CLIB}
    ${SRC_COMMON}
    ${SRC_CRYPTO}
    ${SRC_CXX}
    ${SRC_DB}
    ${SRC_DBM}
    ${SRC_DBREG}
    ${SRC_ENV}
    ${SRC_FILEOPS}
    ${SRC_HASH}
    ${SRC_HMAC}
    ${SRC_HSEARCH}
    ${SRC_LOCK}
    ${SRC_LOG}
    ${SRC_MP}
    ${SRC_MUTEX}
    ${SRC_OS}
    ${SRC_QAM}
    ${SRC_REP}
    ${SRC_REPMGR}
    ${SRC_SEQ}
    ${SRC_TXN}
    )


set_target_properties(libdb PROPERTIES COMPILE_FLAGS "${PLATFORM_C_FLAGS} ${PLATFORM_CXX_FLAGS} ${PLATFORM_COMPILE_FLAGS}")
if (WIN32)
    target_include_directories(libdb PRIVATE ${libdb_SOURCE_DIR} ${libdb_SOURCE_DIR}/dbinc ${libdb_SOURCE_DIR}/build_windows)
else (WIN32)
    target_include_directories(libdb PRIVATE ${libdb_SOURCE_DIR} ${libdb_SOURCE_DIR}/dbinc ${libdb_SOURCE_DIR}/build_unix)
endif (WIN32)



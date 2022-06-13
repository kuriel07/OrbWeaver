SET(PROJECT_NAME orbweaver)

PROJECT(${PROJECT_NAME} C CXX ASM)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#set(CMAKE_VERBOSE_MAKEFILE ON)
enable_language(ASM)

#add_subdirectory(libglome)
#add_subdirectory(glome)
#add_subdirectory(torus)
#add_library(libglome)
#add_executable(torus)

set(ORBWEAVER_TARGET_VERSION "1.0.0" CACHE STRING "OrbWeaver logical version")
set(ORBWEAVER_TARGET_SOVERSION "1.0.0" CACHE STRING "OrbWeaver soname version")

set(BUILD_TYPE Release)
#set(BUILD_TYPE RelWithDebInfo)

set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)
#set(CMAKE_MODULE_PATH /usr/local/lib/cmake)

# optional libraries
find_package(libcgi QUIET)

# flags
message("Processor Type : ${CMAKE_HOST_SYSTEM_PROCESSOR}")
#message("Is UNIX : ${CMAKE_HOST_UNIX}")
#message("Is WIN32 : ${CMAKE_HOST_WIN32}")
#message("Is APPLE : ${CMAKE_HOST_APPLE}")
message("OSX Architectures : ${CMAKE_OSX_ARCHITECTURES}")
message("OSX Deployment Target : ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message("System Processor : ${CMAKE_SYSTEM_PROCESSOR}")
message("Lib Architecture : ${CMAKE_LIBRARY_ARCHITECTURE}")
message("Target Compiler : ${CMAKE_CXX_COMPILER_TARGET}")
message("ABI : ${CMAKE_CXX_COMPILER_ABI}")
message("C Compiler : ${CMAKE_C_COMPILER}")
message("C++ Compiler : ${CMAKE_CXX_COMPILER}")
message("Assembler : ${CMAKE_ASM_COMPILER}")
message("Lib Architecture : ${CMAKE_CXX_LIBRARY_ARCHITECTURE}")


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    # OSX
    SET(OperatingSystem "MacOS")
    add_definitions("-DDARWIN")
    find_package(OpenSSL REQUIRED) 
    find_package(PkgConfig REQUIRED)
    #pkg_search_module(GLIB REQUIRED glib-2.0)
    pkg_check_modules(GLIB REQUIRED glib-2.0 gobject-2.0 gthread-2.0 gmodule-2.0)
    find_package(PostgreSQL QUIET)
    pkg_search_module(jansson REQUIRED jansson>=1.0)
    #pkg_search_module(sqlite3 REQUIRED sqlite3>=3.0)
    find_package(SQLite3 REQUIRED) 
    add_definitions(${GLIB_CFLAGS_OTHER})
    #pkg_search_module(openssl REQUIRED openssl>=1.0)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Linux specific code
    SET(OperatingSystem "Linux")
    add_definitions("-DUNIX")
    find_package(OpenSSL REQUIRED) 
    find_package(PkgConfig REQUIRED)
    #pkg_search_module(GLIB REQUIRED glib-2.0)
    pkg_check_modules(GLIB REQUIRED glib-2.0 gobject-2.0 gthread-2.0 gmodule-2.0)
    find_package(PostgreSQL QUIET)
    pkg_search_module(jansson REQUIRED jansson>=1.0)
    #pkg_search_module(sqlite3 REQUIRED sqlite3>=3.0)
    find_package(SQLite3 REQUIRED) 
    add_definitions(${GLIB_CFLAGS_OTHER})
    #pkg_search_module(openssl REQUIRED openssl>=1.0)
endif() 

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Windows specific code
    SET(OperatingSystem "Windows")
    add_definitions("-DWINDOWS")
endif()

add_definitions("-DENABLE_SSE")
message("OpenSSL : ${OPENSSL_FOUND}")
if(${OPENSSL_FOUND} MATCHES "TRUE")
	message("OpenSSL Enabled")
	message("OpenSSL Include : ${OPENSSL_INCLUDE_DIR}")
	message("OpenSSL Libs : ${OPENSSL_LIBRARIES}")
	add_definitions("-DUSE_OPENSSL")
endif()

message("SQLite3 : ${SQLite3_FOUND}")
if(${SQLite3_FOUND} MATCHES "TRUE")
	message("SQLite3 Enabled")
	add_definitions("-DHAVE_SQLITE")
endif()

message("PostgreSQL : ${PostgreSQL_FOUND}")
if(${PostgreSQL_FOUND} MATCHES "TRUE")
	message("PostgreSQL Enabled")
	add_definitions("-DHAVE_LIBPQ")
endif()


set(ASM_OPTIONS "-x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}")

set(CMAKE_CXX_FLAGS
   "${SSE_FLAGS} -O0 -g -std=c++11 -rdynamic -march=native"
#   "${SSE_FLAGS} -O3 -g -std=c++0x -fno-omit-frame-pointer"
)
set(CMAKE_C_FLAGS
   "${SSE_FLAGS} -O0 -g -std=c11 -rdynamic -march=native"
#   "${SSE_FLAGS} -O3 -g -std=c++0x -fno-omit-frame-pointer"
)

if (MSVC)
     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif (MSVC)

#global include directories
include_directories(
  ${PROJECT_SOURCE_DIR}/StackLib
  ${PROJECT_SOURCE_DIR}/StackLib/Stack
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/inc
  ${SQLite3_INCLUDE_DIRS}
) 

# Sources files
set(glome_source_files
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/asm_streamer.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/il_streamer.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/il1_optimizer.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/il2_optimizer.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/lex.yy.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/lex_proto.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/pkg_encoder.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/pkg_encoder_apis.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/pkg_linker.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/scr_generator.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/sem_proto.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/sym_table.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/midgard.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/midgard_apis.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_framework.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_stack.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_stream.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_aes.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_core.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_crc.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_crypt.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_des.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_mac.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_md5.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_sha1.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_sha256.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_stack.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_stack_apis.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_framework.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_stream.c
)

# Sources files
set(orbweaver_source_files
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/pkg_encoder_apis.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/midgard_apis.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_stack_apis.c
)

set(torus_source_files
  ${PROJECT_SOURCE_DIR}/StackHost/stack/stack_ext_framework.cpp
  ${PROJECT_SOURCE_DIR}/StackHost/stack/stack_port.c
  ${PROJECT_SOURCE_DIR}/StackHost/stack/stack_ext_network.c
  ${PROJECT_SOURCE_DIR}/StackHost/core/http.c
  ${PROJECT_SOURCE_DIR}/StackHost/core/https.c
  ${PROJECT_SOURCE_DIR}/StackHost/core/net.cpp
  ${PROJECT_SOURCE_DIR}/StackHost/core/global.c
  ${PROJECT_SOURCE_DIR}/StackHost/core/http_parser.c
  ${PROJECT_SOURCE_DIR}/StackHost/dal/dal_core.c
  ${PROJECT_SOURCE_DIR}/StackHost/dal/dal_pgsql.c
  ${PROJECT_SOURCE_DIR}/StackHost/util/util.cpp
  ${PROJECT_SOURCE_DIR}/StackHost/util/minIni.c
  ${PROJECT_SOURCE_DIR}/StackHost/StackHost.cpp
)

# compile main library.
include_directories( ${CSPARSE_INCLUDE_DIR} ${CHOLMOD_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${PostgreSQL_INCLUDE_DIR}) 

if(${CMAKE_HOST_UNIX})
  message("UNIX System setup")
    set_source_files_properties(${PROJECT_SOURCE_DIR}/StackLib/StackVM/stack_abis_sysv_x64_interface.s 
        PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp"
    )
  set(INSTALL_CMAKE_DIR lib/cmake/stack)
  set(glome_source_files 
	${glome_source_files}
  	${PROJECT_SOURCE_DIR}/StackLib/Stack/stack.tab.c
        ${PROJECT_SOURCE_DIR}/StackLib/StackVM/stack_abis_sysv_x64_interface.s
        ${PROJECT_SOURCE_DIR}/StackLib/StackVM/stack_abis_sysv_x64.c
  )
  set(ASM_SYS_FLAGS "-64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed,-z,now -ldl ")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--no-as-needed,-z,now -ldl ")
  set(CMAKE_SHARED_LINKER_FLAGS "-shared -Wl,--as-needed,-z,now -ldl ")
endif()
if(${CMAKE_HOST_WIN32})
  message("WIN32 System setup")
  set(INSTALL_CMAKE_DIR lib/cmake/stack)
  set(glome_source_files 
	${glome_source_files}
  	${PROJECT_SOURCE_DIR}/StackLib/Stack/stack_tab.c
        ${PROJECT_SOURCE_DIR}/StackLib/StackVM/dlfcn.c
        ${PROJECT_SOURCE_DIR}/StackLib/StackVM/stack_abis_win64_x64.cpp
        ${PROJECT_SOURCE_DIR}/StackLib/StackVM/stack_abis_win64_x64_interface.asm
        ${PROJECT_SOURCE_DIR}/StackHost/Jansson/src/*.c
  )
endif()
if(${CMAKE_HOST_APPLE})
  message("APPLE System setup")
  set(INSTALL_CMAKE_DIR lib/cmake/stack)
  set(glome_source_files 
	${glome_source_files}
  	${PROJECT_SOURCE_DIR}/StackLib/Stack/stack.tab.c
        ${PROJECT_SOURCE_DIR}/StackLib/StackVM/stack_abis_sysv_x64_interface.s
        ${PROJECT_SOURCE_DIR}/StackLib/StackVM/stack_abis_sysv_x64.c
  )
  set(ASM_SYS_FLAGS "-arch x86_64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -ldl")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--no-as-needed -ldl")
  set(CMAKE_SHARED_LINKER_FLAGS "-shared -Wl,--as-needed -ldl")
endif()


message("--- building stack library.")
add_library(stack SHARED ${glome_source_files} )
target_compile_definitions(stack PRIVATE "-DSTACK_HOST")
target_include_directories(stack PRIVATE
	${PROJECT_SOURCE_DIR}/StackHost
	${PROJECT_SOURCE_DIR}/StackHost/core
	${PROJECT_SOURCE_DIR}/StackHost/dal
	${PROJECT_SOURCE_DIR}/StackHost/util
)
target_include_directories(stack PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
	$<INSTALL_INTERFACE:${INSTALL_CMAKE_DIR}/public>)
#set_target_properties(orbweaver PROPERTIES PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/include/orbweaver)
install(FILES ${PROJECT_SOURCE_DIR}/include/orbweaver.h DESTINATION include)
install(TARGETS stack 
	EXPORT stack-targets DESTINATION lib
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
install(EXPORT stack-targets DESTINATION lib/cmake/stack)
install(FILES StackConfig.cmake DESTINATION lib/cmake/stack)

#set_property( TARGET glome APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )

message("--- building glome compiler.")
#add_executable(glome_int ${glome_source_files} )
add_executable(glome ${glome_source_files} ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_interpreter.c)
target_compile_definitions(glome PRIVATE "-DSTANDALONE_INTERPRETER")
target_link_libraries(glome ${libcgi_LIBRARIES})


message("--- building orbweaver library.")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    # OSX 
    set_source_files_properties(${PROJECT_SOURCE_DIR}/StackLib/StackVM/stack_abis_sysv_x64_interface.s 
        PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp"
    )
    add_library(orbweaver SHARED
        ${orbweaver_source_files} 
    )
    target_compile_definitions(orbweaver PRIVATE "-DSTACK_HOST")
    target_include_directories(orbweaver PRIVATE
  	${PROJECT_SOURCE_DIR}/StackHost
  	${PROJECT_SOURCE_DIR}/StackHost/core
  	${PROJECT_SOURCE_DIR}/StackHost/dal
  	${PROJECT_SOURCE_DIR}/StackHost/util
	${OPENSSL_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${PostgreSQL_INCLUDE_DIRS}
    	${SQLite3_INCLUDE_DIRS}
    )
    #target_link_libraries(orbweaver jansson ${OPENSSL_LIBRARIES})
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Linux specific code
    add_library(orbweaver SHARED 
        ${orbweaver_source_files} 
    )
    target_compile_definitions(orbweaver PRIVATE "-DSTACK_HOST")
    target_include_directories(orbweaver PRIVATE
  	${PROJECT_SOURCE_DIR}/StackHost
  	${PROJECT_SOURCE_DIR}/StackHost/core
  	${PROJECT_SOURCE_DIR}/StackHost/dal
  	${PROJECT_SOURCE_DIR}/StackHost/util
	${OPENSSL_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${PostgreSQL_INCLUDE_DIRS}
    	${SQLite3_INCLUDE_DIRS}
    )
    #target_link_libraries(orbweaver jansson ${OPENSSL_LIBRARIES})
endif() 

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Windows specific code
    add_library(orbweaver SHARED
        ${glome_source_files} 
        ${torus_source_files} 
    )
    target_compile_definitions(orbweaver PRIVATE "-DSTACK_HOST")
    target_include_directories(orbweaver PRIVATE
  	${PROJECT_SOURCE_DIR}/POSIX
  	${PROJECT_SOURCE_DIR}/StackHost
  	${PROJECT_SOURCE_DIR}/StackHost/core
  	${PROJECT_SOURCE_DIR}/StackHost/dal
  	${PROJECT_SOURCE_DIR}/StackHost/util
    )
    #target_link_libraries(orbweaver 
    #    ${PROJECT_SOURCE_DIR}/StackHost/openssl/lib/libssl.lib 
    #    ${PROJECT_SOURCE_DIR}/StackHost/openssl/lib/libcrypto.lib
    #)
endif()
target_include_directories(orbweaver PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
	$<INSTALL_INTERFACE:${INSTALL_CMAKE_DIR}/public>)
#set_target_properties(orbweaver PROPERTIES PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/include/orbweaver)
#install(FILES ${PROJECT_SOURCE_DIR}/include/orbweaver.h DESTINATION include)
install(TARGETS orbweaver 
	EXPORT orbweaver-targets DESTINATION lib
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
install(EXPORT orbweaver-targets DESTINATION lib/cmake/orbweaver)
install(FILES OrbWeaverConfig.cmake DESTINATION lib/cmake/orbweaver)

message("--- building torus.")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
#set_source_files_properties(${glome_source_files} PROPERTIES COMPILE_FLAGS "-pthread")
#set_source_files_properties(${torus_source_files} PROPERTIES COMPILE_FLAGS "-pthread")
add_executable(torus 
        ${glome_source_files} 
        ${torus_source_files}
	${PROJECT_SOURCE_DIR}/StackHost/main.cpp)

#set_target_properties(torus PROPERTIES COMPILE_FLAGS "-pthread")
#target_compile_options(torus PRIVATE "-pthread")
set_target_properties(torus PROPERTIES COMPILE_FLAGS "-pthread")
target_include_directories(torus PRIVATE
    ${PROJECT_SOURCE_DIR}/StackHost
    ${PROJECT_SOURCE_DIR}/StackHost/core
    ${PROJECT_SOURCE_DIR}/StackHost/dal
    ${PROJECT_SOURCE_DIR}/StackHost/util
    ${OPENSSL_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${PostgreSQL_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)
#target_compile_options(torus PRIVATE "")
target_compile_definitions(torus PRIVATE "-DSTACK_HOST -DVM_EXT_APIS")
set_target_properties(torus PROPERTIES LINK_FLAGS "-lpthread")
target_link_libraries(torus jansson ${OPENSSL_LIBRARIES} ${PostgreSQL_LIBRARIES} ${SQLite3_LIBRARIES})
#target_link_libraries(torus orbweaver)

#install(TARGETS torus DESTINATION bin RUNTIME DESTINATION bin)



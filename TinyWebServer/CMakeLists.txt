file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_executable(TinyWebServer ${ALL_CLANG_FORMAT_SOURCE_FILES})

set(DEP_LIBS
    OpenSSL::SSL  # modern
    #${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}  # old style
)

find_package(OpenSSL 1.1 REQUIRED)

if(WIN32)
    target_link_libraries(TinyWebServer wsock32 ws2_32 ${DEP_LIBS})
else()
    find_package(Threads)
    target_link_libraries(TinyWebServer #[[profiler]] ${CMAKE_THREAD_LIBS_INIT} ${DEP_LIBS})
endif()

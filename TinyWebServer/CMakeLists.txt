file(GLOB SOURCES "src/*")

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_executable(TinyWebServer ${SOURCES})

set(DEP_LIBS
    OpenSSL::SSL  # modern
    #${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}  # old style
)

find_package(OpenSSL 1.1 REQUIRED)

if(WIN32)
    target_link_libraries(TinyWebServer wsock32 ws2_32 ${DEP_LIBS})
else()
    find_package(Threads)
    target_link_libraries(TinyWebServer ${CMAKE_THREAD_LIBS_INIT} ${DEP_LIBS})
endif()

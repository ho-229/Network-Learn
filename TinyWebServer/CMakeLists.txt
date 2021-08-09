set(SOURCES
   src/main.cpp

   src/webserver.h
   src/webserver.cpp

   src/httpservices.h
   src/httpservices.cpp

   src/httprequest.h
   src/httprequest.cpp

   src/httpresponse.h
   src/httpresponse.cpp

   src/abstractsocket.h

   src/tcpsocket.h
   src/tcpsocket.cpp

   src/sslsocket.h
   src/sslsocket.cpp

   src/event.h
   src/event.cpp

   src/until.h
)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_executable(TinyWebServer ${SOURCES})

set(DEP_LIBS
    OpenSSL::Crypto OpenSSL::SSL  # modern
    #${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}  # old style
)

find_package(OpenSSL REQUIRED)

if(WIN32)
    target_link_libraries(TinyWebServer wsock32 ws2_32 ${DEP_LIBS})
else()
    find_package(Threads)
    target_link_libraries(TinyWebServer ${CMAKE_THREAD_LIBS_INIT} ${DEP_LIBS})
endif()

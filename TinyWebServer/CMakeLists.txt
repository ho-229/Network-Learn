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

   src/event.h
   src/event.cpp

   src/until.h
)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_executable(TinyWebServer ${SOURCES})

if(WIN32)
    target_link_libraries(TinyWebServer wsock32 ws2_32)
else()
    find_package(Threads)
    target_link_libraries(TinyWebServer ${CMAKE_THREAD_LIBS_INIT})
endif()

cmake_minimum_required(VERSION 2.6)
project(qbiff)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories( ${KDE4_INCLUDES} )
 
set(qbiff_SRCS button.cpp clientfolder.cpp clientinfo.cpp clientinit.cpp notify.cpp parser.cpp qbiff.cpp serverhandler.cpp sslclient.cpp sslcommon.cpp sslconnection.cpp sslinit.cpp sslserver.cpp)
 
kde4_add_executable(qbiff ${qbiff_SRCS})
target_link_libraries(qbiff ${KDE4_KDEUI_LIBS} ssl crypto Xtst)

install(TARGETS qbiff DESTINATION ${BIN_INSTALL_DIR})

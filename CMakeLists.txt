project(qbiff)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.2.0")

cmake_policy(SET CMP0063 NEW)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

# Find Qt modules
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS 
    Core    # QCommandLineParser, QStringLiteral
    Widgets # QApplication 
)

# Find KDE modules
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons      # KAboutData
    I18n            # KLocalizedString
    WidgetsAddons   # KMessageBox
    Wallet          # KWallet
)

feature_summary(
    WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES
)

set(qbiff_SRCS button.cpp clientfolder.cpp clientinfo.cpp clientinit.cpp notify.cpp parser.cpp qbiff.cpp serverhandler.cpp sslclient.cpp sslcommon.cpp sslconnection.cpp sslinit.cpp sslserver.cpp)
 
add_executable(qbiff ${qbiff_SRCS})

target_compile_options(qbiff PRIVATE -Wno-vla)

target_link_libraries(
    qbiff
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    KF5::Wallet
    ssl
    crypto
    Xtst
)

install(TARGETS qbiff  ${INSTALL_TARGETS_DEFAULT_ARGS})

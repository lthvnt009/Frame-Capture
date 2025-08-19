# CMakeLists.txt - Version 1.7
# --- Cài đặt CMake tối thiểu và thông tin dự án ---
cmake_minimum_required(VERSION 3.16)
project(FrameCapture VERSION 1.0 LANGUAGES CXX)

# --- Cài đặt chuẩn C++ ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Cấu hình cho Qt6 ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Multimedia)

# --- Cấu hình cho FFmpeg ---
set(FFMPEG_DIR "D:/VSCode/Frame Capture/ffmpeg")
include_directories(${FFMPEG_DIR}/include)

find_library(AVCODEC_LIBRARY NAMES avcodec HINTS ${FFMPEG_DIR}/lib)
find_library(AVFORMAT_LIBRARY NAMES avformat HINTS ${FFMPEG_DIR}/lib)
find_library(AVUTIL_LIBRARY NAMES avutil HINTS ${FFMPEG_DIR}/lib)
find_library(SWSCALE_LIBRARY NAMES swscale HINTS ${FFMPEG_DIR}/lib)

# --- Tạo file thực thi ---
add_executable(FrameCapture
    main.cpp
    mainwindow.cpp
    videoprocessor.cpp
    videowidget.cpp
    viewpanel.cpp
    libraryitemdelegate.cpp
    cropdialog.cpp
    librarywidget.cpp
    imageviewerdialog.cpp # Thêm file mới
)

# --- Liên kết các thư viện cần thiết ---
target_link_libraries(FrameCapture PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Core
    Qt6::Multimedia
    ${AVCODEC_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWSCALE_LIBRARY}
)

# --- Thêm các file header để VSCode/Qt Creator nhận diện ---
target_sources(FrameCapture PRIVATE
    mainwindow.h
    videoprocessor.h
    videowidget.h
    viewpanel.h
    libraryitemdelegate.h
    cropdialog.h
    librarywidget.h
    imageviewerdialog.h # Thêm file mới
)

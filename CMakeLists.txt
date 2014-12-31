cmake_minimum_required(VERSION 2.8.4)

project(video_encoder)

#set(SOURCE_FILES1 x264_encoder.c)
#set(SOURCE_FILES2 x265_encoder.c)
set(SOURCE_FILES3 vpx_encoder.c)

include_directories(${PROJECT_SOURCE_DIR}/deps "/usr/local/include")

link_directories("/usr/local/lib")

#add_executable(x264_encoder ${SOURCE_FILES1})
#add_executable(x265_encoder ${SOURCE_FILES2})
add_executable(vpx_encoder ${SOURCE_FILES3})

#target_link_libraries(x264_encoder "libx264.a")
#target_link_libraries(x265_encoder "libx265.a")
target_link_libraries(vpx_encoder "libvpx.a")

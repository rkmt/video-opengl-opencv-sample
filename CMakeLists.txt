# CMake entry point
cmake_minimum_required (VERSION 2.6)
project (Tutorials)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package(OpenGL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

link_directories(${GLEW}/lib ${GLFW}/lib)
#include_directories(${GLEW}/include ${GLFW}/include)
include_directories(/usr/local/Cellar/glfw3/3.1.2/include/GLFW/)
include_directories(/usr/include/GLFW/)

link_directories(/usr/local/lib)

# video
add_executable(video video.cpp fps.cpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
	simple.frag
	simple.vert
)
target_link_libraries(video ${OpenCV_LIBS} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ALL_LIBS})


# video1
add_executable(video1 video1.cpp fps.cpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
	simple.frag
	simple.vert
)
target_link_libraries(video1 ${OpenCV_LIBS} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ALL_LIBS})

# video2
add_executable(video2 video2.cpp fps.cpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
	simple.frag
	simple.vert
)
target_link_libraries(video2 ${OpenCV_LIBS} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ALL_LIBS})

# video3
add_executable(video3 video3.cpp fps.cpp
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
	simple.frag
	simple.vert
)
target_link_libraries(video3 ${OpenCV_LIBS} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ALL_LIBS})


# video4
add_executable(video4 video4.cpp VideoTexture.cpp fps.cpp 
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
	morph.frag
	simple.frag
	simple.vert
)
target_link_libraries(video4 ${OpenCV_LIBS} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ALL_LIBS})

# video5
add_executable(video5 video5.cpp VideoTexture.cpp fps.cpp 
	common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
	morph.frag
	simple.frag
	simple.vert
)
target_link_libraries(video5 ${OpenCV_LIBS} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${ALL_LIBS})




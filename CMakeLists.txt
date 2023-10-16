cmake_minimum_required(VERSION 3.10)
project(Renderer)

set(CMAKE_CXX_STANDARD 11)

add_executable(Renderer
#include
 include/Bound3.h
 include/BVH.h
 include/Global.h
 include/Intersection.h
 include/Material.h
 include/Matrix.h
 include/Object.h
 include/Ray.h
 include/Render.h
 include/Scene.h
 include/Sphere.h
 include/Trangle.h
 include/Vector.h

#source
 src/BVH.cpp
 src/main.cpp
 src/Material.cpp
 src/Render.cpp
 src/Scene.cpp
 src/Sphere.cpp
 src/Trangle.cpp
)
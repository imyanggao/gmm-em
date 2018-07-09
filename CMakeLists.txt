cmake_minimum_required(VERSION 2.8)
 
project(gmmEM)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
#add_executable(GMM MACOSX_BUNDLE gaussian.cxx)
add_executable(gmmEM Gaussian.cxx main.cxx GaussianMixtureModel.cxx EMGaussianMixtures.cxx KMeansPlusPlus.cxx)

target_link_libraries(gmmEM ${ITK_LIBRARIES})
install(TARGETS gmmEM RUNTIME DESTINATION bin)
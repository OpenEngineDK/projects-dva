# Project name. Should be the same as the project directory name and
# will become the name of the binary target.
SET( PROJECT_NAME "dva")

# Project source code list
SET( PROJECT_SOURCES
  # Add all the cpp source files here
  main.cpp
  DVASetup.cpp
  InputController.cpp
  ScreenplayController.cpp
  RelayBox.cpp
  Scene/GridNode.cpp
  Utils/Stages.cpp
  Devices/LaserSensor.cpp
  Devices/SICKDeviceDriver.cpp
  Devices/ClusterAnalyser.cpp
)

# Include needed to use SDL under Mac OS X
IF(APPLE)
  SET(PROJECT_SOURCES ${PROJECT_SOURCES}  ${SDL_MAIN_FOR_MAC})
ENDIF(APPLE)

# Project executable
ADD_EXECUTABLE(${PROJECT_NAME}
  ${PROJECT_SOURCES}
)

# Project dependencies
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  # Core library dependencies
  OpenEngine_Core
  OpenEngine_Logging
  OpenEngine_Display
  OpenEngine_Devices
  OpenEngine_Scene

  # Extension dependencies
  Extensions_AssimpResource
  Extensions_GenericHandlers
  Extensions_OpenGLPostProcessEffects
  Extensions_SDL
  Extensions_SetupHelpers
  Extensions_AnimationFramework
  Extensions_Boids
  Extensions_Sockets
  Extensions_VorbisResource
  Extensions_OpenALSoundSystem
  Extensions_MusicPlayer
  Extensions_PropertyBar
)

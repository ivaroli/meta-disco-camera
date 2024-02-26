# add hardcoded guesses for the location of Vmb to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/../.. ENV{VMB_HOME}/api $ENV{VIMBA_X_HOME}/api)

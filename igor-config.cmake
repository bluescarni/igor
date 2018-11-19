# Get current dir.
get_filename_component(_IGOR_CONFIG_SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

include(${_IGOR_CONFIG_SELF_DIR}/igor_export.cmake)

# Clean up.
unset(_IGOR_CONFIG_SELF_DIR)

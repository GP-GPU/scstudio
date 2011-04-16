# hhc.exe always returns nonzero exit codes
# indicect hhc.exe execution is to avoid build error in visual studio
EXECUTE_PROCESS(
  COMMAND ${HTML_HELP_COMPILER} scstudio.hhp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

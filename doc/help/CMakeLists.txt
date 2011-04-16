FIND_PACKAGE(HTMLHelp)

IF(HTML_HELP_COMPILER)
  ADD_CUSTOM_TARGET(help ALL
    DEPENDS scstudio.chm
  )
  ADD_CUSTOM_COMMAND(
    OUTPUT scstudio.chm
    COMMAND ${CMAKE_COMMAND} -DHTML_HELP_COMPILER=${HTML_HELP_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR}/hhc.cmake
    DEPENDS
      help.css
      algorithms.html
      frontend.html
      localChoice/localchoice.html
      membership/membership.html
      fifo/fifo.html
      deadlock/deadlock.html
      livelock/livelock.html
      race/race.html
      realizability/realizability.html
      boundedness/boundedness.html
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
ENDIF(HTML_HELP_COMPILER)

# $Id: CMakeLists.txt 789 2010-05-16 20:44:30Z vacek $

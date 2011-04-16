ADD_LIBRARY(scmembership SHARED
  export.h
  module.cpp
  membership_alg.cpp
  membership_alg.h
  membership_time.cpp
  membership_time.h
  membership_additional.cpp
  membership_additional.h
  membership_base.h
  diff_impl.h
  diff_impl.cpp
)

TARGET_LINK_LIBRARIES(scmembership
  scmsc
  scpseudocode
  sctime
)

INSTALL(TARGETS scmembership
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 1030 2011-02-02 23:57:58Z madzin $

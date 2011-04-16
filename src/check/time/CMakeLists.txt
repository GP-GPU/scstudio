ADD_LIBRARY(sctime SHARED
  export.h
  module.cpp
  time_pseudocode.cpp
  time_pseudocode.h
  time_consistency.cpp
  time_consistency.h
  traverse_erase.cpp
  traverse_erase.h
  tightening.cpp
  tightening.h
  constraint_syntax.cpp
  constraint_syntax.h
  time_trace_race.h
  hmsc_all_paths.h
  hmsc_all_paths.cpp
  find_block.cpp
  find_block.h
  hmsc_block_paths.cpp  
  hmsc_block_paths.h

  
)

TARGET_LINK_LIBRARIES(sctime
  scmsc
  scpseudocode
)


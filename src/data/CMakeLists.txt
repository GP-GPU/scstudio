ADD_LIBRARY(scmsc SHARED
  export.h
  msc_types.cpp
  msc_types.h
  fcmp.cpp
  fcmp.h
  time.cpp
  time.h
  strip_diacritics.cpp
  msc.cpp
  msc.h
  configurator.h
  reporter.h
  prerequisite_check.h
  formatter.h
  checker.h
  searcher.h
  simulator.h
  transformer.h
  session_attribute.cpp
  session_attribute.h
  dfs_area_traverser.cpp
  dfs_area_traverser.h
  dfs_events_traverser.cpp
  dfs_events_traverser.h
  dfs_hmsc_traverser.cpp
  dfs_hmsc_traverser.h
  node_finder.cpp
  node_finder.h
  dfs_refnode_hmsc_traverser.cpp
  dfs_refnode_hmsc_traverser.h
  dfsb_hmsc_traverser.cpp
  dfsb_hmsc_traverser.h
  dfs_instance_events_traverser.cpp
  dfs_instance_events_traverser.h
  dfs_bmsc_graph_traverser.cpp
  dfs_bmsc_graph_traverser.h
  dfs_hmsc_flat_traverser.cpp
  dfs_hmsc_flat_traverser.h
  hmsc_reference_checker.h
  elementary_cycles_traverser.h
  elementary_cycles_traverser.cpp
)

# build import-export formatters
ADD_SUBDIRECTORY(engmann)
ADD_SUBDIRECTORY(Z120)
ADD_SUBDIRECTORY(modelchecking)
# build transformers
ADD_SUBDIRECTORY(beautify)
ADD_SUBDIRECTORY(pysc)

OPTION(BUILD_PCAP "Enable to build the PCAP import module" OFF)
IF(BUILD_PCAP)
  ADD_SUBDIRECTORY(pcap)
ENDIF(BUILD_PCAP)

INSTALL(FILES
  msc.h
  msc_types.h
  time.h
  export.h
  DESTINATION include/scstudio/data)

INSTALL(TARGETS scmsc
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 707 2010-03-29 16:24:16Z vacek $

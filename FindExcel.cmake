FIND_FILE(EXCEL_MSO
  NAMES mso.dll
  PATHS
    "C:/Program Files/Common Files/Microsoft Shared/Office12"
    "C:/Program Files (x86)/Common Files/Microsoft Shared/Office12"
    "C:/Program Files/Common Files/Microsoft Shared/Office11"
    "C:/Program Files (x86)/Common Files/Microsoft Shared/Office11"
  DOC "Microsoft Shared Objects, path to mso.dll")

FIND_FILE(EXCEL_VBA
  NAMES vbe6ext.olb
  PATHS
    "C:/Program Files/Common Files/Microsoft Shared/VBA/VBA6"
    "C:/Program Files (x86)/Common Files/Microsoft Shared/VBA/VBA6"
  DOC "Microsoft VBA Objects, path to vbe6ext.olb")

FIND_FILE(EXCEL_EXECUTABLE
  NAMES excel.exe
  PATHS
    "C:/Program Files/Microsoft Office/OFFICE12"
    "C:/Program Files (x86)/Microsoft Office/OFFICE12"
    "C:/Program Files/Microsoft Office/OFFICE11"
    "C:/Program Files (x86)/Microsoft Office/OFFICE11"
  DOC "Microsoft Excel Application, path to excel.exe")

IF(EXCEL_MSO AND EXCEL_VBA AND EXCEL_EXECUTABLE)
  SET(EXCEL_FOUND 1)
ELSE()
  SET(EXCEL_FOUND 0)
ENDIF()

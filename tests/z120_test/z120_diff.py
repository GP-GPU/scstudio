#!/usr/bin/env python
import subprocess
import difflib
import sys
import glob

p = subprocess.Popen([sys.argv[1], sys.argv[2], sys.argv[3]], stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

output = p.stderr.readlines() + p.stdout.readlines()
# print the output as it may contain valgrind error reports
# note: ctest expects the valgring errors at stderr
sys.stderr.writelines(output)

if int(sys.argv[3]) == 0:
  exit = p.wait()
  sys.exit(exit)

else:
  result_files = glob.glob(sys.argv[2] + ".result*")
  # if no result file, return the error code
  if len(result_files) == 0:
    exit = p.wait()
    sys.exit(1)

  error_count = 0
  passed_count = 0
  for result_file in result_files:
    try:
      expected_output = open(result_file, "rt")
    except IOError:
      error_count = error_count+1
      print "----------------------------------------------------------"
      print "FAILED: cannot open", result_file
      continue

    result = difflib.unified_diff(expected_output.readlines(), output)
    result_lines = 0
    # print the differences between the real and the expected output
    for line in result:
      if result_lines == 0:
        error_count = error_count+1
        print "----------------------------------------------------------"
        print "FAILED: does not match", result_file

      print line,
      result_lines = result_lines + 1

    if result_lines == 0:
      passed_count = passed_count + 1
      print "----------------------------------------------------------"
      print "PASSED: matches", result_file

  if passed_count > 0:
    sys.exit(0)
  else:
    sys.exit(error_count)

# $Id: z120_diff.py 1039 2011-02-15 17:15:11Z madzin $

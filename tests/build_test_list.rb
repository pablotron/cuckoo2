#!/usr/bin/env ruby

# get working dir
dir = File.dirname(__FILE__)

# get paths
src_path = File.join(dir, 'all_tests.txt')
dst_c_path = File.join(dir, 'list.c')
dst_h_path = File.join(dir, 'list.h')

# read tests
tests = File.readlines(src_path).map { |line| 
  line = line.strip.gsub(/\s*(#.*)?$/, '')
  (line && line.size > 0) ? line  : nil
}.compact

FMT = [
  "fprintf(stderr, \"[-- started __TEST__ --]\\n\");",
  " __TEST__(argc, argv);",
  "fprintf(stderr, \"[-- finished __TEST__ --]\\n\\n\");",
].join(' ')

# write c file
File.open(dst_c_path, 'w') do |fh| 
  fh.puts '/* automatically generated from all_tests.txt */', 
          tests.map { |test| FMT.gsub(/__TEST__/, test) }
end

# write h file
File.open(dst_h_path, 'w') do |fh| 
  fh.puts '/* automatically generated from all_tests.txt */',
          tests.map { |test| "void #{test}(int, char **);" }
end


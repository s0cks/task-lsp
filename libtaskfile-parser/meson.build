parser_inc_dir = include_directories('include')

parser = static_library(
  'taskfile-parser',
  [
    'src/token.c',
    'src/diagnostic.c',
    'src/document.c',
    'src/parser.c',
    'src/parse_result.c',
    'src/node_include.c',
    'src/node_task.c',
    'src/node_include.c',
  ],
  dependencies: all_deps,
  link_with: [],
  include_directories: [
    parser_inc_dir,
  ],
  install: true,
)
parser_dep = declare_dependency(
  link_with: [
    parser,
  ],
  dependencies: all_deps,
  include_directories: [
    parser_inc_dir,
  ],
)

if enable_testing and gtest_dep.found()
  subdir('tests')
endif

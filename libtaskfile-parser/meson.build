parser_inc_dir = include_directories('include')
parser_private_inc = include_directories('src')

parser = static_library(
  'taskfile-parser',
  [
    'src/diagnostic.c',
    'src/document.c',
    'src/lexer.c',
    'src/node.c',
    'src/node_include.c',
    'src/node_string.c',
    'src/node_task.c',
    'src/parse_result.c',
    'src/parser.c',
    'src/token.c',
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

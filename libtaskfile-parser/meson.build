enable_tracing = get_option('enable_tracing')
enable_graphviz = get_option('enable_graphviz')
enable_testing = get_option('enable_testing')

all_deps = []
if enable_graphviz
  gvc = dependency('libgvc', version: '>= 15.1.0', required: false)
  if gvc.found()
    message('graphviz is enabled')
    add_project_arguments(
      '-DTASKFILE_PARSER_GRAPHVIZ_ENABLED',
      language: ['c', 'cpp'],
    )
    all_deps += gvc
  endif
endif

tracy_dep = dependency('tracy', required: false)
if enable_tracing
  if tracy_dep.found()
    message('profiling is enabled with tracy')
    all_deps += tracy_dep
    add_project_arguments(
      '-DTASKFILE_PARSER_ENABLE_TRACING',
      language: ['c', 'cpp'],
    )
  endif
endif

if enable_testing
  gtest_dep = dependency('gtest', main: true, required: false)
endif

parser_inc_dir = include_directories('include')
parser_private_inc = include_directories('src')

parser = static_library(
  'taskfile-parser',
  [
    'src/diagnostic.c',
    'src/document.c',
    'src/lexer.c',
    'src/node.c',
    'src/node_for.c',
    'src/node_include.c',
    'src/node_string.c',
    'src/node_task.c',
    'src/parse_result.c',
    'src/parser.c',
    'src/token.c',
    'src/visit.c',
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

pkg = import('pkgconfig')
pkg.generate(
  name: meson.project_name(),
  version: meson.project_version(),
  description: 'A Language Server Protocol implementation for Taskfiles',
  libraries: [
    parser,
  ],
  requires: all_deps,
)

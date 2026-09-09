test_suites = [
  'parser',
  'lexer',
]

base_test_include_directories = [
  parser_private_inc,
  include_directories('include'),
]
base_test_deps = [
  gtest_dep,
  parser_dep,
]
foreach suite : test_suites
  subdir(suite)
  test_exec_sources = []
  foreach source : get_variable('test_' + suite + '_sources', [])
    test_exec_sources += suite / source
  endforeach
  test_exec_include_directories = base_test_include_directories + get_variable(
    'test_' + suite + '_inc',
    [],
  )
  test_exec = executable(
    'test-taskfile-parser-' + suite,
    sources: test_exec_sources,
    include_directories: test_exec_include_directories,
    dependencies: base_test_deps + get_variable('test_' + suite + '_deps', []),
    install: false,
  )
  test(
    suite + ' tests',
    test_exec,
    protocol: 'gtest',
  )
endforeach

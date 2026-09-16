project(
  'taskfile-lsp',
  [
    'c',
    'cpp',
  ],
  version: files('libtaskfile-parser' / 'VERSION'),
  default_options: [
    'c_std=c2x',
  ],
  meson_version: '>= 1.3.0',
)
add_project_arguments(
  '-DTASKFILE_PARSER_VERSION="' + meson.project_version() + '"',
  language: 'c',
)

if get_option('buildtype') == 'debug'
  message('debug mode is enabled')
  add_project_arguments('-DTASKFILE_PARSER_DEBUG', language: ['c', 'cpp'])
endif

subdir('libtaskfile-parser')

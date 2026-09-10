project(
    'hypha',
    ['c', 'cpp'],
    version: files('VERSION'),
    default_options: ['c_std=c23,c2x'],
    meson_version: '>= 1.10.0',
)
add_project_arguments(
    '-DTASKFILE_PARSER_VERSION="' + meson.project_version() + '"',
    language: 'c',
)

if get_option('buildtype') == 'debug'
    message('debug mode is enabled')
    add_project_arguments('-DTASKFILE_PARSER_DEBUG', language: ['c', 'cpp'])
endif

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

subdir('libtaskfile-parser')

pkg = import('pkgconfig')
pkg.generate(
    name: meson.project_name(),
    version: meson.project_version(),
    description: '',
    libraries: [],
    requires: all_deps,
)

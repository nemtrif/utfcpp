project('utfcpp', 'cpp',
  version: '4.0.9',
  license: 'BSL-1.0',
  meson_version: '>= 1.9.0',
  default_options: {
    'install': false
  })

opt_install = get_option('install')

headers = files(
  'source/utf8/checked.h',
  'source/utf8/core.h',
  'source/utf8/cpp11.h',
  'source/utf8/cpp17.h',
  'source/utf8/cpp20.h',
  'source/utf8/unchecked.h'
)


inc = include_directories('source')

utfcpp_dep = declare_dependency(include_directories: inc)
meson.override_dependency('utfcpp', utfcpp_dep)


# only generate the pkgconfig file if we are installing
if opt_install
  install_headers('source/utf8.h')
  install_headers(headers, install_dir: get_option('includedir') / 'utf8')

  pkg = import('pkgconfig')
  pkg.generate(
    name: meson.project_name(),
    filebase: meson.project_name(),
    url: 'https://github.com/nemtrif/utfcpp',
    description: 'UTF-8 with C++ in a Portable Way',
    version: meson.project_version(),
    license: 'BSL-1.0',
    install_dir: get_option('datadir') / 'pkgconfig')
endif

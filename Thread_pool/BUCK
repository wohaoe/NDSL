linux_linker_flags = [
  '-lpthread',
]

cxx_library(
  name = 'reuzel',
  header_namespace = '',
  exported_headers = subdir_glob([
    ('src', '**/*.h'),
  ], prefix = 'Reuzel'),
  headers = subdir_glob([
    ('src', '**/*.h'),
  ]),
  srcs = glob([
    'src/**/*.cpp',
  ]),
  platform_linker_flags = [
    ('^linux.*', linux_linker_flags),
  ],
  licenses = [
    'LICENSE',
  ],
  visibility = [
    'PUBLIC',
  ],
)

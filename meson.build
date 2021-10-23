project('crustyboy', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c89'])

src = [
  'src/main.c',
]

sdl = dependency('SDL2')

exe = executable(
  'crustyboy',
  src,
  dependencies : [sdl],
  install : true
)

test('basic', exe)

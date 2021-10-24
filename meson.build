project('gamegirl', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c89'])

src = [
  'src/main.c',
  'src/cpu.c',
  'src/gamegirl.c',
]

sdl = dependency('SDL2')

exe = executable(
  'gamegirl',
  src,
  dependencies : [sdl],
  install : true
)

test('basic', exe)

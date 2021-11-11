project('gamegirl', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c89'])

base_src = [
  'src/bus.c',
  'src/cartridge.c',
  'src/cpu.c',
  'src/cpu_utils.c',
  'src/gamegirl.c',
  'src/utils.c',
]

src = base_src + [ 'src/main.c' ]

test_src = base_src + [ 'src/unit_test.c' ]


if get_option('buildtype') == 'debug'
  add_project_link_arguments('-rdynamic', '-no-pie', language: 'c')
endif

sdl = dependency('SDL2')

exe = executable(
  'gamegirl',
  src,
  dependencies : [sdl],
  install : true
)

test_exe = executable(
  'gamegirl_test',
  test_src,
  dependencies : [sdl]
)

test('cpu', test_exe)

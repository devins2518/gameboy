project('gameboy', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c89', 'werror=true'])

base_src = [
  'src/bus.c',
  'src/cartridge.c',
  'src/cpu.c',
  'src/cpu_utils.c',
  'src/decoder.c',
  'src/gameboy.c',
  'src/instruction.c',
  'src/ppu.c',
  'src/utils.c',
]

sdl = dependency('SDL2')

src = base_src + 'src/main.c'
exe = executable('gameboy', src,
      dependencies : [sdl], install : true)

test_src = base_src + 'test/cpu.c'
test_cpu = executable('gameboy_test', test_src,
           dependencies : [sdl], c_args : '-DTESTING')

test_src = base_src + 'test/disassembler.c'
test_disassembler = executable('disassembler_test', test_src,
           dependencies : [sdl], c_args : '-DTESTING')

test('cpu', test_cpu)
test('disassembler', test_disassembler)

# Vídeo:
https://drive.google.com/file/d/1TT0qk8TX_nulOE8D83c3UKJ8hGJdZe_Z/view

# Simulação:
Para simular no renode, basta copiar os comandos abaixo:

using sysbus

$central_bin?=@(arquivo central .elf)

$peripheral_bin?=@(arquivo periférico .elf)

emulation CreateBLEMedium "wireless"

mach create "central"

machine LoadPlatformDescription @platforms/cpus/nrf52840.repl

connector Connect sysbus.radio wireless

showAnalyzer uart0 

sysbus LoadELF $central_bin

start

mach create "peripheral"

machine LoadPlatformDescription @platforms/cpus/nrf52840.repl

connector Connect sysbus.radio wireless

showAnalyzer uart0 

emulation SetGlobalQuantum "0.00001"

sysbus LoadELF $peripheral_bin

start

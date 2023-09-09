# Sistemas_Embarcados
Para simular no renode, basta copiar os comandos abaixo:

using sysbus

$central_bin?=@C:\Users\renil\OneDrive\Documentos\PlatformIO\Projects\ble_central\.pio\build\nrf52840_dk\firmware.elf

$peripheral_bin?=@C:\Users\renil\OneDrive\Documentos\PlatformIO\Projects\ble_peripheral\.pio\build\nrf52840_dk\firmware.elf

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

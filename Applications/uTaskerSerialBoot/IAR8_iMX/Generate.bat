rem Put your root directory path here:
set MY_PATH=C:\MJBC\Repositories\uTasker-GIT-Kinetis

if %2==MIMXRT1050 (set OFFSET=0x40000) else (set OFFSET=0x10000)

rem Encrypt the serial loader code
"%MY_PATH%\Tools\uTaskerConvert.exe" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Flash_BM\Exe\uTaskerSerialBoot_BM.bin" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\_temp.bin" $%3 $%4

rem Add an authentication header to the encrypted serial loader [Uploadable serial loader]
"%MY_PATH%\Tools\uTaskerConvert.exe" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\_temp.bin" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerSerialLoaderUpload.bin" -0x9%5 -%6


if %1==0 (copy "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerSerialLoaderUpload.bin" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerSerialLoaderUpload_%2.bin")

rem Combine the bare-minimum loader with the encrypted Fall-back loader [Fall-back loader image]
if %1==1 ("%MY_PATH%\Tools\uTaskerCombine.exe" "%MY_PATH%\Applications\uTaskerBoot\IAR8_iMX_RT\Objects\uTaskerBoot_%2.bin" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerSerialLoaderUpload.bin" 0x4000 "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerFallbackLoaderImage_%2.bin" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerFallbackLoaderImage_%2.hex")

rem Combine the Fall-back loader image with the encrypted serial loader [serial loader image]
if %1==0 ("%MY_PATH%\Tools\uTaskerCombine.exe" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerFallbackLoaderImage_%2.bin" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerSerialLoaderUpload.bin" %OFFSET%[0x100-FALLBACK_LD] "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerBootLoaderImage_%2.bin" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerBootLoaderImage_%2.hex")


rem Clean up
del "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\_temp.bin"
del "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerSerialLoaderUpload.bin"

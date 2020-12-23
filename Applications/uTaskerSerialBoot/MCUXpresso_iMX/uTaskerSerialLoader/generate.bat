rem generate a binary output file
arm-none-eabi-objcopy  --output-target=binary uTaskerSerialLoader.axf "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoader.bin"
if %2==MIMXRT1050 (set OFFSET=0x40000) else (set OFFSET=0x10000)
set TARGET="..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerFallbackLoaderImage_%2.bin"

rem Encrypt using AES256
..\Tools\uTaskerConvert.exe "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoader.bin" "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\_temp.bin" $%3 $%4

rem Add header and authentication to the encrypted image - this is uploaded to a board programmed with the uTaskerBootLoader [build without iMX_FALLBACK_SERIAL_LOADER]
..\Tools\uTaskerConvert.exe "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\_temp.bin" "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoaderUpload.bin" -0x9%5 -%6

if %1==0 (copy "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoaderUpload.bin" "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoaderUpload_%2.bin")

rem combine the boot loader with the serial loader (fallback loader) - this image is loaded to a fresh board  [build with iMX_FALLBACK_SERIAL_LOADER]
..\Tools\uTaskerCombine "..\Applications\uTaskerBoot\MCUXpresso_iMX\uTaskerBM_loader\uTaskerBoot_%2.bin" "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoaderUpload.bin" 0x4000 "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerBootLoaderImage.bin"

rem - Prepare the Fall-back binary archive or combine the serial loader with the Fall-back loader
if %1==1 (copy "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerBootLoaderImage.bin" %TARGET%)  else (..\Tools\uTaskerCombine %TARGET% "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoaderUpload_%2.bin" %OFFSET%[0x100-FALLBACK_LD] "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerBootComplete_%2.bin" "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerBootComplete_%2.hex")


rem Clean up
del "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\_temp.bin"
del "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoader.bin"
del "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerBootLoaderImage.bin"
del "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerSerialLoaderUpload.bin"








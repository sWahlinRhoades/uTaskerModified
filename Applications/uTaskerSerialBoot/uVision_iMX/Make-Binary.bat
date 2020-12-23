rem Put your root directory path here:
set MY_PATH=C:\MJBC\Repositories\uTasker-GIT-Kinetis

REM "uVision creates strange binary outputs which are often split into multiple files and of no use to most programming tools
REM so we use GCC objcopy to allow a single output that can be used in all cases"
..\..\..\Tools\arm-none-eabi-objcopy --input-target=ihex --output-target=binary Objects/uTaskerSerialBoot.hex uTaskerSerialBoot.bin
..\..\..\Tools\arm-none-eabi-objcopy --input-target=ihex --output-target=srec Objects/uTaskerSerialBoot.hex uTaskerSerialBoot.srec

rem Project Keys
set SECRET_KEY="aes256 secret key"
set VECTOR="initial vector"
set MAGIC=234
set AUTHENTICATION=a748b6531124

rem Encrypt the serial loader
"%MY_PATH%\Tools\uTaskerConvert.exe" uTaskerSerialBoot.bin _temp.bin $%SECRET_KEY% $%VECTOR%

rem Add header and authentication to the encrypted image - this is uploaded to a board programmed with the uTaskerBootLoader [build without iMX_FALLBACK_SERIAL_LOADER]
"%MY_PATH%\Tools\uTaskerConvert.exe" _temp.bin uTaskerSerialLoaderUpload.bin -0x9%MAGIC% -%AUTHENTICATION%
if %1==0 (copy uTaskerSerialLoaderUpload.bin "uTaskerSerialLoaderUpload_%2.bin")

rem combine the boot loader with the serial loader (fallback loader) - this image is loaded to a fresh board  [build with iMX_FALLBACK_SERIAL_LOADER]
if %1==1 ("%MY_PATH%\Tools\uTaskerCombine" "../../uTaskerBoot/uVision_iMX/uTaskerBoot_%2.bin" uTaskerSerialLoaderUpload.bin 0x4000 uTaskerFallbackLoaderImage_%2.bin uTaskerFallbackLoaderImage_%2.hex)

if %1==0 (if %2==MIMXRT1050 ("%MY_PATH%\Tools\uTaskerCombine" uTaskerFallbackLoaderImage_%2.bin "uTaskerSerialLoaderUpload_%2.bin" 0x40000[0x100-FALLBACK_LD] uTaskerBootComplete_%2.bin) else ("%MY_PATH%\Tools\uTaskerCombine" uTaskerFallbackLoaderImage_%2.bin "uTaskerSerialLoaderUpload_%2.bin" 0x10000[0x100-FALLBACK_LD] uTaskerBootComplete_%2.bin uTaskerBootComplete_%2.hex))


rem Clean up
del _temp.bin
del uTaskerSerialBoot.bin
del uTaskerSerialLoaderUpload.bin
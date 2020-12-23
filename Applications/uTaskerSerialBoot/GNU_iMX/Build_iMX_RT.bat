rem - enter your own tool chain path here if needed and check this forum entry in case the make file doesn't work correctly:
rem "https://www.utasker.com/forum/index.php?topic=1988.msg7410#msg7410"
SET PATH=%PATH%;C:\nxp\MCUXpressoIDE_11.1.0_3209\ide\tools\bin
SET PATH=%PATH%;../../../Tools
make -f make_uTaskerSerialBoot_GNU_iMX all

rem - select the target being built for in order to automate combining production file
rem - (set fallback to 1 only when building the fallback loader image and then set to 0 and build the serial loader image)
set fallback=0
set iMX_RT=iMX_RT1062_EMB_ART

set SECRET_KEY="aes256 secret key"
set VECTOR="initial vector"
set MAGIC=234
set AUTHENTICATION=a748b6531124

rem Encrypt the serial loader
uTaskerConvert.exe uTaskerSerialBoot_BM.bin _temp.bin $%SECRET_KEY% $%VECTOR%

rem Add header and authentication to the encrypted image - this is uploaded to a board programmed with the uTaskerBootLoader [build without iMX_FALLBACK_SERIAL_LOADER]
uTaskerConvert.exe _temp.bin uTaskerSerialLoaderUpload.bin -0x9%MAGIC% -%AUTHENTICATION%
if %fallback%==0 (copy uTaskerSerialLoaderUpload.bin "Objects/uTaskerSerialLoaderUpload_%iMX_RT%.bin")

rem combine the boot loader with the serial loader (fallback loader) - this image is loaded to a fresh board  [build with iMX_FALLBACK_SERIAL_LOADER]
if %fallback%==1 (uTaskerCombine "../../uTaskerBoot/GNU_iMX/Objects/uTaskerBoot_%iMX_RT%.bin" uTaskerSerialLoaderUpload.bin 0x4000 Objects/uTaskerFallbackLoaderImage_%iMX_RT%.bin Objects/uTaskerFallbackLoaderImage_%iMX_RT%.hex)

rem uTaskerCombine "../../uTaskerBoot/IAR8_iMX_RT/uTaskerBoot - Flash/Exe/uTaskerBoot.bin" uTaskerSerialLoaderUpload.bin 0x4000 uTaskerBootLoaderImageIAR.bin

if %fallback%==0 (if %iMX_RT%==MIMXRT1050 (uTaskerCombine "Objects/uTaskerFallbackLoaderImage_%iMX_RT%.bin" "Objects/uTaskerSerialLoaderUpload_%iMX_RT%.bin" 0x40000[0x100-FALLBACK_LD] "Objects//uTaskerBootComplete_%iMX_RT%.bin") else (uTaskerCombine Objects/uTaskerFallbackLoaderImage_%iMX_RT%.bin "Objects/uTaskerSerialLoaderUpload_%iMX_RT%.bin" 0x10000[0x100-FALLBACK_LD] Objects/uTaskerBootComplete_%iMX_RT%.bin Objects/uTaskerBootComplete_%iMX_RT%.hex))

rem Clean up
del _temp.bin
del uTaskerSerialLoaderUpload.bin




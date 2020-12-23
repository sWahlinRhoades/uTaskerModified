rem Generate a binary output
arm-none-eabi-objcopy --output-target=binary uTaskerV1.4_BM.axf "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_BM.bin"

if %1==MIMXRT1050 (set OFFSET=0x80000) else (set OFFSET=0x20000)

rem Add an authentication header to the binary output
..\Tools\uTaskerConvert.exe "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_BM.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_application.bin" -0x1%4 -%5

rem Encrypt the binary file
..\Tools\uTaskerConvert.exe "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_BM.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\aes256_coded.bin" $%2 $%3

rem Add an authentication header to the encrypted binary file
..\Tools\uTaskerConvert.exe "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\aes256_coded.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_AES256_%1.bin" -0x9%4 -%5

rem Generate encrypted SREC output file
..\Tools\uTaskerCombine.exe "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_AES256_%1.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\dummy.bin" 0 "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\dummy_out.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_AES256_%1.srec" 0

rem Combine the loader with the application
..\Tools\uTaskerCombine "..\Applications\uTaskerSerialBoot\MCUXpresso_iMX\uTaskerSerialLoader\uTaskerBootComplete_%1.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_AES256_%1.bin" %OFFSET%[0x100-UPLOAD_DISK] "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerCompleteImage_%1.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerCompleteImage_%1.hex"

rem Clean-up
del "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\dummy_out.bin"
del "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_BM.bin"
del "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\uTaskerV1.4_application.bin"
del "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM\aes256_coded.bin"


rem Generate a binary output
arm-none-eabi-objcopy --output-target=binary uTaskerV1.4_BM_XiP.axf "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\uTaskerV1.4_BM_XiP.bin"

if %1==MIMXRT1010 (set CODE=E=128-60020200) else (if %1==MIMXRT1050 (set CODE=E=128B-60080200) else (set CODE=E=128B-60020200))

rem Add an authentication header to the binary output
..\Tools\uTaskerConvert.exe "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\uTaskerV1.4_BM_XiP.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\uTaskerV1.4_BM_XiP_%1.bin" +"..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\boot_header.txt" -0x2%4 -%5

rem encrypt the content for on-the-fly decryption
..\Tools\uTaskerConvert.exe "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\uTaskerV1.4_BM_XiP.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\aes128_coded_XiP.bin" %CODE% $%2 $%3

rem Add an authentication header to the encrypted binary output
..\Tools\uTaskerConvert.exe "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\aes128_coded_XiP.bin" "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\uTaskerV1.4_BM_XiP_AES128_%1.bin" +"..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\boot_header.txt" -0x6%4 -%5

rem cleanup
del "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\uTaskerV1.4_BM_XiP.bin"
del "..\Applications\uTaskerV1.4\MCUXpresso_iMX\uTaskerV1.4_BM_XiP\aes128_coded_XiP.bin"

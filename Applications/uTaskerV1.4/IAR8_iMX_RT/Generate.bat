rem Put your root directory path here:
set MY_PATH=C:\MJBC\Repositories\uTasker-GIT-Kinetis

if %2==MIMXRT1050 (set OFFSET=0x80000) else (set OFFSET=0x20000)

if %1==MIMXRT1010 (set CODE=E=128-60020200) else (if %1==MIMXRT1050 (set CODE=E=128B-60080200) else (set CODE=E=128B-60020200))

rem Copy the standalone image to the Objects directory
if %1==0 (copy "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Flash\Exe\uTaskerV1.4.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_%2.bin")


rem ITC
rem Add an authentication header to the binary output
if %1==2 ("%MY_PATH%\Tools\uTaskerConvert.exe" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\ITC\Exe\uTaskerV1.4_BM.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_application.bin" -0x1%5 -%6)

rem Encrypt the binary file
if %1==2 ("%MY_PATH%\Tools\uTaskerConvert.exe" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\ITC\Exe\uTaskerV1.4_BM.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\ITC\Exe\aes256_coded.bin" $%3 $%4)

rem Add an authentication header to the encrypted binary file
if %1==2 ("%MY_PATH%\Tools\uTaskerConvert.exe" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\ITC\Exe\aes256_coded.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_AES256_%2.bin" -0x9%5 -%6)

rem Generate encrypted SREC output file
if %1==2 ("%MY_PATH%\Tools\uTaskerCombine.exe" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_AES256_%2.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\dummy_DONT_DELETE.bin" 0 "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\dummy_out.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_AES256_%2.srec" 0)

rem Combine the loader with the application
if %1==2 ("%MY_PATH%\Tools\uTaskerCombine.exe" "%MY_PATH%\Applications\uTaskerSerialBoot\IAR8_iMX\Objects\uTaskerBootLoaderImage_%2.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_AES256_%2.bin" %OFFSET%[0x100-UPLOAD_DISK] "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerCompleteImage_%2.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerCompleteImage_%2.hex")



rem XIP
rem Add an authentication header to the binary output
if %1==1 ("%MY_PATH%\Tools\uTaskerConvert.exe" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\XiP\Exe\uTaskerV1.4_XIP.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_XiP_%2.bin" +"%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\boot_header.txt" -0x2%5 -%6)

rem encrypt the content for on-the-fly decryption
if %1==1 ("%MY_PATH%\Tools\uTaskerConvert.exe" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\XiP\Exe\uTaskerV1.4_XIP.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\aes128_coded_XiP.bin" %CODE% $%3 $%4)

rem Add an authentication header to the encrypted binary output
if %1==1 ("%MY_PATH%\Tools\uTaskerConvert.exe" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\aes128_coded_XiP.bin" "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_XiP_AES128_%2.bin" +"%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\boot_header.txt" -0x6%5 -%6)




rem Clean up
if %1==2 (del "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\uTaskerV1.4_application.bin")
if %1==2 (del "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\ITC\Exe\aes256_coded.bin")
if %1==2 (del "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\dummy_out.bin")

if %1==1 (del "%MY_PATH%\Applications\uTaskerV1.4\IAR8_iMX_RT\Objects\aes128_coded_XiP.bin")


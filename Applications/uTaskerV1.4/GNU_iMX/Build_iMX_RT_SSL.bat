rem - enter your own tool chain path here if needed and check this forum entry in case the make file doesn't work correctly:
rem "https://www.utasker.com/forum/index.php?topic=1988.msg7410#msg7410"
SET PATH=%PATH%;C:\nxp\MCUXpressoIDE_11.1.0_3209\ide\tools\bin
SET PATH=%PATH%;../../../Tools

make -f make_uTaskerV1.4_SSL_GNU_iMX all

rem - select the target being built for in order to automate combining production file
set iMX_RT=iMX_RT1062_EMB_ART

set SECRET_KEY="aes256 secret key"
set VECTOR="initial vector"
set MAGIC=234
set AUTHENTICATION=a748b6531124

uTaskerConvert.exe uTaskerV1.4.bin Objects/uTaskerV1.4_XiP_%iMX_RT%.bin -0x2%MAGIC% -%AUTHENTICATION%
if %iMX_RT%==MIMXRT1010 (uTaskerConvert.exe uTaskerV1.4.bin aes128_coded_XiP.bin E=128-60020110 $%SECRET_KEY% $%VECTOR%) else (uTaskerConvert.exe uTaskerV1.4.bin aes128_coded_XiP.bin E=128B-60020110 $%SECRET_KEY% $%VECTOR%)
uTaskerConvert.exe aes128_coded_XiP.bin Objects/uTaskerV1.4_XiP_%iMX_RT%_otf.bin -0x6%MAGIC% -%AUTHENTICATION%
uTaskerConvert.exe uTaskerV1.4_BM.bin Objects/uTaskerV1.4_%iMX_RT%.bin -0x1%MAGIC% -%AUTHENTICATION%
uTaskerCombine.exe uTaskerV1.4_application.bin dummy_DO_NOT_DELETE.bin 0 dummy_out.bin Objects/uTaskerV1.4_%iMX_RT%.srec 0
uTaskerConvert.exe uTaskerV1.4_BM.bin aes256_coded.bin E=256 $%SECRET_KEY% $%VECTOR%
uTaskerConvert.exe aes256_coded.bin Objects/uTaskerV1.4_AES256_%iMX_RT%.bin -0x9%MAGIC% -%AUTHENTICATION%
uTaskerCombine.exe Objects/uTaskerV1.4_AES256_%iMX_RT%.bin dummy_DO_NOT_DELETE.bin 0 dummy_out.bin Objects/uTaskerV1.4_AES256_%iMX_RT%.srec 0

if %iMX_RT%==MIMXRT1050 (uTaskerCombine "../../uTaskerSerialBoot/GNU_iMX/Objects/uTaskerBootComplete_%iMX_RT%.bin" "Objects/uTaskerV1.4_AES256_%iMX_RT%.bin" 0x80000[0x100-UPLOAD_DISK] Objects/uTaskerCompleteImage_%iMX_RT%.bin Objects/uTaskerCompleteImage_%iMX_RT%.hex) else (uTaskerCombine "../../uTaskerSerialBoot/GNU_iMX/Objects/uTaskerBootComplete_%iMX_RT%.bin" "Objects/uTaskerV1.4_AES256_%iMX_RT%.bin" 0x20000[0x100-UPLOAD_DISK] Objects/uTaskerCompleteImage_%iMX_RT%.bin Objects/uTaskerCompleteImage_%iMX_RT%.hex)

rem Clean-up
del uTaskerV1.4.bin
del aes128_coded_XiP.bin
del dummy_out.bin
del aes256_coded.bin

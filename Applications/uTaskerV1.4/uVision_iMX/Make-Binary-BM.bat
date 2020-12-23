rem Put your root directory path here:
set MY_PATH=C:\MJBC\Repositories\uTasker-GIT-Kinetis

REM "uVision creates strange binary outputs which are often split into multiple files and of no use to most programming tools
REM so we use GCC objcopy to allow a single output that can be used in all cases"
"%MY_PATH%\Tools\arm-none-eabi-objcopy" --input-target=ihex --output-target=binary Objects/uTaskerV1_4_BM.hex uTaskerV1_4_BM.bin
"%MY_PATH%\Tools\arm-none-eabi-objcopy" --input-target=ihex --output-target=srec Objects/uTaskerV1_4_BM.hex uTaskerV1_4_BM.srec
"%MY_PATH%\Tools\uTaskerCombine" "../../uTaskerBoot/uVision_Kinetis/uTaskerBoot.bin" uTaskerV1_4_BM.bin 0x8080 uTaskerBM.bin uTaskerBM.hex
"%MY_PATH%\Tools\uTaskerConvert.exe" uTaskerV1_4_BM.bin z_Upload.bin -0x1234 -a748b6531124
"%MY_PATH%\Tools\uTaskerConvert.exe" uTaskerV1_4_BM.bin sd_card_enc.bin -0x1235 -b748b6531124 -ff25a788f2e681338777 -afe1 -c298
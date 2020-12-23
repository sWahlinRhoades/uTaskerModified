rem SET PATH=%PATH%;D:\Freescale\KDS_v3\Toolchain\bin
SET PATH=%PATH%;C:\nxp\MCUXpressoIDE_11.1.0_3209\ide\tools\bin
SET PATH=%PATH%;../../../Tools
make -f make_uTaskerV1.4_GNU_Kinetis all
uTaskerCombine "../../uTaskerBoot/GNU_Kinetis/uTaskerBoot.bin" uTaskerV1.4_BM.bin 0x1000 uTaskerBM.bin uTaskerBM.hex
uTaskerCombine "../../uTaskerSerialBoot/GNU_Kinetis/uTaskerSerialBoot.bin" uTaskerV1.4_BM.bin 0x8080 uTaskerSA.bin
uTaskerConvert.exe uTaskerV1.4_BM.bin z_Upload.bin -0x1234 -a748b6531124
rem arm-none-eabi-objcopy --input-target=ihex --output-target=binary software.hex application.bin
rem uTaskerConvert.exe application.bin software.bin -0x1234 -a748b6531124
uTaskerConvert.exe uTaskerV1.4_BM.bin software.bin -0x1234 -a748b6531124
rem uTaskerConvert.exe uTaskerV1.4_BM.bin sd_card_enc.bin -0x1235 -b748b6531124 -ff25a788f2e681338777 -afe1 -c298
uTaskerConvert.exe uTaskerV1.4_BM.bin aes256_coded.bin $"aes256 secret key" $"initial vector"
uTaskerConvert.exe aes256_coded.bin sd_card_aes256_coded.bin -0x1234 -a748b6531124
uTaskerCombine.exe aes256_coded.bin dummy.bin 0 dummy_out.bin aes256_coded.srec 0x8080




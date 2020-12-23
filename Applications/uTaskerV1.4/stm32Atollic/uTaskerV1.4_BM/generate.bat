arm-atollic-eabi-objcopy  --output-target=binary uTaskerV1_4.elf "..\Applications\uTaskerV1.4\stm32Atollic\uTaskerV1.4_BM\uTasker_V1.4_BM.bin"
arm-atollic-eabi-objcopy  --output-target=srec uTaskerV1_4.elf "..\Applications\uTaskerV1.4\stm32Atollic\uTaskerV1.4_BM\uTasker_V1.4_BM.srec"
"..\Tools\uTaskerCombine.exe" "..\Applications\uTaskerBoot\stm32Atollic\uTaskerBMBoot\uTaskerBareMinimumBoot.bin" "..\Applications\uTaskerV1.4\stm32Atollic\uTaskerV1.4_BM\uTasker_V1.4_BM.bin" 0x4000 "..\Applications\uTaskerV1.4\stm32Atollic\uTaskerV1.4_BM\uTaskerBM.bin" "..\Applications\uTaskerV1.4\stm32Atollic\uTaskerV1.4_BM\uTaskerBM.hex"
"..\Tools\uTaskerConvert.exe" "..\Applications\uTaskerV1.4\stm32Atollic\uTaskerV1.4_BM\uTasker_V1.4_BM.bin" "..\Applications\uTaskerV1.4\stm32Atollic\uTaskerV1.4_BM\1_Upload.bin" -0x5432 -622319de22b1


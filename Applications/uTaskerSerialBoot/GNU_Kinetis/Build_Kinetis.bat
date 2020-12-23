rem - enter your own tool chain path here if needed and check this forum entry in case the make file doesn't work correctly:
rem "https://www.utasker.com/forum/index.php?topic=1988.msg7410#msg7410"
SET PATH=%PATH%;C:\nxp\MCUXpressoIDE_11.1.0_3209\ide\tools\bin
SET PATH=%PATH%;../../../Tools
make -f make_uTaskerSerialBoot_GNU_Kinetis all
uTaskerCombine "../../uTaskerBoot/GNU_Kinetis/uTaskerBoot.bin" uTaskerSerialBoot_BM.bin 0x1000 uTaskerSerialBM.bin uTaskerSerialBM.hex
uTaskerConvert.exe uTaskerSerialBoot_BM.bin uTaskerSerialLoad.bin -0x1240 -f408b278017a
uTaskerCombine.exe uTaskerSerialLoad.bin dummy.bin 0 dummy_out.bin uTaskerSerialLoad.srec 0x8000
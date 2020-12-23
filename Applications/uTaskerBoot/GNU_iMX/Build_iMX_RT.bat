rem - enter your own tool chain path here if needed and check this forum entry in case the make file doesn't work correctly:
rem "https://www.utasker.com/forum/index.php?topic=1988.msg7410#msg7410"
SET PATH=%PATH%;C:/nxp/MCUXpressoIDE_11.1.0_3209/ide/tools/bin
make -f make_uTaskerBoot_GNU_iMX all

rem - select the target being built for in order to archive its binary image
set iMX_RT=iMX_RT1062_EMB_ART
copy uTaskerBoot.bin "Objects/uTaskerBoot_%iMX_RT%.bin""

rem Clean up
del uTaskerBoot.bin


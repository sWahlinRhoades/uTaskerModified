After the "uTaskerV1.4 - Flash" target has been successfully built there will be one file created in this directory:
- uTaskerV1.4_XXXX.bin (where XXXX is the name of the board configured by the uTaskerV1.4 project's Custom Argument Variable "TargetBoard"). This is a standalone image that runs in XiP without a boot loader. The use of thsi target is generally depreciated.



After the "uTaskerV1.4 - ITC" target has been successfully built there will be four files created in this directory:

- uTaskerV1.4_AES256_XXXX.bin (where XXXX is the name of the board configured by the uTaskerV1.4 project's Custom Argument Variable "TargetBoard"). This encrypted application image can be uploaded using the uTsker serial loader
- uTaskerV1.4_AES256_XXXX.srec - the same uploadable excrypted image in SREC format

- uTaskerCompleteImage_XXXX.bin - This contains the "BM" loader + "Fallback" + "serial" loader that can be progammed in a single step.
- uTaskerCompleteImage_XXXX.hex - This is the same in Intel Hex format which is useful for Teensy 4.x users who need this format to load code


After the "uTaskerV1.4 - XiP" target has been successfully built there will be two files created in this directory:
- uTaskerV1.4_BM_XiP_XXXX.bin (where XXXX is the name of the board configured by the uTaskerV1.4 project's Custom Argument Variable "TargetBoard"). This plain code XiP image can be loaded by the serial loader an executes directly in SPI flash
- uTaskerV1.4_BM_XiP_AES128_XXXX.bin is an AES128 encrypted loadable XiP image that can be uploaded using the uTasker serial loader (which will automatically configure the on-the-fly QSPI decryption needed)
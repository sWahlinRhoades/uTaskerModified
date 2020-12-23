After the uTaskerV1.4_BM_ITC target has been successfully built there will be several files created in this directory:

- uTaskerV1.4_AES256_XXXX.bin (where XXX is the name of the board configured by the MCUXpresso build variable "TargetBoard") - This is the AES256 encrypted output that can be uploaded to the board installed with the uTasker serial loader. It will be decrypted to SRAM when it operates.

- uTaskerV1.4_AES256_XXXX.srec - Equivalent to uTaskerV1.4_AES256_application but in SREC format for loading with uTasker serial loader SREC methods

- uTaskerComplete_XXXX.bin - This contains a complete image with "BM" loader + "Fall-Back" loader + "Serial Loader" + Application and can be loaded as a single step for production purposes. The files image content is fully encrypted.

- uTaskerComplete_XXXX.hex - This is equivalent to uTaskerBootComplete_XXXX.bin but in Intel Hex format, as needed by Teensy 4.x HW

WARNING: the file dummy.bin is an empty binary file that is used for an intermediate process when combining files. It should be present for this to operate correctly and thus not deleted!

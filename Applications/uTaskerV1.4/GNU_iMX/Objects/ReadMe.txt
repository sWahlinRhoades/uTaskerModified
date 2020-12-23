After the uTaskerV1.4 GCC projct has been successfully built there will be several files created in this directory:

- uTaskerV1.4_AES256_XXXX.bin (where XXXX is the name of the board configured by the bat file parameter set iMX_RT=XXXX) - This is the AES256 encrypted output that can be uploaded to the board installed with the uTasker serial loader. It will be decrypted to SRAM when it operates.
- uTaskerV1.4_AES256_XXXX.srec - Equivalent to uTaskerV1.4_AES256_XXXX.bin but in SREC format for loading with uTasker serial loader SREC methods

- uTaskerCompleteImage_XXXX.bin - This contains a complete image with "BM" loader + "Fall-Back" loader + "Serial Loader" + Application and can be loaded as a single step for production purposes. The files image content is fully encrypted.
- uTaskerCompleteImage_XXXX.hex - This is equivalent to uTaskerBootCompleteImage_XXXX.bin but in Intel Hex format, as needed by Teensy 4.x HW


- uTaskerV1.4_XXXX.bin - This is plain code output that can be uploaded to the board installed with the uTasker serial loader. It will be copied to SRAM when it operates.
- uTaskerV1.4_XXXX.srec - Equivalent to uTaskerV1.4_XXXX.bin but in SREC format for loading with uTasker serial loader SREC methods

- uTaskerV1.4_XiP_XXXX.bin - Plain code XiP (eXecute-In-Place) image that can be uploaded to the board installed with the uTasker serial loader. It is not copied to SRAM but instead runs directly from QSPI flash.
- uTaskerV1.4_XiP_XXXX_otf.bin - AES128 coded XiP (eXecute-In-Place) image that can be uploaded to the board installed with the uTasker serial loader. It is not copied to SRAM but instead runs directly from QSPI flash using On-The_flash decryption


WARNING: the file dummy_DO_NOT_DELETE.bin in the above directry is an empty binary file that is used for an intermediate process when combining files. It should be present for this to operate correctly and thus not deleted!



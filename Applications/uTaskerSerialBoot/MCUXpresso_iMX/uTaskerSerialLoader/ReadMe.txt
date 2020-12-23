After the uTaskerFallbackLoader target has been successfully built there will be one file created in this directory:
- uTaskerFallbackLoaderImage_XXXX.bin (where XXX is the name of the board configured by the MCUXpresso build variable "TargetBoard").
This contains the "BM" loader + "Fallback" loader that can be progammed in a single step. It is also used as input to building the "Serial" loader full image.

Note that the corresponding "BM" loader should have been built previously for this to be able to operate successfully.




After the uTaskerSerialBoot target has been successfully built there will be three files created in this directory:
- uTaskerBootComplete_XXXX.bin (where XXX is the name of the board configured by the MCUXpresso build variable "TargetBoard").
This contains the "BM" loader + "Fallback" loader + "Serial" loader that can be progammed in a single step. It is also used as input to building the "Application" full image.
- uTaskerBootComplete_XXXX.hex is equivalent to uTaskerBootComplete_XXXX.bin but in Intel Hex format, as required by Teensy 4.X users.
- uTaskerSerialLoaderUpload_XXXX.bin is the encrypted "Serial" loader in a format that can be uploaded to the HW using the "Fall-back" serial loader in order to update the serial loader used by the product.

Note that the corresponding "Fall-back" loader should have benn built previously for this to be able to operate successfully.




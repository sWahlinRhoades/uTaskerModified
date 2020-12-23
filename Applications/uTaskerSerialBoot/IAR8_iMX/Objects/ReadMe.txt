After the "uTaskerSerialBoot - FallBackLoader" target has been successfully built there will be two files created in this directory:

- uTaskerFallbackLoaderImage_XXXX.bin (where XXXX is the name of the board configured by the uTaskerV1.4 project's Custom Argument Variable "TargetBord")

This contains the "BM" loader + "Fallback" loader that can be progammed in a single step. It is also used as input to building the "Serial" loader full image.
- uTaskerFallbackLoaderImage_XXXX.hex This is the same in Intel Hex format which is useful for Teensy 4.x users who need this format to load code

Note that the corresponding "BM" loader should have been built previously for this to be able to operate successfully.



After the "uTaskerSerialBoot - SerialLoader" target has been successfully built there will be three files created in this directory:
- uTaskerBootComplete_XXXX.bin (where XXXX is the name of the board configured by the uTaskerV1.4 project's Custom Argument Variable "TargetBord").
This contains the "BM" loader + "Fallback" loader + "Serial" loader that can be progammed in a single step. It is also used as input to building the "Application" full image.
- uTaskerBootComplete_XXXX.hex is equivalent to uTaskerBootComplete_XXXX.bin but in Intel Hex format, as required by Teensy 4.X users.
- uTaskerSerialLoaderUpload_XXXX.bin is the encrypted "Serial" loader in a format that can be uploaded to the HW using the "Fall-back" serial loader in order to update the serial loader used by the product.

Note that the corresponding "Fall-back" loader should have been built previously for this to be able to operate successfully.
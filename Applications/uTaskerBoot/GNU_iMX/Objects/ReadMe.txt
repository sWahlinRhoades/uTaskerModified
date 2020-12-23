After the uTaskerBoot target has been successfully built by GCC with the bat file there will be one file created in this directory:

- uTaskerBoot_XXXX.bin (where XXXX is the name of the board configured by the iMX_RT valuable in the build bat file "Build_iMX_RT.bat")

This is used as input to the fall-back serial loader build and cannot be used stand-alone.

WARNING: Beware that this binary file contains the project's security keys in readable form and so must never be distributed!
Once loaded and operated together with the uTasker Fall-back (or serial) loader the keys will be saved securely and the readable version destroyed.

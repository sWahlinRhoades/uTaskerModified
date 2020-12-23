The FT800 emulator from FTDI is supplied as VS 2012 and VS 2015 versions.
Copy the corresponding library file from one of the sub-folders to here in case a different version is needed.
Take the ft8xxemu.dll file from the appropriate sub-directory and copy it to the simulator's working directory to ensure that its can be found at run-time (eg. copy to \Applications\uTaskerV1.4\Simulator\uTasker___Win32_uTasker_Kinetis_build)

When using VS 2012 version ensure that FT8XXEMU_VERSION_API 9 is set in WinSimMain.cpp.
When using VS 2015 version ensure that FT8XXEMU_VERSION_API 10 is set in WinSimMain.cpp.
In case they don't match the library used the FT800 emulator will not start and an exception will result to indicate the mismatch so that it can be quickly identified.


2.10.2019:
BT8XX emulator added (to 2019 sub-directory).
bt8xxemu.lib is located here and linked to by the VS project.
bt8xxemu.dll shoudl be copied from the 2019 folder to the simulator director when used.
Set BT800_EMULATOR (rather than FT800_EMULATOR) in config.h to use the newer version, which supports also FT815 and FT816 display types.
When using the BT8XX emulator ensure that BT8XXEMU_VERSION_API 11 is set in WinSimMain.cpp (presently only setting)


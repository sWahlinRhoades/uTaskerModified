After the uTaskerV1.4_BM_XiP target has been successfully built there will be two files created in this directory:
- uTaskerV1.4_BM_XiP_XXXX.bin (where XXXX is the name of the board configured by the MCUXpresso post-build bat file parameter) which is a plain-code loadable XiP image that can be uploaded using the uTasker serial loader
- uTaskerV1.4_BM_XiP_AES128_XXXX.bin which is an AES128 encrypted loadable XiP image that can be uploaded using the uTasker serial loader (which will automatically configure the on-the-fly QSPI decryption needed)

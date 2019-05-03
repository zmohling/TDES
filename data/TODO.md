##TO-DO TDES
*Fix 64 bit truncation at EOF
*Full TDES implementation DES -> TDES
*Instead of loading the entire file into a buffer, read and write x number of bytes at a time to lower memory cost
*Multithreading for concurrent encryption/decryption
*Validation of keys before decryption
*Checksum at EOF to validate successful encryption/decryption (maybe)
*Increase progress status accuracy by including read, write, and key generation
*Documentation with Doxygen
*Consider building as a <popular *nix package manager> package.
*Cross-platform compatible with Windows and OSX
*Implement header file for use as a library

>DISCLAIMER: Due to limited review and the ECB mode of operation, my implementation of TDES is not suited for any purpose other than education. I am not a professional cryptographer.

# TDES
An implementation of the [Triple Data Encryption Standard](https://en.wikipedia.org/wiki/Data_Encryption_Standard) (TDES) specification as a Linux and OSX command-line tool.

![demo.gif](https://raw.githubusercontent.com/zmohling/TDES/master/data/demo.gif)

### Purpose
This project served as a personal introduction to cryptographic block ciphers, symmetrical encryption, and the general paradigm of security-first software development.

### Usage 
`tdes [-enc|-dec] <src path> <dest path>
`
### Installation
`make && sudo make install

### Dependencies
* g++
* make
* openssl (via Homebrew on OSX)

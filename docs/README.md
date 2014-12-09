# Documentation
This folder contains all documentation about the Initium and files who are used
like reference to develop some functionalities.

# Table of Contents

    1. Serial Console

## 1. Serial Console
Serial console are used to DEBUG the LAOS in case of failure, the implementation
uses UART NS16550 definitions and the code are based on this
[website](http://www.freebsd.org/doc/en_US.ISO8859-1/articles/serial-uart/article.html).

The Universal Asynchronous Receiver/Transmitter (UART) controller is the key
componente of the serial communication subsystem of a computer.
The UART takes bytes of data and transmits the individual bits in a sequencial
fashion. At the destination, a second UART re-assembles the bits into complete
bytes.

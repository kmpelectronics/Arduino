Welcome to KMPDinoEthernet Library!
===================


This library supports [ProDiNo NetBoard V2.1](http://www.kmpelectronics.eu/en-us/products/prodinoethernet.aspx). More information you can see on our site http://www.kmpelectronics.eu/.

----------

Installation
-------------

This library includes two files. The first includes Ethernet library which it supports Ethernet chip. The second includes board code and examples. The latest version of the library files you can download from [here](https://github.com/kmpelectronics/Arduino/tree/master/KMPDinoEthernet/Releases/Last) and they have to save on your desktop.
#### The installation has two steps:
1. Ethernet library install (Sketch > Include Library > Add .ZIP Library...)
2. KMPDinoEthernet install (Sketch > Include Library > Add .ZIP Library...)

###How to use Ethernet library with different chips 
This library supported the follow chips: WIZnet  W5100, WIZnet  W5200 and WIZnet  W5500
In a file Ethernet\src\utility\w5100.h has block, which chip is using  at the moment.
By default for ProDiNo NetBoard V2.1 board we use W5200_ETHERNET_SHIELD and KMPDINOETHERNET.

>//#define W5100_ETHERNET_SHIELD // Default. Arduino Ethernet Shield and Compatibles ...
>#define W5200_ETHERNET_SHIELD // WIZ820io, W5200 Ethernet Shield 
>#define KMPDINOETHERNET // This is set KMP board W5200 specific configuration.
>//#define W5500_ETHERNET_SHIELD   // WIZ550io, ioShield series of WIZnet

If you use standard **Arduino Ethernet shield with W5100**, you have to make follow changes:
>  #define W5100_ETHERNET_SHIELD // Default. Arduino Ethernet Shield and Compatibles ...
>//#define W5200_ETHERNET_SHIELD // WIZ820io, W5200 Ethernet Shield 
>//#define KMPDINOETHERNET // This is set KMP board W5200 specific configuration.
>//#define W5500_ETHERNET_SHIELD   // WIZ550io, ioShield series of WIZnet

If you use standard **Arduino Ethernet with W5200**, you have to make follow changes:
>//#define W5100_ETHERNET_SHIELD // Default. Arduino Ethernet Shield and Compatibles ...
>  #define W5200_ETHERNET_SHIELD // WIZ820io, W5200 Ethernet Shield 
>//#define KMPDINOETHERNET // This is set KMP board W5200 specific configuration.
>//#define W5500_ETHERNET_SHIELD   // WIZ550io, ioShield series of WIZnet

If you use standard **Arduino Ethernet shield with W5500**, you have to make follow changes:
>//#define W5100_ETHERNET_SHIELD // Default. Arduino Ethernet Shield and Compatibles ...
>//#define W5200_ETHERNET_SHIELD // WIZ820io, W5200 Ethernet Shield 
>//#define KMPDINOETHERNET // This is set KMP board W5200 specific configuration.
>  #define W5500_ETHERNET_SHIELD   // WIZ550io, ioShield series of WIZnet

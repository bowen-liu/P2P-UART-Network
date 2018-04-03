# Peer-to-Peer Networking Over UART

This project proposes how a network stack can be potentially implemented on top of the ubiquitious serial port, allowing multiple microcontrollers to communicate with each other. Specifically, a link layer will be overlayed on top of the standard UART physical interface, transforming it from a simple end-to-end protocol into one supporting structured multi-way communication, P2P and ad-hoc capabilities, and dynamic membership management to allow devices join and leave the network at any time. The project aims to implement suffice networking primitives at the link layer, so that more complex networking layers can be added on top. 

**The reader is highly recommended to not read the code provided here, but read the provided documentations instead.** The included code's purpose was to only to provide a functional prototype ready for presentation. The code is poorly structured, unoptimized, and largely uncommented, requiring a lot of cleanup still.

The reader should refer to _Final Report.pdf_ to get a deep understanding of the proposed networking protocol, or _presentation_poster.pdf_ for a summerized version in point form. Both files documents all aspects of the code included.

Below is an image showing the hardware setup for the presentation demo, with 3 nodes and 1 switch:
![alt text](https://github.com/bowen-liu/P2P-UART-Network/raw/master/demosetup.jpg)


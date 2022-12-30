# battery-monitor (lora-receiver with a 128x128 OLED screen)
Based on Jim Booth's Boat Monitor hardware and software. Thanks, Jim!

This software runs the "base station" of Jim's transmitter / receiver setup, where each thing that's being monitored (a boat, a truck, a swimming pool) has a transmitter that gathers the data and sends it out into the ether (that's how LoRa works), and a receiver that's always waiting for data from transmitters. Initially, each transmitter was fairly customized for what it was monitoring, and the receiver was VERY customized for what it was hoping to receive.

In October and November 2022, I finally got a 3D printer and started playing with it, and my first project is to add a larger OLED display to this project. It will involve a lot of coding changes, so I've mirrored the "lora-receiver" repository to this new repository, and this is where I'll do all of the work to utilize the new, bigger (128x128 vs. 128x64) display.

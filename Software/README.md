[![LoraType](https://raw.githubusercontent.com/AutomationArt/LoraType/main/Image/LoraType.png "LoraType")](https://raw.githubusercontent.com/AutomationArt/LoraType/main/Image/LoraType.png "LoraType")

### LoraType Now  v2.0.2

### Latest changes 2.0.2

**- Changed**
1. 	Changed the approach of working with the keyboard. The method of polling the keyboard is changed to interrupt operation.
2. Removed unused variables and functions
3. Explicit indication of the use of a particular type of variable data (not everywhere, in the process)
4. Added lora module settings (frequencies and communication quality)
5. Added esp32 processor frequency setting item 
6. The function prototypes are shown separately and comments are added
7. Moving away from using Serial to ESP_LOGD()
8. Changing all blocking delay() to nonblocking vTaskDelay()
9. With activated self-messaging (for debugging) added the display of user signal quality next to the message
10. Added notification window (without windows, of course)
11. Changed the work with the general menu for general understanding (still not optimal)

**-  Future planned changes** 
1. Evasion of confirmation when requesting firmware. The server requires confirmation of the device to enter the device pool to update, this must be confirmed manually, it is not a decentralized solution depending on the update server.
2. Return capital letters on the keyboard
3. Individual chat and on-air #tags are not available at the moment, but the functionality does exist.
4. The LED does not support control at less than 80mhz - this is a hardware solution, but if the frequency is less, there is no need to use the LED - economy mode. Implement LED shutdown at lower frequency, may hang up.
5. Sleep mode, which is turned on manually or activated by pressing on the keyboard of the device, and wakes up when you receive Lora messages. We have already tested this, but the library did not allow to implement it.

**Always open to your suggestions, because we are an open project for people.** 
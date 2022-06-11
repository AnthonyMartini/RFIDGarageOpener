# RFIDGarageOpener
Arduino-powered RFID garage door opener.

<h2>EEPROM</h2>
All cards that are added to the garage opener are stored in the Arduino's EEPROM memory. There is the ability to add 40 cards to the reader, chosen arbitrarily.

In the EEPROM memory, the cards are stored sequentially, with certain bits signalling the start/end of a card.


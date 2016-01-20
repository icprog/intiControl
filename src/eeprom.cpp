/*
 * This file is part of intiLED.
 *
 * intiLED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * intiLED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with intiLED.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Original file built from https://github.com/thijse/Arduino-EEPROMEx
 *
 */

#include <eeprom.h>

Eeprom::Eeprom()
  :  m_allowedWrites(1024)
{
    while (!isReady());
}

 /**
 * Set starting position and memory size that EEPROMEx may manage
 */
void Eeprom::setMemPool(int base, int memSize)
{
    //Base can only be adjusted if no addresses have already been issued
    if (m_nextAvailableaddress == m_base)
        m_base = base;

    m_nextAvailableaddress=m_base;

    //Ceiling can only be adjusted if not below issued addresses
    if (memSize >= m_nextAvailableaddress )
        m_memSize = memSize;
}

/**
 * Set global maximum of allowed writes
 */
void Eeprom::setMaxAllowedWrites(int allowedWrites)
{
#ifdef _EEPROMEX_DEBUG
    _allowedWrites = allowedWrites;
#endif
}

/**
 * Get a new starting address to write to. Adress is negative if not enough space is available
 */
int Eeprom::getAddress(int noOfBytes)
{
    int availableaddress   = m_nextAvailableaddress;
    m_nextAvailableaddress += noOfBytes;

    return availableaddress;
}

/**
 * Check if EEPROM memory is ready to be accessed
 */
bool Eeprom::isReady()
{
    return eeprom_is_ready();
}

/**
 * Read a single byte
 * This function performs as readByte and is added to be similar to the EEPROM library
 */
uint8_t Eeprom::read(int address)
{
    return readByte(address);
}

/**
 * Read a single bit
 */
bool Eeprom::readBit(int address, uint8_t bit)
{
      if (bit > 7)
          return false;

      if (!isReadOk(address+sizeof(uint8_t)))
          return false;

      uint8_t byteVal =  eeprom_read_byte((unsigned char *) address);
      uint8_t bytePos = (1 << bit);

      return (byteVal & bytePos);
}

/**
 * Read a single byte
 */
uint8_t Eeprom::readByte(int address)
{
    if (!isReadOk(address + sizeof(uint8_t)))
        return 0;

    return eeprom_read_byte((unsigned char *) address);
}

/**
 * Read a single 16 bits integer
 */
uint16_t Eeprom::readInt(int address)
{
    if (!isReadOk(address + sizeof(uint16_t)))
        return 0;

    return eeprom_read_word((uint16_t *) address);
}

/**
 * Read a single 32 bits integer
 */
uint32_t Eeprom::readLong(int address)
{
    if (!isReadOk(address + sizeof(uint32_t)))
        return 0;

    return eeprom_read_dword((unsigned long *) address);
}

/**
 * Read a single float value
 */
float Eeprom::readFloat(int address)
{
    if (!isReadOk(address+sizeof(float)))
        return 0;

    float _value;

    readBlock<float>(address, _value);
    return _value;
}

/**
 * Read a single double value (size will depend on board type)
 */
double Eeprom::readDouble(int address)
{
    if (!isReadOk(address + sizeof(double)))
        return 0;

    double _value;
    readBlock<double>(address, _value);

    return _value;
}

/**
 * Write a single byte
 * This function performs as writeByte and is added to be similar to the EEPROM library
 */
bool Eeprom::write(int address, uint8_t value)
{
    return writeByte(address, value);
}

/**
 * Write a single bit
 */
bool Eeprom::writeBit(int address, uint8_t bit, bool value)
{
    updateBit(address, bit, value);
    return true;
}

/**
 * Write a single byte
 */
bool Eeprom::writeByte(int address, uint8_t value)
{
    if (!isWriteOk(address + sizeof(uint8_t)))
        return false;

    eeprom_write_byte((unsigned char *) address, value);
    return true;
}

/**
 * Write a single 16 bits integer
 */
bool Eeprom::writeInt(int address, uint16_t value)
{
    if (!isWriteOk(address + sizeof(uint16_t)))
        return false;

    eeprom_write_word((uint16_t *) address, value);
    return true;
}

/**
 * Write a single 32 bits integer
 */
bool Eeprom::writeLong(int address, uint32_t value)
{
    if (!isWriteOk(address + sizeof(uint32_t)))
        return false;

    eeprom_write_dword((unsigned long *) address, value);
    return true;
}

/**
 * Write a single float value
 */
bool Eeprom::writeFloat(int address, float value)
{
    return (writeBlock<float>(address, value)!=0);
}

/**
 * Write a single double value (size will depend on board type)
 */
bool Eeprom::writeDouble(int address, double value)
{
    return (writeBlock<float>(address, value)!=0);
}

/**
 * Update a single byte
 * The EEPROM will only be overwritten if different. This will reduce wear.
 * This function performs as updateByte and is added to be similar to the EEPROM library
 */
bool Eeprom::update(int address, uint8_t value)
{
    return (updateByte(address, value));
}

/**
 * Update a single bit
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool Eeprom::updateBit(int address, uint8_t bit, bool value)
{
      if (bit > 7) return false;

      uint8_t byteValInput  = readByte(address);
      uint8_t byteValOutput = byteValInput;

      // Set bit
      if (value)
        byteValOutput |= (1 << bit);  //Set bit to 1
      else
        byteValOutput &= ~(1 << bit); //Set bit to 0

      // Store if different from input
      if (byteValOutput!=byteValInput)
        writeByte(address, byteValOutput);

      return true;
}


/**
 * Update a single byte
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool Eeprom::updateByte(int address, uint8_t value)
{
    return (updateBlock<uint8_t>(address, value)!=0);
}

/**
 * Update a single 16 bits integer
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool Eeprom::updateInt(int address, uint16_t value)
{
    return (updateBlock<uint16_t>(address, value)!=0);
}

/**
 * Update a single 32 bits integer
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool Eeprom::updateLong(int address, uint32_t value)
{
    return (updateBlock<uint32_t>(address, value)!=0);
}

/**
 * Update a single float value
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool Eeprom::updateFloat(int address, float value)
{
    return (updateBlock<float>(address, value)!=0);
}

/**
 * Update a single double value (size will depend on board type)
 * The EEPROM will only be overwritten if different. This will reduce wear.
 */
bool Eeprom::updateDouble(int address, double value)
{
    return (writeBlock<double>(address, value)!=0);
}

/**
 * Performs check to see if writing to a memory address is allowed
 */
bool Eeprom::isWriteOk(int address)
{
#ifdef _EEPROMEX_DEBUG
    _writeCounts++;
    if (_allowedWrites == 0 || _writeCounts > _allowedWrites ) {
        return false;
    }

    if (address > _memSize) {
        return false;
    } else {
        return true;
    }
#endif
    return true;
}

/**
 * Performs check to see if reading from a memory address is allowed
 */
bool Eeprom::isReadOk(int address)
{
#ifdef _EEPROMEX_DEBUG
    if (address > _memSize) {
        return false;
    } else {
        return true;
    }
#endif
    return true;
}

int Eeprom::m_base= 0;
int Eeprom::m_memSize= 512;
int Eeprom::m_nextAvailableaddress= 0;
int Eeprom::_writeCounts =0;

Eeprom EEPROM;

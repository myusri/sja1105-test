# Testing Configuring NXP SJA1105 Ethernet Switch

SPI is used for the static configuration of the Ethernet switch. For this
project we will use Microchip MCP2210 USB-to-SPI bridge to ease interfacing to
switch.

Some points to note:

* The switch operates as an SPI slave device in Mode 1 with CPOL = 0 (polarity)
  and CPHA = 1 (phase) -- clock idle at logic low, and data is sampled on
  falling edge and shifted out on the rising edge.
* A frame consisting of access type, address and data must be encoded within a
  single SPI transaction.
    - 32b addressing scheme (register size) is used.
    - Write access consists of 32b control word followed by one or more 32b
      data words. Access type is encoded in the MSB, bit\[31], of the control 
      word. Value 1 means it's a write access. Bits\[24:4] (21b) are the
      address to start writing. Control and data words are mirrored to SDO 
      during a write operation.
    - For read access, the access bit, bit\[31], of the control word is set
      to 0. Bits\[30:25] (6b) is the number of 32b words to be read. The 
      control word, which has the address to start reading in Bits\[24:4]
      (21b), is mirrored to SDO before the 32b words being read is streamed
      out.
* While the SPI clock supports up to 25MHz, it is best to keep it under 17.8MHz 
  because otherwise some hardware needs to provision for some timing 
  requirement.
* Note that the SJA1105 will not enable its ports, i.e., it won't be 
  functional, without the static configuration being done. All xMII ports 
  will be tri-stated (except for the SGMII, but we will not cover that for
  now).
* Unused ports should be disabled in software (SPI).

## Configuration

After startup, the switch expects a stream of data to the Static Configuration
Interface (via SPI), starting from address 20000h, the configuration area.

Once configured statically, subsequent configuration can be done through the
Dynamic Control Interface (also via SPI).

The configuration area is write-only via the Static Configuration Interface.
Some parts can be read later via the Dynamic Control Interface.

The static configuration data can be downloaded in one go in a single SPI
transaction, or it can be split into multiple transactions, based on the
generic loader format below. 

Configuration need to be restarted if the previous configuration is failed.
In this case the CONFIGS flag stays de-asserted. Once successfully configured,
the CONFIGS flag will be asserted. The CONFIGS flags is in the Initial device
configuration flag register.

32b core device IDs:

| Device     | Switch core device ID | Part No |
|------------|-----------------------|---------|
| SJA1105PEL | AF00030Eh             | 9A84h   |
| SJA1105QEL | AE00030Eh             | 9A85h   |
| SJA1105REL | AF00030Eh             | 9A86h   |
| SJA1105SEL | AE00030Eh             | 9A87h   |

Generic loader format:

    |=========================================|
    | Device ID (32b)                         |
    |=========================================|
    | BlockID (8b) | Unused (24b)             |
    | Unused (8b)  | BlockLength (24b)        |
    | CRC (32b)                               |
    |-----------------------------------------|
    | Data 1 (32b)                            |
    | ...                                     |
    | Data N (32b)                            |
    | CRC (32b)                               |
    |=========================================|
    | 0, 1, or more blocks                    |
    |=========================================|
    | Unused (32b)                            |
    | Unused (8b)  | 0 (24b)                  |
    | Global CRC (32b)                        |
    |=========================================|

* Device ID (32b) followed by sequence of configuration blocks.
* Each block starts with 8b BlockID followed by BlockLength. The block header
  has CRC protection.
* Block header is followed by sequence of 32b words. The data words are CRC
  protected also.
* CRC is the CRC-32 Ethernet calculate from the lowest byte to the highest byte 
  in each word.

Each block is a stream of 32b words that defines one or more entries. Each 
entry can take multiple of words arranged from the least significant word
address to the most significant word address. The bit fields for the entries
depends on the type of the table.

## Other References

* [SJA1105 Tools](https://github.com/nxp-archive/openil_sja1105-tool)


  



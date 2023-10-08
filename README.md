# Testing Configuring NXP SJA1105 Ethernet Switch

SPI is used for the static configuration of the Ethernet switch. For this
project we will use Microchip MCP2210 USB-to-SPI bridge to ease interfacing to
switch. We will also not focus on the Time-Triggered Ethernet feature. 

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

## Static Configuration

After startup, the switch expects a stream of data to the Static Configuration
Interface (via SPI), starting from address 20000h, the configuration area.

Once configured statically, subsequent configuration can be done through the
Dynamic Control Interface (also via SPI).

The configuration area is write-only via the Static Configuration Interface.
Some parts can be read later via the Dynamic Control Interface.

The static configuration data can be downloaded in one go in a single SPI
transaction, or it can be split into multiple transactions, based on the
generic loader format below. 

Configuration need to be restarted if the previous configuration failed. In 
this case the CONFIGS flag stays de-asserted. Once successfully configured,
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
depend on the type of the table.

The minimum required configuration tables for static configuration:

| Block ID | Table                         | Note               |
|----------|-------------------------------|--------------------|
| 06h      | L2 Policing Table             | At least one entry |
| 08h      | L2 Forwarding Table           |
| 09h      | MAC Configuration Table       |
| OEh      | L2 Forwarding Parameter Table |
| 11h      | General Parameters Table      |
| 4Eh      | xMII Mode Parameter Table     |

### L2 Policing Table

* It defines policing rules and priority for each port.
* At least entry 0 needs to be loaded. Ethernet frames with no policy entry will
  default to entry 0.
* All frames will be dropped if entry 0 is not loaded.
* Initially, bandwidth credit for an entry is set to SMAX. For every valid 
  Ethernet frame, bandwidth credit is decreased by the number of bytes in the
  frame.
* At the same time, for every 8μs, bandwidth credit is incremented by (RATE/64)
  bytes. RATE is set to the desired data rate (bps) divided by 15625, e.g., for
  1Gbps, RATE is 64000.
  
L2 Policing Table entry:

| Bit   | Size | Field     | Note                                                   |
|-------|------|-----------|--------------------------------------------------------|
| 63:58 | 6b   | SHARINDX  | Point to shared policing entry                         |
| 57:42 | 16b  | SMAX      | Max burst size (bytes) for received frames             |
| 41:26 | 16b  | RATE      | Admitted bandwidth (multiple of 15.625 kbps)           |
| 25:15 | 11b  | MAXLEN    | Max frame length (bytes) including overhead. Max 2043. |
| 14:12 | 3b   | PARTITION | L2 memory partition to take the Ethernet frame from    |

## Dynamic Configuration

These configuration tables can only be configured dynamically during runtime.

| Table                              | Note                               |
|------------------------------------|------------------------------------|
| L2 Address Lookup Table            | For dynamically learned L2 entries |
| Management L2 Address Lookup table | For management frames sent by host |



## Other References

* [SJA1105 Tools](https://github.com/nxp-archive/openil_sja1105-tool)


  



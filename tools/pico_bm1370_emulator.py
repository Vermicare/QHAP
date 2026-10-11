"""Pico H (RP2040) synthetic BM1370 UART stimulus for an ISOLATED test ESP32.

Works with Pico MicroPython. Packet builders are also CPython-compatible
for host-only tests. This is NOT a BM1370 ASIC, Bitcoin miner, or pool client.

Manual Pico MicroPython REPL use (after powering Pico and confirmed 3.3 V-compatible experimental
ESP32 via separate USB leads and verifying 3.3 V signal wiring):
    import pico_bm1370_emulator as mock
    mock.start_uart_test(frames=10)

Wiring only for the later physical UART test:
    Pico H GP0 (physical pin 1)  -> experimental ESP32's verified 3.3 V RX GPIO
    Pico H GND (physical pin 3)  -> experimental ESP32 GND
Do not connect Pico VBUS/VSYS/3V3 to the ESP32's power pins.
DO NOT connect to the known-good live Bitcoin miner or public pool.
"""

PREAMBLE = b"\xAA\x55"
RESULT_SIZE = 11


def crc5(data):
    """Match QHAP qhap_bm1370_crc5: polynomial 0x05, initial 0x1F."""
    crc = 0x1F
    for octet in data:
        for bit in range(7, -1, -1):
            feedback = ((octet >> bit) & 1) ^ ((crc >> 4) & 1)
            crc = (crc << 1) & 0x1F
            if feedback:
                crc ^= 0x05
    return crc


def make_result(nonce, raw_id=0x5A, raw_version=0, job_response=True,
                midstate_num=0):
    """Return a checksummed 11-byte SYNTHETIC BM1370-format result.

    Big-endian result fields match the repository's existing QHAP mock
    packet and parser. No proof-of-work is performed.
    """
    if (not 0 <= nonce <= 0xFFFFFFFF or
            not 0 <= raw_id <= 255 or
            not 0 <= raw_version <= 0xFFFF or
            not 0 <= midstate_num <= 255):
        raise ValueError("out-of-range BM1370 mock field")
    packet = bytearray(RESULT_SIZE)
    packet[0:2] = PREAMBLE
    packet[2:6] = nonce.to_bytes(4, "big")
    packet[6] = midstate_num
    packet[7] = raw_id
    packet[8:10] = raw_version.to_bytes(2, "big")
    flag = 0x80 if job_response else 0
    for bits in range(32):
        packet[10] = flag | bits
        if crc5(packet[2:]) == 0:
            return bytes(packet)
    raise ValueError("unable to synthesize CRC5 residue-zero packet")


def corrupt_payload(packet):
    """Flip a checksum-protected nonce bit without recomputing CRC5."""
    if len(packet) != RESULT_SIZE:
        raise ValueError("expected 11-byte result")
    broken = bytearray(packet)
    broken[5] ^= 0x01
    return bytes(broken)


def start_uart_test(frames=10, baud=115200, gap_ms=1000,
                    corrupt_every=0):
    """Transmit a small finite stream only after explicit REPL invocation.

    Pico UART0 TX=GP0; RX=GP1 (unused by this simple output test).
    The receiver must be the ISOLATED experimental ESP32 (model to be identified), never
    the original production miner. Synthetic frames are not shares.
    """
    if not 1 <= frames <= 1000:
        raise ValueError("frames must be from 1 to 1000")
    if baud not in (9600, 19200, 57600, 115200):
        raise ValueError("use a known safe UART baud rate")
    if not 1 <= gap_ms <= 60000:
        raise ValueError("gap_ms must be from 1 to 60000")
    if not 0 <= corrupt_every <= 1000:
        raise ValueError("invalid corrupt_every")
    from machine import UART, Pin
    from time import sleep_ms
    uart = UART(0, baudrate=baud, tx=Pin(0), rx=Pin(1))
    try:
        for i in range(frames):
            packet = make_result(0x01020304 + i, 0x5A, 0, True)
            if corrupt_every and (i + 1) % corrupt_every == 0:
                packet = corrupt_payload(packet)
            sent = uart.write(packet)
            if sent != RESULT_SIZE:
                raise OSError("UART short write")
            print("mock BM1370 frame", i + 1, packet.hex())
            sleep_ms(gap_ms)
    finally:
        uart.deinit()


if __name__ == "__main__":
    print("Pico mock loaded but UART is NOT started automatically.")
    print("On a Pico, import this module and call start_uart_test() explicitly.")

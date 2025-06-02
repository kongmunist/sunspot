import atexit
import IPython
import math
from PIL import Image
import serial
import serial.tools
import struct
import sys
import time

CMD_REG_WRITE = 0xAA
CMD_REG_READ = 0xBB
CMD_CAPTURE = 0xCC

# Print possible serial ports
# print(serial.tools.list_ports.comports())
print('before serial init')
# ser = serial.Serial(sys.argv[1], 1000000, rtscts=True, dsrdtr=True)
# ser = serial.Serial(sys.argv[1], 115200, rtscts=True, dsrdtr=True)
ser = serial.Serial(sys.argv[1], 2e6, rtscts=True, dsrdtr=True, timeout=1)
atexit.register(lambda: ser.close())
print('after serial init')

# ======================
# Actual device commands
# ======================
def reg_write(reg, value):
    ser.reset_output_buffer()
    ser.reset_input_buffer()

    assert isinstance(reg, int)
    assert 0x00 <= reg <= 0xFF
    assert isinstance(value, int)
    assert 0x00 <= value <= 0xFF

    ser.write(struct.pack('BBB', CMD_REG_WRITE, reg, value))

def reg_read(reg):
    ser.reset_output_buffer()
    ser.reset_input_buffer()

    assert isinstance(reg, int)
    assert 0x00 <= reg <= 0xFF

    print("reg read, write BBB")
    ser.write(struct.pack('BB', CMD_REG_READ, reg))

    print("about to return")
    return ser.read(1)[0]

def capture(filename):
    # print("resetting serial buffers")
    ser.reset_output_buffer()
    ser.reset_input_buffer()

    # print("writing CC") 
    ser.write(struct.pack('B', CMD_CAPTURE)) # becomes 0xCC somehow
    time.sleep(.3)
    print("starting to read")
    now = time.time()
    # rawlist = []
    # while ser.in_waiting > 0 and len(rawlist) < 72:
    #     rawlist.append(ser.read(352*2*2*2))
    #     print('', len(rawlist))
    # raw = b''.join(rawlist)
    raw = ser.read(352*288*2)
    print(len(raw), raw[:10])
    print("read time", time.time() - now)
    
    # print("post read")
    width, height = (352, 288)
    print("Width, height", width, height)
    img = Image.new('RGB', (width, height))
    data = img.load()

    print("for loop")
    for y in range(height):
        try:
            for x in range(width):
                idx = y * width + x
                v = struct.unpack('<H', raw[2*idx:2*(idx+1)])[0]
                # v = struct.unpack('<H', raw[2*idx:2*(idx+1)])[0]
                
                # bit order is scrambled: 2 1 3 0 4 _ 5 6
                # ok actually its 16 bits like "2211330044__5566"
                # 16 bit 565 RGB is composed like:      "RRRRRGGGGGGBBBBB"
                # 16 bit for us is then scrambled like: "RG RR GG RR GG __ GB BB"  
                # # Red bits: positions 15, 13, 12, 9, 8 (5 bits total)
                # r = ((v >> 15) & 1) << 4 | \
                #     ((v >> 13) & 1) << 3 | \
                #     ((v >> 12) & 1) << 2 | \
                #     ((v >> 9) & 1) << 1 | \
                #     ((v >> 8) & 1)
                
                # # Green bits: positions 14, 11, 10, 5, 4, 3 (6 bits total)  
                # g = ((v >> 14) & 1) << 5 | \
                #     ((v >> 11) & 1) << 4 | \
                #     ((v >> 10) & 1) << 3 | \
                #     ((v >> 5) & 1) << 2 | \
                #     ((v >> 4) & 1) << 1 | \
                #     ((v >> 3) & 1)
                
                # b = ((v >> 6) & 1) << 4 | \
                #     ((v >> 2) & 1) << 3 | \
                #     ((v >> 1) & 1) << 2 | \
                #     ((v >> 0) & 1) << 1 | \
                #     0  # This m

                bit0 = (v >> 8) & 1   # or (v >> 9) & 1, they should be the same
                bit1 = (v >> 12) & 1  # or (v >> 13) & 1
                bit2 = (v >> 14) & 1  # or (v >> 15) & 1  
                bit3 = (v >> 10) & 1  # or (v >> 11) & 1
                bit4 = (v >> 6) & 1   # or (v >> 7) & 1
                bit5 = (v >> 2) & 1   # or (v >> 3) & 1
                bit6 = (v >> 0) & 1   # or (v >> 1) & 1
                
                # Reconstruct the normal 565 value
                # Normal 565: RRRRRGGGGGGBBBBB (positions 15-11 R, 10-5 G, 4-0 B)
                unscrambled = (bit2 << 6) | (bit1 << 5) | (bit3 << 4) | (bit0 << 3) | (bit4 << 2) | (bit5 << 1) | bit6
                
                # Extract RGB from 7-bit unscrambled value
                # Wait, we only have 7 bits but need 16 for full 565...
                # Let me reconsider the mapping
                
                # If pattern is "2 1 3 0 4 _ 5 6" for 8 positions (doubled to 16)
                # We need to map these to a full 565 format
                # Assuming the 7 valid bits represent the most significant bits of each channel
                
                r = unscrambled >> 2  # Top 5 bits for red (bits 6-2)
                g = (unscrambled << 4) & 0b111111  # Middle bits for green  
                b = (unscrambled << 2) & 0b11111   # Bottom bits for blue
        



                # r, g, b = v >> (5 + 6), (v >> 5) & 0b111111, v & 0b11111 

                r = math.floor(r / 0x1f * 0xff)
                g = math.floor(g / 0x3f * 0xff)
                b = math.floor(b / 0x1f * 0xff)
                data[x, y] = (r, g, b)
        except Exception as e:
            print("error at y", y, e)
            for x in range(width):
                data[x, y] = (0, 0, 0)
            break

    img.save(filename)

# =======
# Helpers
# =======
def reg_write_list(l):
    for reg, value in l:
        reg_write(reg, value)

def reg_set_bit(reg, bit):
    value = reg_read(reg)
    value |= 1 << bit
    reg_write(reg, value)

def reg_clear_bit(reg, bit):
    value = reg_read(reg)
    value &= 0xFF ^ (1 << bit)
    reg_write(reg, value)

def reg_get_bit(reg, bit):
    value = reg_read(reg)
    return (value >> bit) & 1

# ====================
# Camera configuration
# ====================
def set_hflip(hflip):
    reg_write(0xff, 0x01)
    if hflip:
        reg_set_bit(0x04, 7)
    else:
        reg_clear_bit(0x04, 7)

def set_vflip(vflip):
    reg_write(0xff, 0x01)
    if vflip:
        reg_set_bit(0x04, 6)
    else:
        reg_clear_bit(0x04, 6)

IPython.embed()
# if __name__ == '__main__':
#     print("capturing test.jpg")
#     # capture('test.jpg')
    # print(reg_read(0x1C))
#     print("post")
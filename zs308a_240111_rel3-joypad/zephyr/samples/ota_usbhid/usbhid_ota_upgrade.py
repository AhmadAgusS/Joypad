import usb.core
import usb.util
import time
import struct
import os

def get_file_size(file_path):
    return os.path.getsize(file_path)

# Send the trigger command (0x55, 0x55 followed by zeros)
def send_upgrade_trigger(dev):
    trigger_command = [0x55, 0x55] + [0x00] * 62
    dev.write(0x01, trigger_command)  # 0x01 is out endpoint address

def read_cmd_packet(dev):
    endpoint = 0x81
    # Define the command header and payload specifications
    cmd_header = dev.read(endpoint, 64, 5000)  # Read 64 bytes of command packet    
    
    svc_id = cmd_header[0]
    cmd = cmd_header[1]
    param_type = cmd_header[2]
    param_len = struct.unpack('<H', cmd_header[3:5])[0]

    # Check if the command packet meets specifications
    if svc_id != 0x09 or cmd != 0x03 or param_type != 128 or param_len != 14:
        raise ValueError("Invalid packet header")

    # Parse the offset value
    offset_type = cmd_header[5]
    if offset_type != 0x01:
        raise ValueError("Unexpected offset type")
    offset_len = struct.unpack('<H', cmd_header[6:8])[0]
    offset_value = struct.unpack('<I', cmd_header[8:8+offset_len])[0]

    # Parse the length value
    length_type = cmd_header[12]
    if length_type != 0x02:
        raise ValueError("Unexpected length type")
    length_len = struct.unpack('<H', cmd_header[13:15])[0]
    length_value = struct.unpack('<I', cmd_header[15:15+length_len])[0]

    return offset_value, length_value

# Send chunks of file "OTA.bin"
def send_file_chunk(dev, file_path, offset, length):
    endpoint_out = 0x01
    with open(file_path, "rb") as file:
        # Move to the correct file offset
        file.seek(offset)
        
        # Send the file in chunks of up to 63 bytes, with 0x41 as the first byte of each 64-byte packet
        while length > 0:
            chunk_size = min(length, 63)  # Leave space for the first byte
            data_chunk = file.read(chunk_size)
            
            # Create packet with 0x41 as the first byte, followed by the data
            packet = bytes([0x41]) + data_chunk
            
            # Pad the packet to 64 bytes if it's the last one and is smaller than 64 bytes
            if len(packet) < 64:
                packet += bytes(64 - len(packet))  # Padding with 0x00

            # Send packet to USB HID endpoint
            dev.write(endpoint_out, packet)
            
            # Update remaining length
            length -= chunk_size

def read_result_packet(dev):
    endpoint = 0x81
    # Define the command header and payload specifications
    cmd_header = dev.read(endpoint, 64, 5000)  # Read 64 bytes of command packet    
    
    svc_id = cmd_header[0]
    cmd = cmd_header[1]
    param_type = cmd_header[2]
    param_len = struct.unpack('<H', cmd_header[3:5])[0]

    # Check if the command packet meets specifications
    if svc_id != 0x09 or cmd != 0x06 or param_type != 128 or param_len != 4:
        raise ValueError("Invalid packet header")

    # Parse the offset value
    result_type = cmd_header[5]
    if result_type != 0x01:
        raise ValueError("Unexpected offset type")
    result_len = struct.unpack('<H', cmd_header[6:8])[0]
    result_value = cmd_header[8]

    return result_value

def main():
    # Initialize HID device (please modify with your vendor ID and product ID
    dev = usb.core.find(idVendor=0x10d6, idProduct=0xb012)
    # print(dev) # Double check IN/OUT endpoint address of this device

    ota_file_path = 'ota.bin'
    file_size = get_file_size(ota_file_path)
    print(f"File: {ota_file_path}\nSize: {file_size} Bytes")

    if dev is None:
        raise ValueError('Device not found')
  
    # Set configuration
    dev.set_configuration()

    # Send upgrade trigger
    send_upgrade_trigger(dev)

    while(1):
        offset, length = read_cmd_packet(dev)
        if offset is None or length is None:
            print("Did not receive cmd packet")
            break
        
        print(f"Offset = {offset}, Length = {length}")                
        send_file_chunk(dev, ota_file_path, offset, length)
        
        if (offset + length >= file_size):            
            break

    upgrade_result = read_result_packet(dev)
    if (upgrade_result == 0x01):
        print("Upgrade successfully")
    else:
        print("Upgrade failed")

if __name__ == "__main__":
    main()
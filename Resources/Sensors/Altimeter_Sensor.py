import time
import smbus2

class Lps25hsensor:
    address = 0x5d

    PRESS_OUT_XL = 0x28
    PRESS_OUT_L = 0x29
    PRESS_OUT_H = 0x2A

    TEMP_OUT_L = 0x2B
    TEMP_OUT_H = 0x2C

    def __init__(self, smbus_addr=1):
        self.bus = smbus2.SMBus(smbus_addr)

    def setup(self):
        try:
            # self.bus.write_i2c_block_data(self.address, 0x00, [])
            self.bus.write_i2c_block_data(self.address, 0x21, [0x04])
            self.bus.write_i2c_block_data(self.address, 0x20, [0x00])
            self.bus.write_i2c_block_data(self.address, 0x21, [0x40])
            self.bus.write_i2c_block_data(self.address, 0x10, [0x0c])
        except OSError as e:
            print(e)

    def read_i2c_block(self, register):
        blocks = self.bus.read_i2c_block_data(self.address, register, 1)
        return blocks[0]

    def read_pressure(self):
        try:
            self.bus.write_i2c_block_data(self.address, 0x20, [0xc4])
        except OSError as e:
            print(e)

        time.sleep(0.1)

        XL = self.read_i2c_block(self.PRESS_OUT_XL)
        L = self.read_i2c_block(self.PRESS_OUT_L)
        H = self.read_i2c_block(self.PRESS_OUT_H)

        pressure = (
                (H << 16) | (L << 8) | XL
        )
        pressure = pressure / 4096
        return pressure

    def read_temp(self):
        try:
            self.bus.write_i2c_block_data(self.address, 0x20, [0x90])
        except OSError as e:
            print(e)

        time.sleep(0.1)

        TL = self.read_i2c_block(self.TEMP_OUT_L)
        TH = self.read_i2c_block(self.TEMP_OUT_H)

        temp = (
                (TH << 8) | TL
        )
        temp = self.binary_to_twos_complement(temp)
        temp = 42.5 + (temp / 480)
        return temp

    def binary_to_twos_complement(self, binary_value):
        binary_str = str(bin(binary_value))[2:]

        if binary_str[0] == '0':
            return int(binary_str, 2)

        else:
            inverted = ''
            for bit in binary_str:
                if bit == '0':
                    inverted += "1"
                else:
                    inverted += "0"
            val = int(inverted, 2)
            val += 1
            val *= -1
            return val

def Mpa_to_Altimeter(hPa):
    P = hPa / 10000
    P0 = 0.101325
    altimeter = 44330 * (1 - ((P / P0) ** (1000 / 5255)))

    return altimeter

def get_data():
    sensor = Lps25hsensor(1)
    sensor.setup()

    hPa = sensor.read_pressure()
    alt = Mpa_to_Altimeter(hPa)
    temp = sensor.read_temp()

    return alt, temp

def write_average_alt_per_second():  # About 200ms per Get_Pressure Function.
    total_alt = 0
    total_temp = 0

    for j in range(5):  # 200ms per Get_Pressure function, so 1sec = 5times
        alt, temp = get_data()
        total_alt += round(alt, 3)
        total_temp += round(temp, 3)

    total_alt /= 5
    total_alt = round(total_alt, 3)

    total_temp /= 5
    total_temp = round(total_temp, 3)

    return total_alt, total_temp


if __name__ == '__main__':
    total_alt, total_temp = write_average_alt_per_second()

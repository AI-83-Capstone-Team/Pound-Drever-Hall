import time
from pdh_api import *

if __name__ == "__main__":
    api_reset_fpga()
    time.sleep(0.01)
    ##while True:

    #api_set_dac(0.0, 0)
    api_set_dac(0.0,1)
    api_get_adc()

    api_set_rotation(0)
    api_set_pid(kp=0.5, kd=0.2, ki=0.2, sp=-0.807, dec=50, alpha = 2, sat=18, en=1)
    api_get_frame(50, 1)
    api_get_adc()


    api_check_signed(6)
    api_get_frame(5, 1)

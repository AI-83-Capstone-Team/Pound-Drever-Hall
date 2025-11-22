import pickle
import numpy as np

from linien_client.connection import LinienClient
from linien_client.device import Device
#from linien_common.common import FAST_AUTOLOCK

from matplotlib import pyplot as plt
from time import sleep




DEVICE = Device(
    host = "10.42.0.62", #Access your computer's ARP table to find the IP mapping to MAC address 00:26:32:f0:bb:74 - do note this MAC will change if a different Pitaya physical instance is being used
    username = "root",
    password = "root"
)


c = LinienClient(
        device = DEVICE
    )


c.connect(autostart_server=True, use_parameter_cache=True)

#c.parameters.autolock_mode_preference.value = FAST_AUTOLOCK


def wait_for_lock_status(should_be_locked):
    """A helper function that waits until the laser is locked or unlocked."""
    counter = 0
    while True:
        to_plot = pickle.loads(c.parameters.to_plot.value)
        is_locked = "error_signal" in to_plot

        if is_locked == should_be_locked:
            break

        counter += 1
        if counter > 10:
            raise Exception("waited too long")

        sleep(1)


# turn of the lock (if it is running)
c.connection.root.start_sweep()
# wait until the laser is unlocked (if required)
wait_for_lock_status(False)


# we record a reference spectrum
to_plot = pickle.loads(c.parameters.to_plot.value)
error_signal = to_plot["error_signal_1"]


# we plot the reference spectrum and ask the user where the target line is
plt.plot(error_signal)
plt.plot(to_plot["monitor_signal"])
plt.show()

print("Please specify the position of the target line. ")
x0 = int(input("enter index of a point that is on the left side of the target line: "))
x1 = int(input("enter index of a point that is on the right side of the target line: "))


# show the lock point again
plt.axvline(x0, color="r")
plt.axvline(x1, color="r")
plt.plot(error_signal)
plt.show()


# turn on the lock
c.connection.root.start_autolock(x0, x1, pickle.dumps(error_signal))


# wait until the laser is actually locked
try:
    wait_for_lock_status(True)
    print("locking the laser worked \o/")
except Exception:
    print("locking the laser failed :(")
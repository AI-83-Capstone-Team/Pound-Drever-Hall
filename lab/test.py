
import time
from modules import LinienHandler


handler = LinienHandler()
handler.connect()
params = handler.get_parameters()

print(params['target_slope_rising'])

params['target_slope_rising'] = False
handler.set_parameters(params)
params = handler.get_parameters()
print(params['target_slope_rising'])

params['target_slope_rising'] = True
handler.set_parameters(params)
params = handler.get_parameters()
print(params['target_slope_rising'])


#for key, val in params.items():
#    print(f"KEY: {key}, VAL: {str(val)}\n")

handler.serialize_state("logs/", "state1")

params['autolock_running'] = True
handler.set_parameters(params)

time.sleep(5)
handler.serialize_state("logs/", "state2")


handler.sweep()
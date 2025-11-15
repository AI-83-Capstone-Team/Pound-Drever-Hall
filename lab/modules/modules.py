import json
from pathlib import Path
from linien.client.connection import LinienClient
from linien.client.remote_parameters import RemoteParameter, RemoteParameters

__all__ = [
    'LinienHandler'
]

DEVICE = {
    "host" : "10.42.0.62", #Access your client's ARP table to find the IP mapping to MAC address 00:26:32:f0:bb:74 - do note this MAC will change if a different Pitaya is being used
    "username" : "root",
    "password" : "root"
}

class LinienHandler:
    def __init__(self, port=18862):
        self.port = port
        self.client = None
        self.state = {}

    def connect(self, device=DEVICE)->None:
        self.client = LinienClient(
            device = device,
            use_parameter_cache = False
        )

    #Get parameter KV pairs
    def get_parameters(self, to_str = False)->dict:
        params: RemoteParameters = self.client.parameters
        all_parameters = vars(params)
        out = {}
        for key, param in all_parameters.items():
            if isinstance(param, RemoteParameter):
                out[key] = param.value
            else:
                out[key] = param
            if to_str:
                out[key] = str(out[key])
        self.state = out
        return out


    #Update parameters WRT config dict which is subset of full valid parameter set
    def set_parameters(self, new_params: dict)->None:
        curr_params: RemoteParameters = self.client.parameters
        curr_params._attributes_locked = False
        for key, value in new_params.items():
            setattr(curr_params, key, value)
        curr_params._attributes_locked = True


    #Serialize parameter state into JSON file
    def serialize_state(self, dest: str, name: str) -> None:
        state = self.get_parameters(to_str=True)
        path = Path(dest)
        if path.is_dir():
            path = path / f"{name}.json"

        with path.open("w") as f:
            json.dump(state, f, indent=2)

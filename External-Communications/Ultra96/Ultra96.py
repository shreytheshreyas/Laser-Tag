import multiprocessing as mp
import os
import sys

from EvalClient import EvalClient
from GameEngine import GameEngine

from GameEngine_Stub import GameEngine_Stub
from HWAccel import HWAccel
from MQTTClient import MQTTClient

from RelayServer import RelayServer


class Ultra96:
    def __init__(self, relay_host, relay_port, eval_host, eval_port):
        self.relay_host = relay_host
        self.relay_port = relay_port

        self.eval_host = eval_host
        self.eval_port = eval_port

        self.opp_in_frames = mp.Array("i", [0] * 2)

        self.action_queue = mp.Queue()
        self.update_beetle_queue = mp.Queue()
        self.eval_req_queue = mp.Queue()
        self.eval_resp_queue = mp.Queue()
        self.vis_queue = mp.Queue()

        self.engine = GameEngine(
            self.opp_in_frames,
            self.action_queue,
            self.update_beetle_queue,
            self.eval_req_queue,
            self.eval_resp_queue,
            self.vis_queue,
        )

        self.eval_client = EvalClient(
            self.eval_host, self.eval_port, self.eval_req_queue, self.eval_resp_queue
        )
        self.relay_server = RelayServer(
            self.relay_host,
            self.relay_port,
            self.action_queue,
            self.update_beetle_queue,
        )
        self.mqtt_client = MQTTClient(self.vis_queue, self.opp_in_frames)

    def connect_to_eval(self):
        self.eval_client.connect()

    def start_game(self):
        engine_process = mp.Process(target=self.engine.run)
        relay_server_process = mp.Process(target=self.relay_server.run)
        mqtt_process = mp.Process(target=self.mqtt_client.run)
        eval_process = mp.Process(target=self.eval_client.run)

        try:
            relay_server_process.start()
            engine_process.start()
            mqtt_process.start()

            _ = input("")
            self.connect_to_eval()
            eval_process.start()

            eval_process.join()
            engine_process.join()
            mqtt_process.join()
            relay_server_process.join()

        except KeyboardInterrupt:
            eval_process.terminate()
            engine_process.terminate()
            mqtt_process.terminate()
            relay_server_process.terminate()


if __name__ == "__main__":
    relay_port = 8080
    eval_port = 2105

    if len(sys.argv) == 1:
        print(
            "Using default arguments of relay server port 8080 and eval client port of 2105"
        )
    elif len(sys.argv) == 3:
        relay_port = int(sys.argv[1])
        eval_port = int(sys.argv[2])
    else:
        print("Invalid number of arguments")
        print(
            "python "
            + os.path.basename(__file__)
            + " [<Relay Server Port> <Eval Client Port>] [stub]"
        )
        sys.exit()

    ultra96 = Ultra96("127.0.0.1", relay_port, "127.0.0.1", eval_port)
    ultra96.start_game()

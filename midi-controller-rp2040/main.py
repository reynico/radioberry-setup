import time

import adafruit_matrixkeypad
import adafruit_midi
import board
import digitalio
import rotaryio
import usb_midi
from adafruit_midi.control_change import ControlChange
from adafruit_midi.note_off import NoteOff
from adafruit_midi.note_on import NoteOn


class MIDIController:
    def __init__(self, channel=3):
        self.channel = channel
        self.midi = adafruit_midi.MIDI(midi_out=usb_midi.ports[1], out_channel=channel)
        self.led = self._setup_led()
        self.keypad = self._setup_keypad()
        self.encoders = self._setup_encoders()

        print(f"Simple piHpsdr MIDI controller channel {channel}")

    def _setup_led(self):
        led = digitalio.DigitalInOut(board.LED)
        led.direction = digitalio.Direction.OUTPUT
        return led

    def _setup_keypad(self):
        rows = [
            digitalio.DigitalInOut(x)
            for x in (board.GP2, board.GP3, board.GP5, board.GP7)
        ]
        cols = [
            digitalio.DigitalInOut(x)
            for x in (board.GP0, board.GP1, board.GP4, board.GP6)
        ]
        keys = (
            (1, 2, 3, 4),
            (21, 22, 23, 24),
            (31, 32, 33, 34),
            (41, 42, 43, 44),
        )
        return adafruit_matrixkeypad.Matrix_Keypad(rows, cols, keys)

    def _setup_encoders(self):
        encoder_configs = [
            {"pins": (board.GP11, board.GP10), "cc": 11, "name": "VFO"},
            {"pins": (board.GP18, board.GP19), "cc": 12, "name": "Encoder2"},
            {"pins": (board.GP17, board.GP16), "cc": 13, "name": "Encoder3"},
            {"pins": (board.GP13, board.GP12), "cc": 14, "name": "Encoder4"},
            {"pins": (board.GP14, board.GP15), "cc": 15, "name": "Encoder5"},
        ]

        encoders = []
        for config in encoder_configs:
            encoder = {
                "encoder": rotaryio.IncrementalEncoder(
                    config["pins"][0], config["pins"][1]
                ),
                "last_position": 0,
                "cc_number": config["cc"],
                "reverse": -1,
                "name": config["name"],
            }
            encoders.append(encoder)

        return encoders

    def handle_keypad(self):
        keys = self.keypad.pressed_keys
        if keys:
            self.led.value = True
            key_value = keys[0]

            self.midi.send(NoteOn(key_value, 60))
            print(f"Pressed: {keys}")
            self.midi.send(NoteOff(key_value, 0))

            time.sleep(0.2)
            self.led.value = False

    def handle_encoder(self, encoder_data):
        current_position = encoder_data["encoder"].position
        last_position = encoder_data["last_position"]

        if current_position != last_position:
            print(
                f"{encoder_data['name']} pos: {current_position}, last: {last_position}"
            )

            if current_position < last_position:
                control_value = 64 + encoder_data["reverse"]
                self.midi.send(ControlChange(encoder_data["cc_number"], control_value))
                print(f"{encoder_data['name']} down")
            elif current_position > last_position:
                control_value = 64 - encoder_data["reverse"]
                self.midi.send(ControlChange(encoder_data["cc_number"], control_value))
                print(f"{encoder_data['name']} up")

            encoder_data["last_position"] = current_position

    def handle_all_encoders(self):
        for encoder_data in self.encoders:
            self.handle_encoder(encoder_data)

    def run(self):
        while True:
            self.handle_keypad()
            self.handle_all_encoders()
            time.sleep(0.02)


def main():
    controller = MIDIController(channel=3)
    controller.run()


if __name__ == "__main__":
    main()

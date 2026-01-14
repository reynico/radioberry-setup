import time
import analogio
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
        self.midi = adafruit_midi.MIDI(
            midi_out=usb_midi.ports[1], out_channel=channel)
        self.led = self._setup_led()
        self.buttons = self._setup_buttons()
        self.potentiometers = self._setup_potentiometers()
        self.encoder = self._setup_encoder()

        print(f"FT-80C MIDI controller channel {channel}")

    def _setup_led(self):
        led = digitalio.DigitalInOut(board.LED)
        led.direction = digitalio.Direction.OUTPUT
        return led

    def _setup_buttons(self):
        button_configs = [
            {"pin": board.GP0, "note": 1, "name": "narrow"},
            {"pin": board.GP1, "note": 2, "name": "att"},
            {"pin": board.GP2, "note": 3, "name": "noise_blanker"},
            {"pin": board.GP3, "note": 4, "name": "mode_down"},
            {"pin": board.GP4, "note": 5, "name": "mode_up"},
            {"pin": board.GP5, "note": 6, "name": "vfo_to_m"},
            {"pin": board.GP6, "note": 7, "name": "m_to_vfo"},
            {"pin": board.GP7, "note": 8, "name": "vfo"},
            {"pin": board.GP8, "note": 9, "name": "mr"},
            {"pin": board.GP9, "note": 10, "name": "split"},
            {"pin": board.GP10, "note": 11, "name": "pri_m"},
            {"pin": board.GP11, "note": 12, "name": "fast"},
            {"pin": board.GP12, "note": 13, "name": "band"},
            {"pin": board.GP13, "note": 14, "name": "clar"},
            {"pin": board.GP14, "note": 15, "name": "d_lock"},
        ]

        buttons = []
        for config in button_configs:
            button_pin = digitalio.DigitalInOut(config["pin"])
            button_pin.direction = digitalio.Direction.INPUT
            button_pin.pull = digitalio.Pull.UP

            button = {
                "pin": button_pin,
                "note": config["note"],
                "name": config["name"],
                "last_state": True,
            }
            buttons.append(button)

        return buttons

    def _setup_potentiometers(self):
        pot_configs = [
            {"pin": board.A0, "cc": 21, "name": "sql"},
            {"pin": board.A1, "cc": 22, "name": "mic"},
            {"pin": board.A2, "cc": 23, "name": "drive"},
        ]

        potentiometers = []
        for config in pot_configs:
            pot_pin = analogio.AnalogIn(config["pin"])

            potentiometer = {
                "pin": pot_pin,
                "cc_number": config["cc"],
                "name": config["name"],
                "last_value": 0,
                "threshold": 512,
            }
            potentiometers.append(potentiometer)

        return potentiometers

    def _setup_encoder(self):
        encoder = {
            "encoder": rotaryio.IncrementalEncoder(board.GP15, board.GP16),
            "last_position": 0,
            "cc_number": 11,
            "reverse": 1,
            "name": "frequency",
        }
        return encoder

    def handle_buttons(self):
        for button_data in self.buttons:
            current_state = button_data["pin"].value
            last_state = button_data["last_state"]

            if current_state != last_state:
                if not current_state:
                    self.led.value = True
                    self.midi.send(NoteOn(button_data["note"], 127))
                    print(f"{button_data['name']} pressed")
                else:
                    self.led.value = False
                    self.midi.send(NoteOff(button_data["note"], 0))
                    print(f"{button_data['name']} released")

                button_data["last_state"] = current_state

    def handle_potentiometers(self):
        for pot_data in self.potentiometers:
            current_value = pot_data["pin"].value
            last_value = pot_data["last_value"]

            if abs(current_value - last_value) > pot_data["threshold"]:
                midi_value = current_value >> 9
                self.midi.send(ControlChange(pot_data["cc_number"], midi_value))
                print(f"{pot_data['name']}: {midi_value}")
                pot_data["last_value"] = current_value

    def handle_encoder(self):
        current_position = self.encoder["encoder"].position
        last_position = self.encoder["last_position"]

        if current_position != last_position:
            if current_position < last_position:
                control_value = 64 + self.encoder["reverse"]
                self.midi.send(ControlChange(
                    self.encoder["cc_number"], control_value))
                print(f"{self.encoder['name']} down")
            elif current_position > last_position:
                control_value = 64 - self.encoder["reverse"]
                self.midi.send(ControlChange(
                    self.encoder["cc_number"], control_value))
                print(f"{self.encoder['name']} up")

            self.encoder["last_position"] = current_position

    def run(self):
        while True:
            self.handle_buttons()
            self.handle_potentiometers()
            self.handle_encoder()
            time.sleep(0.01)


def main():
    controller = MIDIController(channel=3)
    controller.run()


if __name__ == "__main__":
    main()

import sys
from PyQt5 import QtWidgets, QtCore
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import numpy as np
from bleak import BleakClient, BleakScanner
import asyncio
import qasync
from qasync import asyncSlot
from datetime import datetime
import csv

CURRENT_TIME_SERVICE_UUID = "1805"
CURRENT_TIME_CHAR_UUID = "2A2B"

TEMPERATURE_SERVICE_UUID = "CBB6067B-1918-44F3-89E4-3043A2D12E27"
TEMPERATURE_STREAM_CHAR_UUID = "07E5D6F7-6F18-4DCE-BB5B-732CAB4E8474"

HUMIDITY_SERVICE_UUID = "86AF1E06-D1A5-4A14-A2E9-B49313405EED"
HUMIDITY_STREAM_CHAR_UUID = "97DCE133-0916-4D5C-A1E3-217B10B37D58"

PRESSURE_SERVICE_UUID = "8DCF22A9-F7EF-48CD-ADFC-ACB21BF61B4B"
PRESSURE_STREAM_CHAR_UUID = "55FCD9B7-63B3-4820-A26C-73A8A7BB8D6F"

MICROPHONE_SERVICE_UUID = "FA124461-66EA-4E1B-B1E4-E58CB6DEC6BE"
MICROPHONE_STREAM_CHAR_UUID = "B0CE3C07-AA05-4C8C-8E89-6F62ECD7DAC2"

DEVICE_NAME = "ID-169"

class SensorGraphApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.data_log = []

        self.setWindowTitle("Wykresy wartości z czujników (Bluetooth)")
        self.setGeometry(100, 100, 800, 600)
        self.setStyleSheet("background-color: #FFEBF6;")

        self.central_widget = QtWidgets.QWidget()
        self.setCentralWidget(self.central_widget)
        self.layout = QtWidgets.QVBoxLayout(self.central_widget)

        self.connection_label = QtWidgets.QLabel("Oczekiwanie na połączenie z urządzeniem Bluetooth...")
        self.connection_label.setAlignment(QtCore.Qt.AlignCenter)
        self.connection_label.setStyleSheet("font-size: 20px; font-weight: bold;  background-color: #D9C4DA; padding: 5px; border-radius: 15px;")
        self.layout.addWidget(self.connection_label)

        self.show_graphs_button = QtWidgets.QPushButton("ROZPOCZNIJ POMIARY")
        self.show_graphs_button.clicked.connect(self.show_graphs)
        self.show_graphs_button.setVisible(False)  # Przycisk jest ukryty na początku
        self.show_graphs_button.setStyleSheet("font-size: 20px; font-weight: bold;")
        self.layout.addWidget(self.show_graphs_button)

        self.save_button = QtWidgets.QPushButton("Zapisz dane do CSV")
        self.save_button.clicked.connect(self.save_to_csv)
        self.save_button.setVisible(False)
        self.layout.addWidget(self.save_button)
        self.save_button.setStyleSheet("font-size: 20px; font-weight: bold;  background-color: #ADD8E6; padding: 5px; border-radius: 15px; border-width: 2px;")

        self.tabs = QtWidgets.QTabWidget()
        self.layout.addWidget(self.tabs)
        self.tabs.setVisible(False)

        self.temperature_graph = self.create_graph("Temperatura")
        self.humidity_graph = self.create_graph("Wilgotność")
        self.pressure_graph = self.create_graph("Ciśnienie")
        self.microphone_graph = self.create_graph("Mikrofon")

        self.tabs.addTab(self.temperature_graph, "Temperatura")
        self.tabs.addTab(self.humidity_graph, "Wilgotność")
        self.tabs.addTab(self.pressure_graph, "Ciśnienie")
        self.tabs.addTab(self.microphone_graph, "Mikrofon")

        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_graphs)

        self.ble_client = None
        self.ble_data = {
            "temperature": None,
            "humidity": None,
            "pressure": None,
            "microphone": None,
        }

        self.bt_timer = QtCore.QTimer()
        self.bt_timer.timeout.connect(self.start_bluetooth_connection)
        self.bt_timer.setSingleShot(True)
        self.bt_timer.start(100)

    def create_graph(self, title):
        widget = QtWidgets.QWidget()
        layout = QtWidgets.QVBoxLayout(widget)

        figure = Figure()
        canvas = FigureCanvas(figure)
        ax = figure.add_subplot(111)
        ax.set_title(title)
        ax.set_xlabel("Czas [s]")
        ax.set_ylabel("Wartość")

        # Inicjalizacja danych
        self.x_data = np.arange(0, 10, 0.1)
        self.y_data = np.zeros_like(self.x_data)
        self.line, = ax.plot(self.x_data, self.y_data)

        # Etykieta do wyświetlania czasu RTC
        self.rtc_label = QtWidgets.QLabel("Czas RTC: Nieznany")
        self.rtc_label.setAlignment(QtCore.Qt.AlignCenter)
        self.rtc_label.setStyleSheet("font-size: 16px;")
        layout.addWidget(self.rtc_label)

        layout.addWidget(canvas)
        widget.setLayout(layout)

        return widget

    def start_bluetooth_connection(self):
        asyncio.create_task(self.scan_and_connect())

    async def scan_and_connect(self):
        print("Skanowanie urządzeń Bluetooth...")
        devices = await BleakScanner.discover()
        print(f"Znaleziono {len(devices)} urządzeń.")

        for device in devices:
            print(f"Sprawdzanie urządzenia: {device.name} ({device.address})")
            if device.name == DEVICE_NAME:
                print(f"Znaleziono urządzenie: {device.name} ({device.address})")
                self.ble_client = BleakClient(device.address)
                try:
                    await self.ble_client.connect()
                    print("Połączono z urządzeniem Bluetooth.")
                    await self.ble_client.write_gatt_char(CURRENT_TIME_CHAR_UUID, self.get_current_time_as_bytearray(), response=False)
                    await self.ble_client.start_notify(TEMPERATURE_STREAM_CHAR_UUID, self.handle_temperature_data)
                    await self.ble_client.start_notify(HUMIDITY_STREAM_CHAR_UUID, self.handle_humidity_data)
                    await self.ble_client.start_notify(PRESSURE_STREAM_CHAR_UUID, self.handle_pressure_data)
                    await self.ble_client.start_notify(MICROPHONE_STREAM_CHAR_UUID, self.handle_microphone_data)

                    self.connection_label.setText("Połączono z urządzeniem Bluetooth.")
                    self.show_graphs_button.setVisible(True)  # Pokazujemy przycisk
                    break
                except Exception as e:
                    print(f"Błąd połączenia Bluetooth: {e}")
            else:
                print(f"Pominięto urządzenie: {device.name}")

    @asyncSlot()
    async def show_graphs(self):
        if self.ble_client and self.show_graphs_button:
            await self.read_rtc_time()
            self.tabs.setVisible(True)
            self.show_graphs_button.setVisible(False)
            self.save_button.setVisible(True)
            self.timer.start(1000)

    async def read_rtc_time(self):
        if self.ble_client and self.ble_client.is_connected:
            rtc_data = await self.ble_client.read_gatt_char(CURRENT_TIME_CHAR_UUID)
            self.parse_rtc_data(rtc_data)

    def parse_rtc_data(self, data):
        year = int.from_bytes(data[0:2], byteorder="little")
        month = data[2]
        day = data[3]
        hour = data[4]
        minute = data[5]
        second = data[6]
        day_of_week = data[7]
        milliseconds = data[8]
        adjust_reason = data[9]

        time_str = f"{year}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:{second:02d}.{milliseconds:03d}"

        for i in range(self.tabs.count()):
            widget = self.tabs.widget(i)
            rtc_label = widget.layout().itemAt(0).widget()
            rtc_label.setText(f"Czas RTC: {time_str}")

    def handle_temperature_data(self, sender, data):
        #print(f"Temepratura: odebrane dane{data}, długosc: {len(data)}")
        temperature = []
        if (len(data) % 4 == 0):
            try:
                for i in range(0, len(data), 4):
                    float_value = np.frombuffer(data[i:i+4], dtype = np.float32[0])
                    temperature.append(float_value)
            except Exception as e:
                int_values = int.from_bytes(data[i:i+4], byteorder="little", signed=False)
                temperature.append(int_values)
        else:
            print("nieprawidłowa dlugosc data z temperatury")
        self.ble_data["temperature"] = temperature

    def handle_humidity_data(self, sender, data):
        #print(f"Wilgotnosc: odebrane dane{data}, długosc: {len(data)}")
        humidity = []
        if (len(data) % 4 == 0):
            for i in range(0, len(data), 4):
                try:
                    float_value = np.frombuffer(data[i:i+4], dtype = np.float32[0])
                    humidity.append(float_value)
                except Exception as e:
                    int_value = int.from_bytes(data[i:i+4], byteorder="little", signed=False)
                    humidity.append(int_value)
        else:
            print("nieprawidłowa dlugosc data z wilgotnosci")
        self.ble_data["humidity"] = humidity

    def handle_pressure_data(self, sender, data):
        #print(f"Cisnienie: odebrane dane{data}, długosc: {len(data)}")
        pressure = []
        if (len(data) % 4 == 0):
            try:
                for i in range(0, len(data), 4):
                    float_value = np.frombuffer(data[i:i+4], dtype = np.float32[0])
                    pressure.append(float_value)
            except Exception as e:
                int_value = int.from_bytes(data[i:i+4], byteorder="little", signed=False)
                pressure.append(float(int_value))
        else:
            print("otrzymano nieprawidlowe dane")
        self.ble_data["pressure"] = pressure

    def handle_microphone_data(self, sender, data):
        #print(f"mikrofon: odebrane dane{data}, długosc: {len(data)}")
        microphone = []

        if(len(data) % 4 == 0):
            for i in range(0, len(data), 4):
                try:
                    float_value = np.frombuffer(data[i:i+4], dtype = np.float32[0])
                    microphone.append(float_value)
                except Exception as e:
                    int_value = int.from_bytes(data[i:i+4], byteorder="little", signed=False)
                    microphone.append(float(int_value))
        else:
            print("nieprawidłowa dlugosc data z mikrofonu")
        self.ble_data["microphone"] = microphone
    def update_graphs(self):
        if all(value is not None for value in self.ble_data.values()):
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            self.data_log.append([timestamp, [
                self.ble_data["temperature"],
                self.ble_data["humidity"],
                self.ble_data["pressure"],
                self.ble_data["microphone"]
            ]])

            self.update_graph(self.temperature_graph, self.ble_data["temperature"])
            self.update_graph(self.humidity_graph, self.ble_data["humidity"])
            self.update_graph(self.pressure_graph, self.ble_data["pressure"])
            self.update_graph(self.microphone_graph, self.ble_data["microphone"])

    def update_graph(self, graph_widget, values):
        canvas = graph_widget.layout().itemAt(1).widget()
        ax = canvas.figure.axes[0]
        line = ax.lines[0]
        y_data = line.get_ydata()

        if len(values) > len(y_data):
            values = values[-len(y_data):]

        y_data = np.roll(y_data, -len(values))
        y_data[-len(values):] = values

        line.set_ydata(y_data)
        ax.relim()
        ax.autoscale_view()
        canvas.draw()

    def get_current_time_as_bytearray(self):
        now = datetime.now()
        year = now.year
        year_bytes = year.to_bytes(2, byteorder="little")
        month = now.month
        day = now.day
        hour = now.hour
        minute = now.minute
        second = now.second

        day_of_week = now.weekday()
        day_of_week = (day_of_week + 1) % 7

        milliseconds = now.microsecond // 1000
        milliseconds = int(milliseconds/256)
        adjust_reason = 0

        time_data = bytearray([
            year_bytes[0], year_bytes[1],
            month,
            day,
            hour,
            minute,
            second,
            day_of_week,
            milliseconds,
            adjust_reason
        ])

        return time_data

    def save_to_csv(self):
        filename=f"sensor_data_{datetime.now().strftime("%Y_%m_%d_%H_%M_%S")}.csv"

        with open(filename, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["Temperature (°C)", "Humidity (%)", "Pressure", "Microphone"])

            for _, values in self.data_log:
                flattened_values = []
                for sublist in values:
                    if isinstance(sublist, list):
                        flattened_values.extend(sublist)
                    else:
                        flattened_values.append(sublist)
                writer.writerow(flattened_values)

        print(f"Dane zapisane do {filename}")

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    loop = qasync.QEventLoop(app)
    asyncio.set_event_loop(loop)

    window = SensorGraphApp()
    window.show()

    with loop:
        loop.run_forever()
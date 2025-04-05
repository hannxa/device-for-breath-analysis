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
import os
import scipy.signal as sig
from scipy import signal

CURRENT_TIME_SERVICE_UUID = "1805"
CURRENT_TIME_CHAR_UUID = "2A2B"

TEMPERATURE_SERVICE_UUID = "CBB6067B-1918-44F3-89E4-3043A2D12E27"
TEMPERATURE_STREAM_CHAR_UUID = "07E5D6F7-6F18-4DCE-BB5B-732CAB4E8474"

HUMIDITY_SERVICE_UUID = "86AF1E06-D1A5-4A14-A2E9-B49313405EED"
HUMIDITY_STREAM_CHAR_UUID = "97DCE133-0916-4D5C-A1E3-217B10B37D58"

PRESSURE_SERVICE_UUID = "8DCF22A9-F7EF-48CD-ADFC-ACB21BF61B4B"
PRESSURE_STREAM_CHAR_UUID = "55FCD9B7-63B3-4820-A26C-73A8A7BB8D6F"

DEVICE_NAME = "ID-169"


class SensorGraphApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.data_log = []
        self.x_slowdown_factor = 1
        self.filter_window_size = 8
        self.breath_threshold = 0.5  # Próg dla wykrywania wdechu (można dostosować)

        self.setWindowTitle("Wykresy wartości z czujników (Bluetooth)")
        self.setGeometry(100, 100, 800, 600)
        self.setStyleSheet("background-color: #FFEBF6;")

        self.central_widget = QtWidgets.QWidget()
        self.setCentralWidget(self.central_widget)
        self.layout = QtWidgets.QVBoxLayout(self.central_widget)

        self.connection_label = QtWidgets.QLabel("Oczekiwanie na połączenie z urządzeniem Bluetooth...")
        self.connection_label.setAlignment(QtCore.Qt.AlignCenter)
        self.connection_label.setStyleSheet(
            "font-size: 20px; font-weight: bold;  background-color: #D9C4DA; padding: 5px; border-radius: 15px;")
        self.layout.addWidget(self.connection_label)

        self.show_graphs_button = QtWidgets.QPushButton("ROZPOCZNIJ POMIARY")
        self.show_graphs_button.clicked.connect(self.show_graphs)
        self.show_graphs_button.setVisible(False)
        self.show_graphs_button.setStyleSheet("font-size: 20px; font-weight: bold;")
        self.layout.addWidget(self.show_graphs_button)

        self.save_button = QtWidgets.QPushButton("Zapisz dane do CSV")
        self.save_button.clicked.connect(self.save_to_csv)
        self.save_button.setVisible(False)
        self.layout.addWidget(self.save_button)
        self.save_button.setStyleSheet(
            "font-size: 20px; font-weight: bold;  background-color: #ADD8E6; padding: 5px; border-radius: 15px; border-width: 2px;")

        self.tabs = QtWidgets.QTabWidget()
        self.layout.addWidget(self.tabs)
        self.tabs.setVisible(False)

        self.temperature_graph = self.create_graph("Temperatura")
        self.humidity_graph = self.create_graph("Wilgotność")
        self.pressure_graph = self.create_pressure_graph("Ciśnienie - Wykrywanie wdechu")

        self.tabs.addTab(self.temperature_graph, "Temperatura")
        self.tabs.addTab(self.humidity_graph, "Wilgotność")
        self.tabs.addTab(self.pressure_graph, "Ciśnienie")

        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_graphs)

        self.ble_client = None
        self.ble_data = {
            "temperature": [],
            "humidity": [],
            "pressure": [],
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

        self.x_data = np.arange(0, 10, 0.1)
        self.y_data = np.zeros_like(self.x_data)
        self.line, = ax.plot(self.x_data, self.y_data)

        self.rtc_label = QtWidgets.QLabel("Czas RTC: Nieznany")
        self.rtc_label.setAlignment(QtCore.Qt.AlignCenter)
        self.rtc_label.setStyleSheet("font-size: 16px;")
        layout.addWidget(self.rtc_label)

        layout.addWidget(canvas)
        widget.setLayout(layout)

        return widget

    def create_pressure_graph(self, title):
        widget = QtWidgets.QWidget()
        layout = QtWidgets.QVBoxLayout(widget)

        figure = Figure()
        canvas = FigureCanvas(figure)
        ax = figure.add_subplot(111)
        ax.set_title(title)
        ax.set_xlabel("Czas [s]")
        ax.set_ylabel("Ciśnienie")

        self.x_pressure_data = np.arange(0, 10, 0.1)
        self.y_pressure_data = np.zeros_like(self.x_pressure_data)
        self.pressure_line, = ax.plot(self.x_pressure_data, self.y_pressure_data, 'b-')
        self.inhale_markers, = ax.plot([], [], 'ro', markersize=5)  # Czerwone kropki dla wdechu

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
                    await self.ble_client.write_gatt_char(CURRENT_TIME_CHAR_UUID, self.get_current_time_as_bytearray(),
                                                          response=False)
                    self.connection_label.setText("Połączono z urządzeniem Bluetooth.")
                    self.show_graphs_button.setVisible(True)
                    break
                except Exception as e:
                    print(f"Błąd połączenia Bluetooth: {e}")
            else:
                print(f"Pominięto urządzenie: {device.name}")

    @asyncSlot()
    async def show_graphs(self):
        if self.ble_client and self.show_graphs_button:
            await self.read_rtc_time()
            await self.start_notifications()
            self.tabs.setVisible(True)
            self.show_graphs_button.setVisible(False)
            self.save_button.setVisible(True)
            self.timer.start(100)

    async def start_notifications(self):
        if self.ble_client and self.ble_client.is_connected:
            await self.ble_client.start_notify(TEMPERATURE_STREAM_CHAR_UUID, self.handle_temperature_data)
            await self.ble_client.start_notify(HUMIDITY_STREAM_CHAR_UUID, self.handle_humidity_data)
            await self.ble_client.start_notify(PRESSURE_STREAM_CHAR_UUID, self.handle_pressure_data)

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
        temperature = []
        if len(data) % 4 == 0:
            try:
                for i in range(0, len(data), 4):
                    float_value = np.frombuffer(data[i:i + 4], dtype=np.float32)[0]
                    temperature.append(float_value)
            except Exception as e:
                int_values = int.from_bytes(data[i:i + 4], byteorder="little", signed=False)
                temperature.append(int_values)
        else:
            print("nieprawidłowa dlugosc data z temperatury")

        self.ble_data["temperature"].append(temperature)

    def handle_humidity_data(self, sender, data):
        humidity = []
        if len(data) % 4 == 0:
            for i in range(0, len(data), 4):
                try:
                    float_value = np.frombuffer(data[i:i + 4], dtype=np.float32)[0]
                    humidity.append(float_value)
                except Exception as e:
                    int_value = int.from_bytes(data[i:i + 4], byteorder="little", signed=False)
                    humidity.append(int_value)
        else:
            print("nieprawidłowa dlugosc data z wilgotnosci")

        self.ble_data["humidity"].append(humidity)

    def handle_pressure_data(self, sender, data):
        pressure = []
        if len(data) % 4 == 0:
            try:
                for i in range(0, len(data), 4):
                    float_value = np.frombuffer(data[i:i + 4], dtype=np.float32)[0]
                    pressure.append(float_value)
            except Exception as e:
                int_value = int.from_bytes(data[i:i + 4], byteorder="little", signed=False)
                pressure.append(float(int_value))
        else:
            print("otrzymano nieprawidlowe dane")

        self.ble_data["pressure"].append(pressure)

    def detect_inhale_points(self, pressure_data):
        """Wykrywa punkty gwałtownego wzrostu ciśnienia (wdechu)"""
        if len(pressure_data) < 2:
            return []

        # Oblicz różnice między kolejnymi próbkami
        diffs = np.diff(pressure_data)

        # Znajdź punkty gdzie różnica przekracza próg
        inhale_indices = np.where(diffs > self.breath_threshold)[0] + 1  # +1 bo diff zmniejsza długość o 1

        return inhale_indices

    def update_graphs(self):
        asyncio.create_task(self.read_rtc_time())

        if (self.ble_data["temperature"] and
                self.ble_data["humidity"] and
                self.ble_data["pressure"]):

            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            self.data_log.append({
                "timestamp": timestamp,
                "temperature": self.ble_data["temperature"][-1] if self.ble_data["temperature"] else [],
                "humidity": self.ble_data["humidity"][-1] if self.ble_data["humidity"] else [],
                "pressure": self.ble_data["pressure"][-1] if self.ble_data["pressure"] else []
            })

            # Temperatura i wilgotność
            temperatures = [item for entry in self.data_log for item in
                            (entry["temperature"] if entry["temperature"] else [])]
            humidities = [item for entry in self.data_log for item in (entry["humidity"] if entry["humidity"] else [])]
            pressures = [item for entry in self.data_log for item in (entry["pressure"] if entry["pressure"] else [])]

            # Ogranicz do ostatnich 100 punktów
            max_points = 100
            x_data = np.arange(0, len(temperatures)) * 0.1 / self.x_slowdown_factor
            if len(temperatures) > max_points:
                temperatures = temperatures[-max_points:]
                x_data = x_data[-max_points:]

            filtered_temperature = self.filter_data(temperatures)
            self.update_graph(self.temperature_graph, x_data, filtered_temperature)

            x_data = np.arange(0, len(humidities)) * 0.1 / self.x_slowdown_factor
            if len(humidities) > max_points:
                humidities = humidities[-max_points:]
                x_data = x_data[-max_points:]

            filtered_humidity = self.filter_data(humidities)
            self.update_graph(self.humidity_graph, x_data, filtered_humidity)

            # Aktualizacja wykresu ciśnienia z wykrywaniem wdechu
            x_data = np.arange(0, len(pressures)) * 0.1 / self.x_slowdown_factor
            if len(pressures) > max_points:
                pressures = pressures[-max_points:]
                x_data = x_data[-max_points:]

            filtered_pressure = self.filter_data(pressures)
            inhale_indices = self.detect_inhale_points(filtered_pressure)
            self.update_pressure_graph(x_data, filtered_pressure, inhale_indices)

        self.timer.start(200)

    def update_graph(self, graph_widget, x_data, y_data):
        if not len(y_data):
            return

        canvas = graph_widget.layout().itemAt(1).widget()
        ax = canvas.figure.axes[0]
        line = ax.lines[0]

        line.set_data(x_data, y_data)

        ax.relim()
        ax.autoscale_view()
        canvas.draw()

    def update_pressure_graph(self, x_data, y_data, inhale_indices):
        """Aktualizacja wykresu ciśnienia z zaznaczeniem wdechów"""
        if not len(y_data):
            return

        canvas = self.pressure_graph.layout().itemAt(1).widget()
        ax = canvas.figure.axes[0]

        # Aktualizuj linię ciśnienia
        self.pressure_line.set_data(x_data, y_data)

        # Zaznacz punkty wdechu czerwonymi kropkami
        if len(inhale_indices) > 0:
            inhale_x = x_data[inhale_indices]
            inhale_y = y_data[inhale_indices]
            self.inhale_markers.set_data(inhale_x, inhale_y)
        else:
            self.inhale_markers.set_data([], [])

        ax.relim()
        ax.autoscale_view()
        canvas.draw()

    def filter_data(self, y_data):
        if len(y_data) < self.filter_window_size:
            return y_data

        window = np.ones(self.filter_window_size) / self.filter_window_size
        filtered = np.convolve(y_data, window, 'same')

        half_window = self.filter_window_size // 2
        filtered[:half_window] = y_data[:half_window]
        filtered[-half_window:] = y_data[-half_window:]

        return filtered

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
        milliseconds = int(milliseconds / 256)
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
        save_dir = "C:\\Users\\akabe\\Desktop\\esp_data"
        os.makedirs(save_dir, exist_ok=True)

        filename = f"dane_sensorowe_{datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}.csv"
        full_path = os.path.join(save_dir, filename)

        with open(full_path, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["Temperature (°C)", "Humidity (%)", "Pressure", "Inhale"])

            for entry in self.data_log:
                temperatures = entry["temperature"] or []
                humidities = entry["humidity"] or []
                pressures = entry["pressure"] or []

                # Dla każdej próbki ciśnienia określ czy to wdech
                filtered_pressure = self.filter_data(pressures) if pressures else []
                inhale_indices = self.detect_inhale_points(filtered_pressure) if filtered_pressure else []
                inhale_flags = [0] * len(pressures)
                for idx in inhale_indices:
                    if idx < len(inhale_flags):
                        inhale_flags[idx] = 1

                max_length = max(len(temperatures), len(humidities), len(pressures))
                for i in range(max_length):
                    row = [
                        temperatures[i] if i < len(temperatures) else "",
                        humidities[i] if i < len(humidities) else "",
                        pressures[i] if i < len(pressures) else "",
                        inhale_flags[i] if i < len(inhale_flags) else ""
                    ]
                    writer.writerow(row)

        print(f"Dane zapisane do {filename}")

    def closeEvent(self, event):
        if self.ble_client and self.ble_client.is_connected:
            asyncio.create_task(self.ble_client.disconnect())
        event.accept()


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    loop = qasync.QEventLoop(app)
    asyncio.set_event_loop(loop)

    window = SensorGraphApp()
    window.show()

    with loop:
        loop.run_forever()
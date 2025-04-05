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
        self.inhale_markers = {}

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
        self.show_graphs_button.setVisible(False)  # Przycisk jest ukryty na początku
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
        self.pressure_graph = self.create_graph("Ciśnienie")

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

        # Inicjalizacja danych
        self.x_data = np.arange(0, 10, 0.1)
        self.y_data = np.zeros_like(self.x_data)
        self.line, = ax.plot(self.x_data, self.y_data)  # Change to line plot with markers

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
                    await self.ble_client.write_gatt_char(CURRENT_TIME_CHAR_UUID, self.get_current_time_as_bytearray(),
                                                          response=False)
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
        print("handle_temperature_data called")
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

        self.ble_data["temperature"].append(temperature)  # Append the new data as a new entry
        print(f"Temperature data: {temperature}")

    def handle_humidity_data(self, sender, data):
        print("handle_humidity_data called")
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

        self.ble_data["humidity"].append(humidity)  # Append the new data as a new entry
        print(f"Humidity data: {humidity}")

    def handle_pressure_data(self, sender, data):
        print("handle_pressure_data called")
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

        self.ble_data["pressure"].append(pressure)  # Append the new data as a new entry
        print(f"Pressure data: {pressure}")

    graph_index = 0

    def inhale_humidity(self, data):
        inhale_points = []
        if len(data) <11:
            return inhale_points

        for i in range(10, len(data) - 1):
            change_percent = ((data[i] - data[i - 10]) / data[i - 10]) * 100

            if change_percent > 0.010 and data[i] > data[i - 10]:
                inhale_points.append(i)

        return inhale_points

    def inhale_pressure(self, data):
        inhale_points = []
        if len(data) <11:
            return inhale_points

        for i in range(10, len(data) -1):
            change_percent = ((data[i] - data[i - 10]) / data[i - 10]) * 100

            if change_percent > 0.0015 and data[i] > data[i - 10]:
                inhale_points.append(i)

        return inhale_points

    def inhale_temperature(self, data):
        inhale_points = []
        if len(data) <11:
            return inhale_points

        for i in range(10, len(data) -1):
            change_percent = ((data[i] - data[i - 10]) / data[i - 10]) * 100

            if change_percent > 0.1 and data[i] > data[i-10]:
                    inhale_points.append(i)

        return inhale_points

    def update_graphs(self):
        print("update_graphs called")
        asyncio.create_task(self.read_rtc_time())
        max_points = 300

        if (self.ble_data["temperature"] and
                self.ble_data["humidity"] and
                self.ble_data["pressure"]):

            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            self.data_log.append({
                "timestamp": timestamp,
                "temperature": self.ble_data["temperature"][-1],
                "humidity": self.ble_data["humidity"][-1],
                "pressure": self.ble_data["pressure"][-1]
            })

            # --- TEMPERATURA ---
            temperatures = [item for entry in self.data_log for item in
                            (entry["temperature"] if entry["temperature"] else [])]
            temp_inhale_points = self.inhale_temperature(temperatures)

            if len(temperatures) > max_points:
                start_index = len(temperatures) - max_points
                temperatures = temperatures[-max_points:]
                temp_inhale_points = [i for i in temp_inhale_points if i >= start_index]
                temp_inhale_points = [i - start_index for i in temp_inhale_points]
            else:
                start_index = 0

            x_data_temp = np.arange(start_index, start_index + len(temperatures)) * 0.1 / self.x_slowdown_factor
            print(f"Temperature inhale points: {temp_inhale_points}")
            self.update_graph(self.temperature_graph, x_data_temp, temperatures, temp_inhale_points)

            # --- WILGOTNOŚĆ ---
            humidities = [item for entry in self.data_log for item in (entry["humidity"] if entry["humidity"] else [])]
            hum_inhale_points = self.inhale_humidity(humidities)

            if len(humidities) > max_points:
                start_index = len(humidities) - max_points
                humidities = humidities[-max_points:]
                hum_inhale_points = [i for i in hum_inhale_points if i >= start_index]
                hum_inhale_points = [i - start_index for i in hum_inhale_points]
            else:
                start_index = 0

            x_data_hum = np.arange(start_index, start_index + len(humidities)) * 0.1 / self.x_slowdown_factor
            print(f"Humidities inhale points: {hum_inhale_points}")
            self.update_graph(self.humidity_graph, x_data_hum, humidities, hum_inhale_points)

            # --- CIŚNIENIE ---
            pressures = [item for entry in self.data_log for item in (entry["pressure"] if entry["pressure"] else [])]
            press_inhale_points = self.inhale_pressure(pressures)

            if len(pressures) > max_points:
                start_index = len(pressures) - max_points
                pressures = pressures[-max_points:]
                press_inhale_points = [i for i in press_inhale_points if i >= start_index]
                press_inhale_points = [i - start_index for i in press_inhale_points]
            else:
                start_index = 0

            x_data_press = np.arange(start_index, start_index + len(pressures)) * 0.1 / self.x_slowdown_factor
            print(f"Pressure inhale points: {press_inhale_points}")
            self.update_graph(self.pressure_graph, x_data_press, pressures, press_inhale_points)

        self.timer.start(200)

    def update_graph(self, graph_widget, x_data, y_data, inhale_points):
        print(f"update_graph called for {graph_widget}")
        if not len(y_data):
            print("No data to update the graph.")
            return

        canvas = graph_widget.layout().itemAt(1).widget()
        ax = canvas.figure.axes[0]

        while len(ax.lines) > 1:
            ax.lines[-1].remove()

            # Usuń wszystkie kolekcje (scatter plot)
        while len(ax.collections) > 0:
            ax.collections[0].remove()

        line = ax.lines[0]
        line.set_data(x_data, y_data)

        if inhale_points and len(inhale_points) > 0:
            x_inhale = [x_data[i] for i in inhale_points if i < len(x_data)]
            y_inhale = [y_data[i] for i in inhale_points if i < len(y_data)]

            # Użyj scatter zamiast plot dla lepszej kontroli
            ax.scatter(x_inhale, y_inhale, color='red', marker='*', s=100, zorder=3)

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
            writer.writerow(["time_stamp", "Temperature (°C)", "Humidity (%)", "Pressure"])

            print(len(self.data_log))
            print("Data log:", self.data_log)

            for entry in self.data_log:
                time_only = datetime.strptime(entry["timestamp"], "%Y-%m-%d %H:%M:%S").strftime("%H:%M:%S")
                temperatures = entry["temperature"] or []
                humidities = entry["humidity"] or []
                pressures = entry["pressure"] or []

                max_length = max(len(temperatures), len(humidities), len(pressures))
                for i in range(max_length):
                    row = [
                        time_only,
                        temperatures[i] if i < len(temperatures) else "",
                        humidities[i] if i < len(humidities) else "",
                        pressures[i] if i < len(pressures) else ""
                    ]
                    writer.writerow(row)
                    print("Row:", row)

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

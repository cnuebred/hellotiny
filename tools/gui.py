import tkinter as tk
from tkinter import messagebox
import serial
import threading

class SerialMonitorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Prosty Serial Monitor")
        self.root.geometry("600x400")

        self.serial_port = None
        self.is_reading = False

        # --- PANEL STEROWANIA ---
        control_frame = tk.Frame(root)
        control_frame.pack(padx=10, pady=10, fill=tk.X)

        # Pole na nazwę portu (np. COM3 w Windows, /dev/ttyUSB0 w Linux)
        tk.Label(control_frame, text="Port:").pack(side=tk.LEFT)
        self.port_entry = tk.Entry(control_frame, width=15)
        self.port_entry.insert(0, "COM3")  # Domyślna wartość
        self.port_entry.pack(side=tk.LEFT, padx=5)

        # Pole na Baudrate
        tk.Label(control_frame, text="Baudrate:").pack(side=tk.LEFT)
        self.baud_entry = tk.Entry(control_frame, width=10)
        self.baud_entry.insert(0, "9600")
        self.baud_entry.pack(side=tk.LEFT, padx=5)

        # Przyciski Połącz/Rozłącz
        self.btn_connect = tk.Button(control_frame, text="Połącz", command=self.connect)
        self.btn_connect.pack(side=tk.LEFT, padx=5)

        self.btn_disconnect = tk.Button(control_frame, text="Rozłącz", command=self.disconnect, state=tk.DISABLED)
        self.btn_disconnect.pack(side=tk.LEFT, padx=5)

        # Przycisk do czyszczenia ekranu
        self.btn_clear = tk.Button(control_frame, text="Wyczyść", command=self.clear_text)
        self.btn_clear.pack(side=tk.RIGHT, padx=5)

        # --- OKNO Z DANYMI ---
        self.text_area = tk.Text(root, bg="black", fg="lime", font=("Consolas", 10))
        self.text_area.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)

        # Obsługa zamknięcia okna "iksem"
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)

    def connect(self):
        port = self.port_entry.get()
        baud = self.baud_entry.get()
        
        try:
            # Próba otwarcia portu szeregowego
            self.serial_port = serial.Serial(port, int(baud), timeout=1)
            self.is_reading = True
            
            # Zmiana stanu przycisków
            self.btn_connect.config(state=tk.DISABLED)
            self.btn_disconnect.config(state=tk.NORMAL)
            self.port_entry.config(state=tk.DISABLED)
            self.baud_entry.config(state=tk.DISABLED)
            
            self.log_message(f"--- Połączono z {port} przy prędkości {baud} bps ---\n")
            
            # Uruchomienie wątku czytającego dane w tle
            self.read_thread = threading.Thread(target=self.read_from_port, daemon=True)
            self.read_thread.start()
            
        except Exception as e:
            messagebox.showerror("Błąd połączenia", f"Nie można otworzyć portu {port}:\n{e}")

    def disconnect(self):
        self.is_reading = False
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            
        # Przywrócenie stanu przycisków
        self.btn_connect.config(state=tk.NORMAL)
        self.btn_disconnect.config(state=tk.DISABLED)
        self.port_entry.config(state=tk.NORMAL)
        self.baud_entry.config(state=tk.NORMAL)
        
        self.log_message("\n--- Rozłączono ---\n")

    def read_from_port(self):
        while self.is_reading and self.serial_port.is_open:
            try:
                if self.serial_port.in_waiting > 0:
                    # Odczyt danych i dekodowanie ignorujące błędy (aby aplikacja się nie wywaliła na śmieciach)
                    data = self.serial_port.read(self.serial_port.in_waiting).decode('utf-8', errors='ignore')
                    
                    # Tkinter nie lubi aktualizacji z innych wątków, 
                    # ale metoda insert w większości przypadków działa stabilnie na prostych tekstach.
                    self.text_area.insert(tk.END, data)
                    self.text_area.see(tk.END) # Automatyczne przewijanie w dół
            except Exception as e:
                if self.is_reading: # Zignoruj błąd jeśli sami zamknęliśmy port
                    self.log_message(f"\n[Błąd odczytu: {e}]\n")
                    self.disconnect()
                break

    def log_message(self, message):
        self.text_area.insert(tk.END, message)
        self.text_area.see(tk.END)

    def clear_text(self):
        self.text_area.delete('1.0', tk.END)

    def on_closing(self):
        self.is_reading = False
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = SerialMonitorApp(root)
    root.mainloop()
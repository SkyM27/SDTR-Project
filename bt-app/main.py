import flet as ft
import serial
import serial.tools.list_ports
import threading, queue, time

def main(page: ft.Page):
    page.title = "Bluetooth Alarm Control"
    page.window_width = 390
    page.window_height = 780
    page.bgcolor = "#000000EF"
    page.horizontal_alignment = ft.CrossAxisAlignment.CENTER

    ser = None
    rx_q = queue.Queue()
    stop_evt = threading.Event()
    reader_th = None

    def close_serial():
        nonlocal ser, reader_th
        stop_evt.set()
        try:
            if ser and ser.is_open:
                ser.close()
        except:
            pass
        ser = None
        reader_th = None

    def start_reader():
        nonlocal reader_th
        stop_evt.clear()
        def _reader():
            while not stop_evt.is_set():
                try:
                    if ser and ser.is_open:
                        line = ser.readline()
                        if line:
                            try:
                                rx_q.put_nowait(line.decode(errors="ignore").rstrip())
                            except:
                                pass
                    time.sleep(0.01)
                except:
                    break
        reader_th = threading.Thread(target=_reader, daemon=True)
        reader_th.start()

    #screen connect
    def show_connect_screen():
        page.clean()

        title = ft.Text("Welcome!", size=18, weight=ft.FontWeight.BOLD, color="#FFFFFF")
        list_box = ft.ListView(expand=0, spacing=10, padding=10, height=260, auto_scroll=False)
        status = ft.Text("", color="#7B1C1C")

        dd_baud = ft.Dropdown(
            label="baudrate", width=300, value="9600",
            options=[ft.dropdown.Option(v) for v in ["9600"]]
        )

        def refresh_ports(_=None):
            list_box.controls.clear()
            ports = serial.tools.list_ports.comports()
            if not ports:
                list_box.controls.append(ft.Text("No serial ports found", color="#AF1010"))
            else:
                for p in ports:
                    list_box.controls.append(
                        ft.ElevatedButton(
                            f"{p.device} - {p.description}",
                            bgcolor="#C40155",
                            color="#FFFFFF",
                            on_click=lambda e, pn=p.device: do_connect(pn)
                        )
                    )
            page.update()

        def do_connect(port_name: str):
            nonlocal ser
            status.value, status.color = "connecting...", "#FFFFFF"
            page.update()
            try:
                brate = int(dd_baud.value)
                ser = serial.Serial(port=port_name, baudrate=brate, timeout=0.1)
                start_reader()
                show_control_screen(port_name, brate)
            except Exception as ex:
                ser = None
                status.value = f"connect failed: {ex}"
                status.color = "red"
                page.update()

        page.add(
            ft.Column(
                expand=True,
                alignment=ft.MainAxisAlignment.START,
                horizontal_alignment=ft.CrossAxisAlignment.CENTER,
                controls=[
                    ft.Container(
                        width=360, padding=16, border_radius=20, bgcolor="#2B2B2B",
                        margin=ft.margin.only(top=120),
                        content=ft.Column(
                            spacing=14, horizontal_alignment=ft.CrossAxisAlignment.CENTER,
                            controls=[
                                ft.Container(  # logo in card
                                    content=ft.Image(src="assets/Logo.png", width=120, height=120, fit=ft.ImageFit.CONTAIN),
                                    margin=ft.margin.only(top=6, bottom=6),
                                ),
                                title,
                                ft.Text("Please select a device to connect to", color="#FFFFFF"),
                                list_box,
                                ft.Row([dd_baud], alignment=ft.MainAxisAlignment.CENTER),
                                ft.FilledButton("Search for devices", bgcolor="#C40155", color="#FFFFFF", on_click=refresh_ports),
                                status,
                            ],
                        ),
                    ),
                ],
            )
        )
        refresh_ports()

    #screen-control
    def show_control_screen(port_name: str, brate: int):
        page.clean()

        status = ft.Text(f"Connected to {port_name} @{brate}", color="#FFFFFF", weight=ft.FontWeight.BOLD)
        log = ft.ListView(expand=1, spacing=4, padding=8, height=260, auto_scroll=True)

        def send_bytes(b: bytes, label: str):
            if not ser or not ser.is_open:
                log.controls.append(ft.Text("[warn] not connected", color="#FF6B6B"))
            else:
                try:
                    ser.write(b)
                    log.controls.append(ft.Text(f">>> {label}", color="#FFFFFF"))
                except Exception as ex:
                    log.controls.append(ft.Text(f"[error] send failed: {ex}", color="#FF6B6B"))
            page.update()

        def do_arm(_):    send_bytes(b"A", "A")
        def do_disarm(_): send_bytes(b"D", "D")
        def do_disconnect(_):
            close_serial()
            show_connect_screen()

        arm_btn = ft.FilledButton("ARM System", bgcolor="#C40155", color="#FFFFFF", on_click=do_arm)
        disarm_btn = ft.FilledButton("DISARM System", bgcolor="#C40155", color="#FFFFFF", on_click=do_disarm)
        disconnect_btn = ft.OutlinedButton("Disconnect", on_click=do_disconnect)

        page.add(
            ft.Column(
                expand=True,
                alignment=ft.MainAxisAlignment.START,
                horizontal_alignment=ft.CrossAxisAlignment.CENTER,
                controls=[
                    ft.Container(
                        width=360, padding=16, border_radius=20, bgcolor="#2B2B2B",
                        margin=ft.margin.only(top=120),
                        content=ft.Column(
                            spacing=14, horizontal_alignment=ft.CrossAxisAlignment.CENTER,
                            controls=[
                                # ft.Container(
                                #     content=ft.Image(src="./assets/Logo.png", width=110, height=110, fit=ft.ImageFit.CONTAIN),
                                #     margin=ft.margin.only(top=6, bottom=6),
                                # ),
                                ft.Text("Alarm Control", size=18, weight=ft.FontWeight.BOLD, color="#FFFFFF"),
                                status,
                                arm_btn,
                                disarm_btn,
                                ft.Text("Serial log:", weight=ft.FontWeight.BOLD, color="#FFFFFF"),
                                log,
                                ft.Divider(),
                                disconnect_btn,   # aici apare din nou
                            ],
                        ),
                    ),
                ],
            )
        )

        # update serial log
        def pump_rx(_):
            try:
                while True:
                    line = rx_q.get_nowait()
                    log.controls.append(ft.Text(line, color="#FFFFFF"))
            except queue.Empty:
                pass
            page.update()

        page.timer_interval = 100
        page.on_timer = pump_rx


    show_connect_screen()

ft.app(target=main, view=ft.AppView.FLET_APP, assets_dir="assets")
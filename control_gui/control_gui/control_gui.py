import threading

import rclpy
from rclpy.node import Node

from std_msgs.msg import Bool, Float32, Int32, String

import tkinter as tk
from tkinter import ttk

UPDATE_INTERVAL_MS = 100  # 10 Hz

MISSIONS = [
    "M1: manual",
    "M2: acceleration",
    "M3: skidpad",
    "M4: autocross",
    "M5: track drive",
    "M6: evs test",
    "M7: inspection",
    "M8: shutdown",
    "M9: aux1",
    "M10: aux2",
]

SYSTEM_STATUSES = ["AS Ready", "AS Driving", "AS Emergency", "AS Finished"]


class VehicleControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Vehicle Control Panel")
        self.root.resizable(False, False)

        # --- TX: user-controlled, sent via ROS ---
        self.brake_state = tk.StringVar(value="Release")
        self.acceleration = tk.DoubleVar(value=0)
        self.direction = tk.DoubleVar(value=0)

        # Plain Python TX vars snapshotted at 10 Hz
        self.val_acceleration: float = 0.0
        self.val_direction: float = 0.0
        self.val_brakes: str = "Release"

        # --- RX: received via ROS, displayed read-only ---
        self.rx_ebs1 = tk.IntVar(value=0)
        self.rx_ebs2 = tk.IntVar(value=0)
        self.rx_system_status = tk.StringVar(value="AS Ready")
        self.rx_mission = tk.StringVar(value="M1: manual")

        self._build_ui()
        self._poll()

    def _build_ui(self):
        pad = {"padx": 12, "pady": 8}

        # ── TX section ────────────────────────────────────────────
        tx_frame = ttk.LabelFrame(self.root, text="Send  (user → vehicle)")
        tx_frame.grid(row=0, column=0, sticky="ew", **pad)

        # Acceleration
        acc_frame = ttk.LabelFrame(tx_frame, text="Acceleration")
        acc_frame.grid(row=0, column=0, columnspan=2, sticky="ew", padx=8, pady=6)

        ttk.Scale(
            acc_frame,
            from_=0, to=100,
            orient="horizontal",
            variable=self.acceleration,
            length=280,
            command=lambda v: self._refresh_label(self.acc_label, self.acceleration, "%")
        ).grid(row=0, column=0, padx=8, pady=6)

        self.acc_label = ttk.Label(acc_frame, text="0 %", width=6, anchor="w")
        self.acc_label.grid(row=0, column=1, padx=(0, 8))

        # Direction
        dir_frame = ttk.LabelFrame(tx_frame, text="Direction")
        dir_frame.grid(row=1, column=0, columnspan=2, sticky="ew", padx=8, pady=6)

        ttk.Scale(
            dir_frame,
            from_=-100, to=100,
            orient="horizontal",
            variable=self.direction,
            length=280,
            command=lambda v: self._refresh_label(self.dir_label, self.direction, "", center=True)
        ).grid(row=0, column=0, padx=8, pady=6)

        self.dir_label = ttk.Label(dir_frame, text="  0  (center)", width=14, anchor="w")
        self.dir_label.grid(row=0, column=1, padx=(0, 8))

        # Brakes
        brk_frame = ttk.LabelFrame(tx_frame, text="Brakes")
        brk_frame.grid(row=2, column=0, columnspan=2, sticky="ew", padx=8, pady=6)

        for text in ("Release", "Push"):
            ttk.Radiobutton(
                brk_frame,
                text=text,
                variable=self.brake_state,
                value=text
            ).pack(side="left", padx=16, pady=6)

        # ── RX section ────────────────────────────────────────────
        rx_frame = ttk.LabelFrame(self.root, text="Receive  (vehicle → GUI)")
        rx_frame.grid(row=1, column=0, sticky="ew", **pad)

        # EBS Pressures
        ebs_frame = ttk.LabelFrame(rx_frame, text="EBS Pressures (0–10)")
        ebs_frame.grid(row=0, column=0, columnspan=2, sticky="ew", padx=8, pady=6)

        for col, (text, var) in enumerate([("EBS 1", self.rx_ebs1), ("EBS 2", self.rx_ebs2)]):
            ttk.Label(ebs_frame, text=text).grid(row=0, column=col * 2, padx=(16, 4), pady=8)
            ttk.Label(ebs_frame, textvariable=var, width=5, anchor="center",
                      relief="sunken").grid(row=0, column=col * 2 + 1, padx=(0, 16), pady=8)

        # System Status
        sys_frame = ttk.LabelFrame(rx_frame, text="System Status")
        sys_frame.grid(row=1, column=0, columnspan=2, sticky="ew", padx=8, pady=6)

        ttk.Label(sys_frame, textvariable=self.rx_system_status,
                  width=22, anchor="center", relief="sunken").grid(
            row=0, column=0, padx=16, pady=8)

        # Mission
        mis_frame = ttk.LabelFrame(rx_frame, text="Mission")
        mis_frame.grid(row=2, column=0, columnspan=2, sticky="ew", padx=8, pady=6)

        ttk.Label(mis_frame, textvariable=self.rx_mission,
                  width=22, anchor="center", relief="sunken").grid(
            row=0, column=0, padx=16, pady=8)

    def _refresh_label(self, label, var, suffix, center=False):
        val = var.get()
        if center:
            rounded = round(val)
            side = "left" if rounded < 0 else ("right" if rounded > 0 else "center")
            label.config(text=f"{rounded:+d}  ({side})")
        else:
            label.config(text=f"{round(val)} {suffix}")

    def _poll(self):
        """Snapshot TX widget values at 10 Hz. ROS callbacks update rx_* vars directly."""
        self.val_acceleration = round(self.acceleration.get(), 1)
        self.val_direction    = round(self.direction.get(), 1)
        self.val_brakes       = self.brake_state.get()

        self.root.after(UPDATE_INTERVAL_MS, self._poll)

class VehicleNode(Node):
    def __init__(self, gui):
        super().__init__('vehicle_control_node')
        self.gui = gui

        # Publishers (TX: GUI → vehicle)
        self.pub_acc = self.create_publisher(Float32, '/acceleration', 10)
        self.pub_dir = self.create_publisher(Float32, '/direction', 10)
        self.pub_brk = self.create_publisher(Bool,    '/brakes',       10)

        # Subscribers (RX: vehicle → GUI)
        self.create_subscription(Int32,  '/ebs1',          self._cb_ebs1,   10)
        self.create_subscription(Int32,  '/ebs2',          self._cb_ebs2,   10)
        self.create_subscription(String, '/system_status', self._cb_status,  10)
        self.create_subscription(String, '/mission',       self._cb_mission, 10)

        self.create_timer(UPDATE_INTERVAL_MS / 1000.0, self._publish)

    def _publish(self):
        acc = Float32(); acc.data = float(self.gui.val_acceleration)
        dir_ = Float32(); dir_.data = float(self.gui.val_direction)
        brk = Bool();    brk.data = (self.gui.val_brakes == "Push")
        self.pub_acc.publish(acc)
        self.pub_dir.publish(dir_)
        self.pub_brk.publish(brk)

    # Callbacks se ejecutan en el hilo ROS → usamos root.after para actualizar Tkinter
    def _cb_ebs1(self, msg):
        self.gui.root.after(0, lambda: self.gui.rx_ebs1.set(msg.data))

    def _cb_ebs2(self, msg):
        self.gui.root.after(0, lambda: self.gui.rx_ebs2.set(msg.data))

    def _cb_status(self, msg):
        self.gui.root.after(0, lambda: self.gui.rx_system_status.set(msg.data))

    def _cb_mission(self, msg):
        self.gui.root.after(0, lambda: self.gui.rx_mission.set(msg.data))


def main(args=None):
    rclpy.init(args=args)

    root = tk.Tk()
    gui = VehicleControlGUI(root)

    node = VehicleNode(gui)

    spin_thread = threading.Thread(target=rclpy.spin, args=(node,), daemon=True)
    spin_thread.start()

    root.mainloop()

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
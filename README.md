# FreeRTOS Queue Communication on STM32F401CCU6

This project demonstrates inter-task communication using **FreeRTOS queues** on an **STM32F401CCU6** microcontroller. It features a **menu-driven user interface** over UART to dynamically select between controlling an LED or interacting with a Real-Time Clock (RTC).

---

## 🧩 Key Features

- UART-based runtime menu to select task behavior
- Modular tasks using FreeRTOS:
  - `menu_task`: Displays menu options via UART
  - `command_task`: Interprets user commands and dispatches tasks
  - `print_task`: Outputs responses or system info
  - `led_task`: Toggles LED with varying delays
  - `rtc_task`: Fetches and prints date/time from RTC
- Inter-task communication via **queues**
- Real-time logging of task events

---

## ⚙️ Hardware Used

- STM32F401CCU6 (Black Pill board)
- On-board LED (or external if specified)
- Optional external RTC via I2C (if implemented)
- UART interface (e.g., USB to serial)

---

## 🔧 How It Works

1. `menu_task` prints a list of commands over UART.
2. User selects an option (e.g., `LED_TASK` or `RTC_TASK`).
3. `command_task` receives and parses the command.
4. Corresponding task (`led_task` or `rtc_task`) is triggered via queue message.
5. Output is sent to `print_task`, which prints via UART.

---

## 🛠️ Tools Used

- STM32CubeIDE
- STM32 HAL Drivers
- FreeRTOS (CMSIS integration)
- SEGGER SystemView (optional profiling)
- PuTTY / Tera Term (for UART interaction)

---

## 🚀 Getting Started

1. Clone the repository  
   `git clone https://github.com/Santosh64194/FreeRTOS-Queue-communication-on-STM32F401CCU6`

2. Open the project in **STM32CubeIDE**

3. Connect STM32F401 board via USB

4. Build and flash the code

5. Open a serial monitor (e.g., PuTTY) at 115200 baud and interact with the menu

---

## 📂 Folder Structure

- `Core/Src/`: Task implementations and queue logic
- `Core/Inc/`: Header files for task prototypes
- `Drivers/`: STM32 HAL libraries
- `FreeRTOS/`: CMSIS-RTOS abstraction and kernel
- `README.md`: Project overview

---

## 📌 Future Improvements

- Add more user-selectable tasks
- Dynamic memory allocation example
- Integration with sensors or actuators
- Persistent RTC configuration

---

## 📷 Screenshots

> *(Add UART terminal screenshots or task flow diagram here if needed)*

---

## 📄 License

This project is open source and free to use for educational purposes.


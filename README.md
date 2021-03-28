# traffic-light
The goal of this project is to detect the button state and blink the LED with the multitasking implementation. There are two states for LED: (1) The green LED lights up for 5 seconds, then turns to the red LED lights up for 5 seconds (green LED off), and then switches back to the green LED lights up for 5 seconds (red LED off), and so on; (2) The red LED blinks. If the button is detected as pressed, the LED should switch to another state. 



## Environment

- VirtualBox 5.2.18 
- Ubuntu 18.04
- STM32F407G-DISC1
- STM32CubeIDE 1.5.1

To run the program correctly, note that

1. Install VirtualBox Extension Pack to support USB 2.0/3.0 devices
2. Allocate 4GB for RAM since the IDE requires large memory 
3. Use lsusb command to check if the USB device is connected 



## Implementation

Create two functions called led_task() and button_task() for the LED and button, respectively. To implement an inter-process communication (IPC), create a global queue `qh` to collect the messages for these two tasks. The function button_task() keeps reading the input from the button. Use two local variables `prev_state` and `current_state` to record the detected states. If the button is detected as 0 -> 1, send the message to the global queue with the function xQueueSend(). The function led_task() uses a local variable `key` to switch different jobs. It first executes job 1 and keeps checking whether there is any message in the queue. If it receives the message from the button, it will reset the current LED, change the value of the key, switch to the job 2, and vice versa. To avoid the redundant delay when receiving the messages, we can use busy waiting (waste resource, however) or set `xTicksToWait` value in the function xQueueReceive(). 



## Usage

Open the IDE from the terminal

```
$ cd /opt/st/stm32cubeide_1.5.1
$ ./stm32cubeide
```

Click Project > Build all, then click Run > Run. 



## Demo


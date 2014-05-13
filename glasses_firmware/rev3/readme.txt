Driver projects for the rev3 board (Cortex M3, no FPGA)

******************Feature Test / Demo Projects******************

- basic_usb: Implementation of a simple interrupt-based USB transfer protocol, based on an official example project from ST.

- imu_read: Basic communication with the IMU, including an algorithm for calculating orientation based on IMU data

- isoch_usb_simple: Simple isochronous data transfer example, based on ST example. Device imitates a speaker and receives a data stream from the host.

- isoch_usb_full: Isochronous data transfer with the device imitating a microphone, sending a data stream to the host.

- microphone_read: Get a stream of audio data from the microphone (untested).



******************Camera Streaming Projects******************

- dual_camera_full: Collect data from both cameras at full speed and record to SD card

- gaze_predict: Early gaze prediction driver, superceded by gaze_predict_flash

- gaze_predict_flash: Perform gaze predict based on eye-facing camera data, no video recording at all

- isoch_camera: Transmit video data from scene-facing camera along with the computed gaze prediction over USB
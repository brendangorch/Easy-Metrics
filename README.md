# Easy-Metrics
A C++ program for monitoring AMD GPU and CPU performance metrics through a customizable screen overlay for Windows.
</br>
</br>

## How to Run
- Ensure you have Microsoft Visual C++ Redistributable installed. Most machines should already have it but a link to the download is provided just in case: [Microsoft Visual C++ Redistributable latest supported downloads](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).
- Download the Easy-Metrics.zip from releases.
- Run the Easy-Metrics.exe.
- Select the metrics you want to display and customize the overlay, then click "Display Overlay".
</br>
</br>

## Customize the Overlay
Select a combination of metrics. Adjust the overlay, metric label and value colours. Choose the level of transparency and size of text. View the sample metric at the bottom right and adjust til you're happy.
</br>

![Image](https://github.com/user-attachments/assets/e92ed5fe-dd63-44db-a3d8-0b7573c81f26)

![Image](https://github.com/user-attachments/assets/16f395a1-1cc9-40bb-904a-dab87da93034)
</br>
</br>

## Real-Time Metric Updates
The metrics are updated in real-time, providing you the most current GPU and CPU stats.
</br>

![Image](https://github.com/user-attachments/assets/5d5c6d1c-7ccf-43bf-b9db-71f3f75b9e31)
</br>
</br>

## Libraries Used
- SFML with ImGUI were used for window and overlay creation, and all UI.
  - [SFML](https://www.sfml-dev.org/)
  - [ImGUI](https://github.com/ocornut/imgui)
- ADLX was used for gathering performance metrics from AMD hardware.
  - [ADLX](https://gpuopen.com/adlx/)


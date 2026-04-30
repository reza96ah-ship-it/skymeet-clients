# SkyMeet Clients

Qt 5.15.2 QML client for the SkyMeet video conferencing / stream-session system.

## Features

- Login / register against SkyMeet Drogon backend
- User list and create-member flow
- Session list and create-session flow
- Stream list page
- WebSocket event connection
- Qt 5.15.2 compatible QML imports

## Build with CMake

```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/5.15.2/gcc_64
cmake --build . -j
./skymeet_qml_client_qt515
```

## Build with qmake

```bash
qmake skymeet_qml_client_qt515.pro
make -j
./skymeet_qml_client_qt515
```

## Restore full source bundle

The full generated client source is also available as a base64 tarball in this repository:

```bash
base64 -d skymeet_qml_client_qt515_fix2.tar.gz.base64 > skymeet_qml_client_qt515_fix2.tar.gz
tar -xzf skymeet_qml_client_qt515_fix2.tar.gz
```

# HubSight Desktop App

Bộ khung ứng dụng desktop sử dụng Qt 6, C++20, CMake 3.21 trở lên và HubSight Admin SDK.

## Yêu cầu

- CMake 3.21+
- Trình biên dịch hỗ trợ C++20
- Qt 6 với các module `Gui`, `Qml`, `Quick`, `QuickControls2` và `QuickDialogs2`
- HubSight Qt SDK 0.2.0 với Admin `.hscfg` importer

## Cài đặt HubSight Qt SDK

SDK được lấy từ [HubSight/qt-sdk](https://github.com/HubSight/qt-sdk). Với
workspace hiện tại, SDK đã có bản build trong thư mục `../qt-sdk/dist` và
CMake sẽ tự phát hiện bản phù hợp.

Nếu cần build và cài SDK từ source repository:

```bash
git clone https://github.com/HubSight/qt-sdk.git ../qt-sdk
cmake -S ../qt-sdk -B ../qt-sdk/build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DHUBSIGHT_ADMIN_BUILD_TESTS=OFF
cmake --build ../qt-sdk/build-release --parallel
cmake --install ../qt-sdk/build-release --prefix ../qt-sdk/install
```

Sau đó trỏ ứng dụng đến prefix vừa cài bằng `HUBSIGHT_SDK_PREFIX`.

Nếu cài SDK ở vị trí khác:

```bash
cmake --preset default -DHUBSIGHT_SDK_PREFIX=/path/to/qt-sdk-prefix
```

Prefix phải chứa:

```text
lib/cmake/HubSightAdminSdk/HubSightAdminSdkConfig.cmake
```

Hoặc để CMake tự lấy tag `v0.2.0` từ GitHub và build cùng ứng dụng:

```bash
cmake --preset default -DHUBSIGHT_FETCH_SDK=ON
```

Trong môi trường không có mạng, có thể trỏ trực tiếp đến source checkout:

```bash
cmake --preset default \
  -DHUBSIGHT_SDK_SOURCE_DIR=/path/to/qt-sdk
```

## Build và chạy

```bash
cmake --preset default
cmake --build --preset default
```

Chạy ứng dụng trên macOS:

```bash
open build-debug/HubSight.app
```

Trên Linux, chạy binary:

```bash
./build-debug/HubSight
```

Để tạo bản release:

```bash
cmake --preset release
cmake --build --preset release
```

Nếu Qt không nằm trong đường dẫn mặc định, truyền `CMAKE_PREFIX_PATH` khi cấu hình:

```bash
cmake --preset default -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/<platform>
```

## Cấu trúc

```text
.
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── assets
│   └── icons
├── qml
│   ├── AuthPage.qml
│   ├── ImportWizard.qml
│   ├── Main.qml
│   ├── Splash.qml
│   ├── Workspace.qml
│   └── components
└── src
    ├── appcontroller.cpp
    ├── appcontroller.hpp
    ├── main.cpp
    └── resources.qrc
```

UI được triển khai bằng QML/Qt Quick. `AppController` là lớp bridge C++ giữ
toàn bộ logic HubSight Admin SDK, import `.hscfg`, PIN, đăng nhập và 2FA.

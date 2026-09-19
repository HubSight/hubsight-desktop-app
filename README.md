# HubSight Desktop App

Bộ khung ứng dụng desktop sử dụng Qt 6, C++20, CMake 3.21 trở lên và HubSight Admin SDK.

## Yêu cầu

- CMake 3.21+
- Trình biên dịch hỗ trợ C++20
- Qt 6 với các module `Gui`, `Network`, `Qml`, `Quick`, `QuickControls2`, `QuickDialogs2` và `LinguistTools`
- HubSight Qt SDK 0.2.0 với Admin `.hscfg` importer và `HubSight::Preferences`

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
lib/cmake/HubSightPreferences/HubSightPreferencesConfig.cmake
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

## i18n

Ứng dụng sử dụng Qt Linguist (`QTranslator`, `qsTr()` và `tr()`). Tiếng Anh là
ngôn ngữ nguồn; bản dịch tiếng Việt nằm trong
`translations/hubsight_vi.ts` và được biên dịch thành `.qm`, nhúng vào bundle
tự động bởi CMake.

Khi thêm hoặc sửa chuỗi giao diện, cập nhật catalog bằng:

```bash
cmake --build build-debug --target HubSight_lupdate
```

Sau đó chỉnh bản dịch trong file `.ts` và build lại ứng dụng. Nút ngôn ngữ
trong wizard đổi ngôn ngữ ngay khi chạy và lựa chọn được lưu bằng
`HubSight::Preferences`.

Chạy ứng dụng trên macOS:

```bash
open build-debug/HubSight.app
```

Trên Linux, chạy binary:

```bash
./build-debug/HubSight
```

Ứng dụng chỉ cho phép một instance chạy trong mỗi user session. Nếu mở lần
thứ hai, process mới sẽ gửi yêu cầu activate đến instance đang chạy rồi thoát.
Lock file và kênh IPC được tạo trong thư mục dữ liệu cục bộ của ứng dụng.

Để tạo bản release:

```bash
cmake --preset release
cmake --build --preset release
```

Nếu Qt không nằm trong đường dẫn mặc định, truyền `CMAKE_PREFIX_PATH` khi cấu hình:

```bash
cmake --preset default -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/<platform>
```

## File association `.hscfg`

Ứng dụng đăng ký định dạng `.hscfg` khi được cài đặt:

- macOS: khai báo document type trong bundle `Info.plist`.
- Windows: ghi association vào `HKCU` trong bước `cmake --install`, không yêu cầu quyền administrator.
- Linux: cài MIME type, `.desktop` entry và icon vào thư mục dữ liệu chuẩn.

Khi mở một file `.hscfg`, app sẽ chuyển tới bước chọn file. Nếu HubSight đã
chạy, file sẽ được chuyển vào instance hiện tại thay vì mở thêm process.

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
├── translations
│   └── hubsight_vi.ts
└── src
    ├── appcontroller.cpp
    ├── appcontroller.hpp
    ├── main.cpp
    └── resources.qrc
```

UI được triển khai bằng QML/Qt Quick. `AppController` là lớp bridge C++ giữ
toàn bộ logic HubSight Admin SDK, import `.hscfg`, PIN, đăng nhập và 2FA.

Các thiết lập giao diện được lưu bởi `HubSight::Preferences` trong
`preferences.json` dưới thư mục cấu hình ứng dụng. Bản build cũ dùng QSettings
được migrate một lần khi file PreferenceStore chưa tồn tại; sau đó app chỉ ghi
PreferenceStore.

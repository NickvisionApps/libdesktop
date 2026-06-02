# libdesktop
A cross-platform C++20 base library for native desktop applications. It provides a dependency-injection host, encrypted configuration and keyring storage, cross-platform secrets, HTTP networking, notifications, filesystem utilities, and more — all as a single static or shared library.

## Table of Contents
- [Dependencies](#dependencies)
- [Build Instructions](#build-instructions)
- [Install Instructions](#install-instructions)
- [API Overview](#api-overview)
  - [Hosting](#hosting-desktophosting)
  - [Services](#services-desktopservices)
  - [App](#app-desktopapp)
  - [Database](#database-desktopdatabase)
  - [Secrets](#secrets-desktopsecrets)
  - [Network](#network-desktopnetwork)
  - [Notifications](#notifications-desktopnotifications)
  - [Filesystem](#filesystem-desktopfilesystem)
  - [System](#system-desktopsystem)
  - [Updates](#updates-desktopupdates)
  - [Events](#events-desktopevents)
- [License](#license)

## Dependencies

### All Platforms
| Dependency | Notes |
|---|---|
| CMake ≥ 3.25 | Build system |
| C++20 compiler | GCC 13+, Clang 16+, or MSVC 19.29+ |
| OpenSSL | Encryption backend for SQLCipher |
| libcurl | HTTP networking |
| libarchive | Archive/compression support |
| [nlohmann_json](https://github.com/nlohmann/json) | JSON serialization |
| [maddy](https://github.com/progsource/maddy) 1.6.0 | Markdown-to-HTML conversion |
| gettext / libintl | Translations |

### Linux (additional)
| Dependency | Notes |
|---|---|
| glib-2.0, gio-2.0, gmodule-2.0, gobject-2.0, gthread-2.0 | GLib runtime |
| libsecret-1 | System keychain backend |
| dbus-x11, gnome-keyring | Required to run tests |

### macOS (additional)
| Dependency | Notes |
|---|---|
| CoreFoundation | Bundled with Xcode |
| CoreServices | Bundled with Xcode |
| IOKit | Bundled with Xcode |
| Network | Bundled with Xcode |
| Security | Bundled with Xcode |

### Windows (additional)
| Dependency | Notes |
|---|---|
| Kernel32 | Standard Windows SDK |
| Shell32 | Standard Windows SDK |

## Build Instructions

### Clone
```bash
git clone --recursive https://github.com/NickvisionApps/libdesktop
cd libdesktop
```

### CMake
| Option | Default | Description |
|---|---|---|
| `BUILD_SHARED_LIBS` | `OFF` | Build as a shared library instead of static |
| `BUILD_TESTING` | `ON` | Build the test suite |
| `ENABLE_CLANG_FORMAT` | `ON` | Auto-format sources with clang-format |
| `ENABLE_CLANG_TIDY` | `ON` | Run clang-tidy static analysis |

#### vcpkg
If `VCPKG_ROOT` is set, the toolchain file is applied automatically. Install the required packages before configuring.

#### Configure
```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20
```

### Build
```bash
cmake --build build --parallel
```

### Run Tests
```bash
ctest --test-dir build --output-on-failure
```

---

## Install Instructions

### Linux
```bash
sudo cmake --install build --prefix /usr
```

### macOS (Apple Silicon)
```bash
cmake --install build --prefix /opt/homebrew
```

### macOS (Intel)
```bash
cmake --install build --prefix /usr/local
```

### Windows (vcpkg)
```bash
vcpkg install libdesktop
```

## API Overview

Include everything with a single header:
```cpp
#include <libdesktop.h>
```

Or include individual module headers (e.g. `<hosting.h>`, `<network.h>`, etc.).

---

### Hosting (`desktop::hosting`)
The application entry point. `host` wires up the service container and runs the application lifetime.

```cpp
auto info = std::make_shared<desktop::app::app_info>("com.example.App", "My App", "My App", true);
desktop::hosting::host h{ info, std::span{ argv, static_cast<size_t>(argc) } };
h.use_logging();
h.use_github_updates();
h.get_services()->add<MyService>(desktop::services::service_scope::singleton);
h.run();
```

| Method | Description |
|---|---|
| `host(host_options)` | Construct with app info and argv |
| `get_services()` | Access the `service_collection` DI container |
| `use_logging(min, path)` | Enable file/console logging |
| `use_github_updates()` | Register the GitHub update service |
| `use_lifetime<T>()` | Register a custom `lifetime_service` |
| `run()` | Start the host; returns any unhandled exception |

---

### Services (`desktop::services`)
A compile-time-safe dependency injection container. Services declare their constructor dependencies via a `using dependencies = std::tuple<...>` alias and are resolved automatically.

```cpp
auto services = h.get_services();
services->add<MyService>(service_scope::singleton);
auto svc = services->get_required<MyService>();
```

| Scope | Description |
|---|---|
| `singleton` | One instance shared for the lifetime of the host |
| `transient` | New instance created on every `get` call |

---

### App (`desktop::app`)

#### `app_info`
Metadata about the application (id, name, version, URLs, credits, etc.).

#### `configuration_service`
Typed key-value configuration stored in an encrypted SQLCipher database. Supports `bool`, `int`, `int64_t`, `float`, `double`, `std::string`, enums, and any JSON-serializable object.

```cpp
auto config = services->get_required<desktop::app::configuration_service>();
int count = config->get<int>("launch_count", 0);
config->set("launch_count", count + 1);
config->get_saved_event() += [](auto& sender, auto& args) { /* handle save */ };
```

#### `keyring_service`
Stores named `credential` objects (username + password pairs) in an encrypted local database.

```cpp
auto keyring = services->get_required<desktop::app::keyring_service>();
keyring->add_credential({ "my-service", "alice", "s3cr3t" });
auto creds = keyring->get_all_credentials();
keyring->update_credential({ "my-service", "alice", "new-pass" });
keyring->delete_credential({ "my-service", "alice", "" });
```

#### `logger`
Structured logging with configurable minimum level and optional file output. Log levels: `debug`, `info`, `warning`, `error`.

#### `translation_service`
Wraps gettext for runtime locale-aware string translation.

#### `arguments_service`
Parses and exposes command-line arguments.

#### `ipc_service`
Single-process IPC via a Unix domain socket (Linux/macOS) or named pipe (Windows).

---

### Database (`desktop::database`)

#### `database_service`
SQLCipher-encrypted SQLite wrapper. The database key is derived from the `secret_service`. Falls back to an unencrypted in-memory database when no secret is available.

```cpp
auto db = services->get_required<desktop::database::database_service>();
db->ensure_table_exists("items", { { "id", "TEXT PRIMARY KEY" }, { "value", "TEXT" } });
db->insert_into_table("items", { { "id", "1" }, { "value", "hello" } });
auto rows = db->select_all_from_table("items");
```

---

### Secrets (`desktop::secrets`)

#### `secret_service`
Cross-platform secure secret storage.
- **Windows** — DPAPI
- **macOS** — Keychain
- **Linux** — libsecret / GNOME Keyring

```cpp
auto ss = services->get_required<desktop::secrets::secret_service>();
auto secret = ss->create("my-app-key"); // random password assigned
auto retrieved = ss->get("my-app-key");
```

#### `password_generator`
Generates random passwords with configurable length and character set (`password_content` flags: `letters`, `digits`, `special`).

---

### Network (`desktop::network`)

#### `http_service`
Full HTTP client built on libcurl.

```cpp
auto http = services->get_required<desktop::network::http_service>();

// Basic request
auto response = http->get("https://api.example.com/data");
if (response.is_ok()) { auto json = response.json(); }

// Request with custom headers (e.g. Authorization)
std::vector<std::pair<std::string, std::string>> headers{ { "Authorization", "Bearer <token>" } };
auto auth_response = http->get("https://api.example.com/protected", headers);

// File download with progress callback
http->download_file("https://example.com/file.zip", "/tmp/file.zip", true,
    [](const auto& p) { std::cout << p.get_percent() << "%\n"; });
```

Supports `GET`, `POST`, `PUT`, `PATCH`, `DELETE`, JSON and form bodies, file upload, and file download with progress callbacks. All request methods accept an optional `std::vector<std::pair<std::string, std::string>>` of custom headers appended before any content-type headers.

#### `network_monitor`
Observes connectivity state changes.

---

### Notifications (`desktop::notifications`)

#### `notification_service`
Sends both in-app (`app_notification`) and OS shell (`shell_notification`) notifications.

```cpp
auto notifs = services->get_required<desktop::notifications::notification_service>();
notifs->send(desktop::notifications::app_notification{ "Operation complete", notification_severity::success });
notifs->send(desktop::notifications::shell_notification{ "My App", "Background task finished" });
notifs->get_app_notification_sent_event() += [](auto& sender, auto& args) { /* update UI */ };
```

---

### Filesystem (`desktop::filesystem`)

#### `user_directories`
Resolves platform-specific user directories.

```cpp
auto home   = desktop::filesystem::user_directories::get_home();
auto config = desktop::filesystem::user_directories::get_config();
auto docs   = desktop::filesystem::user_directories::get_documents();
// also: get_cache, get_desktop, get_downloads, get_local_data,
//       get_music, get_pictures, get_templates, get_videos
```

#### `folder_watcher`
Watches a directory for changes and fires events with a `folder_watcher_change_flag` bitmask.

---

### System (`desktop::system`)

#### `environment` (free functions in `desktop::system::environment`)
```cpp
desktop::system::environment::execute("ls -la");
desktop::system::environment::get_variable("HOME");
desktop::system::environment::set_variable("MY_VAR", "value");
desktop::system::environment::find_dependency("ffmpeg", dependency_search_option::path);
desktop::system::environment::get_deployment_mode(); // returns deployment_mode enum
```

#### `process`
Spawns and manages child processes, captures stdout/stderr, and returns a `process_result`.

#### `power_service`
Monitors system power/battery state changes.

#### `executable_service`
Resolves and launches platform-appropriate executables.

---

### Updates (`desktop::updates`)

#### `github_update_service`
Checks the GitHub Releases API for newer versions and optionally downloads assets.

```cpp
auto updater = services->get_required<desktop::updates::github_update_service>();
auto latest = updater->get_latest_version(false); // false = stable only
if (latest && *latest > current_version)
{
    updater->download_asset(*latest, "app-linux-x64.tar.gz", "/tmp/update.tar.gz");
}
```

---

### Events (`desktop::events`)

Thread-safe, typed event system used throughout the library.

```cpp
desktop::events::event<MyClass, MyEventArgs> my_event;
auto id = my_event.add_handler([](const MyClass& sender, const MyEventArgs& args) { });
my_event.invoke(*this, args);
my_event.remove_handler(id);
// or use += / -= operators
```

---

## License
See [LICENSE](LICENSE).

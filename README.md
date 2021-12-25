# esp8266_web_settings
<h1>Generic ESP8266 Web Settings Library</h1>

This repository contains a generic Web page to manage your device settings. Settings are presented in tabs, with a set of buttons below:
* *Save*: Save the settings in the UI.
* *Reset Form*: Reload settings, discarding any changes you made.
* *Reboot*: Reboot your device.
* *Factory Reset*: Erase all settings on the device and reset.
* *Upload Firmware*: Upload firmware to your device.

Save, Reboot, Factory Reset and Uplod Firmware can be optionally password-protected.

Settings on each tab are displayed in a two-column table.

<h1>Usage</h1>
There are several settings classes:

* `NoteSetting`. This "setting" isn't actually a setting; the content of the object is simply placed verbatim in a row spanning the two columns. It can contain arbitrary HTML. It is not updatable.
* `StringSetting`: A setting containing a string. The string is not validated. The HTML is a single-line text input box.
* `PasswordSetting`: A special setting, containing a _write-only_ password. The password is never sent to the web page; to change the password, the user will tick a check box to enable it and enter the password. The HTML is a password input box.
* `SignedIntegerSetting`. A setting containing a signed integer. The HTML is a numeric input box with no constraints.
* `UnsignedIntegerSetting`. A setting containing an unsigned integer. The HTML is a numeric input box with a minimum of 0.
* `FloatSetting`. A setting containing a floating-point value. The HTML is a numeric input box with no constraints.
* `ExclusiveOptionSetting`. A setting presented as a drop-down list.
* `ToggleSetting`. A setting containing a boolean; presented as a checkbox.
* `InfoSetting`. A "setting" that displays information that can be updated every 5 seconds. It can contain arbitrary HTML.

Public methods in the `WebServer` class:

* `WebServer(uint16_t port = 80)`: Construct a new Web server; optionally specify the port.
* `void setup(const notify_t &on_save, const notify_t &on_restart, const notify_t &on_factory_reset)`: Set up to handle requests.
* `void add_setting_set(const __FlashStringHelper *name, const SettingInterface::settings_list_t &setting_set);`: Add a collection of settings; creates a setting tab.
* `void set_credentials(const String &user, const String &password)`: Set credentials for save, reset, factory reset, and upload operations.
* `AsyncWebServer &get_server()`: Get the internal web server.

Settings are complex, see the generated documentation or source for details.

The `SettingPanel` class is intended for internal use.

Example:
```
#include <ESP8266WiFi.h>
#include <LittleFS.h>

#include "grmcdorman/WebServer.h"

grmcdorman::WebServer webServer;

#define DECLARE_INFO_SETTING(name, text) \
static const char * PROGMEM name##_text = text; \
static const char * PROGMEM name##_id = #name; \
static ::grmcdorman::InfoSettingHtml name(FPSTR(name##_text), FPSTR(name##_id));

static const char * PROGMEM info_title_text = "<h1>" FIRMWARE_PREFIX "</h1>"
    "<h2>System information</h2>";
static const char * PROGMEM info_footer_text =
    "<script>window.addEventListener(\"load\", () => { periodicUpdateList.push(\"Overview\"); });</script>";

static ::grmcdorman::NoteSetting info_title(FPSTR(info_title_text));
// Applicable when connected to AP.
DECLARE_INFO_SETTING(host, "Host name");
DECLARE_INFO_SETTING(station_ssid, "Connected to");  // SSID
DECLARE_INFO_SETTING(rssi, "Signal strength");
// Applicable in AP mode.
DECLARE_INFO_SETTING(softap, "Soft Access Point SSID");
DECLARE_INFO_SETTING(free_heap, "Free heap memory");

static ::grmcdorman::NoteSetting info_footer(FPSTR(info_footer_text));

static const ::grmcdorman::SettingInterface::settings_list_t info_item_list{
    &info_title,
    &softap,
    &host,
    &station_ssid,
    &rssi,
    &free_heap,
    &uptime,
    &info_footer
};

static bool factory_reset_next_loop = false;
static bool restart_next_loop = false;
static uint32_t restart_reset_when = 0;
static constexpr uint32_t restart_reset_delay = 500;

static void on_factory_reset(::grmcdorman::WebServer &)
{
    factory_reset_next_loop = true;
    restart_reset_when = millis();
}

static void on_restart(::grmcdorman::WebServer &)
{
    restart_next_loop =-true;
    restart_reset_when = millis();
}

static void on_save(::grmcdorman::WebServer &)
{
    // Save your settings to flash.
}

void setup()
{

    webServer.add_setting_set(F("Overview"), info_item_list);
    // Callbacks for info page.
    static const char in_ap_mode[] PROGMEM = "(In Access Pont mode)";
    static const char in_sta_mode[] PROGMEM = "(In Station mode)";

    host.set_request_callback([] (const ::grmcdorman::InfoSettingHtml &) {
        if (!wifi_device.is_softap_mode())
        {
            host.set(WiFi.hostname() + F(" [") + WiFi.localIP().toString() + F("]"));
        }
        else
        {
            host.set(FPSTR(in_ap_mode));
        }
    });
    station_ssid.set_request_callback([] (const ::grmcdorman::InfoSettingHtml &) {
        if (!wifi_device.is_softap_mode())
        {
            station_ssid.set(WiFi.SSID());
        }
        else
        {
            station_ssid.set(FPSTR(in_ap_mode));
        }
    });
    rssi.set_request_callback([] (const ::grmcdorman::InfoSettingHtml &) {
        if (!wifi_device.is_softap_mode())
        {
            rssi.set(String(WiFi.RSSI()) + F(" dBm"));
        }
        else
        {
            rssi.set(FPSTR(in_ap_mode));
        }
    });
    softap.set_request_callback([] (const ::grmcdorman::InfoSettingHtml &) {
        if (wifi_device.is_softap_mode())
        {
            softap.set(WiFi.softAPSSID());
        }
        else
        {
            softap.set(FPSTR(in_sta_mode));
        }
    });
    free_heap.set_request_callback([] (const ::grmcdorman::InfoSettingHtml &) {
        free_heap.set(String(ESP.getFreeHeap()) + " bytes (fragmentation: " + String(ESP.getHeapFragmentation()) + ")");
    });

    // NOTE: SoftAP capture portal is not happy about authentication. You should not
    // make this call for SoftAP mode, or handle SoftAP differently.
    webServer.set_credentials("admin", "my update password");

    // If `on_restart` is a `nullptr`, the Restart & Upload buttons are not shown.
    // If `on_factory_reset` is a `nullptr`, the Factory Reset button is not shown.
    webServer.setup(on_save, on_restart, on_factory_reset);
}


void loop()
{
    webServer.loop();      // This presently doesn't do anything. It is for future use.

    if (factory_reset_next_loop && millis() - restart_reset_when > restart_reset_delay)
    {
        // Clear file system.
        LittleFS.format();
        // Erase configuration
        ESP.eraseConfig();
        // Reset (not reboot, that may save current state)
        ESP.reset();
    }

    if (restart_next_loop && millis() - restart_reset_when > restart_reset_delay)
    {
        ESP.restart();
    }
}
```
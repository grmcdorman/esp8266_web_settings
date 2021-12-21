# esp8266_web_settings
<h2>Generic ESP8266 Web Settings Library</h2>

This repository contains a generic Web page to manage your device settings. Settings are presented in tabs, with a set of buttons below:
* *Save*: Save the settings in the UI.
* *Reset Form*: Reload settings, discarding any changes you made.
* *Reboot*: Reboot your device.
* *Factory Reset*: Erase all settings on the device and reset.
* *Upload Firmware*: Upload firmware to your device. WORK IN PROGRESS: to be changed.

Settings on each tab are displayed in a two-column table.

<h1>Usage</h1>
There are several settings classes:
* `NoteSetting`. This "setting" isn't actually a setting; the content of the object is simply placed verbatim in a row spanning the two columns. It can contain arbitrary HTML.
* `StringSetting`: A setting containing a string. The string is not validated. The HTML is a single-line text input box.
* `PasswordSetting`: A special setting, containing a _write-only_ password. The password is never sent to the web page; to change the password, the user will tick a check box to enable it and enter the password. The HTML is a password input box.
* `SignedIntegerSetting`. A setting containing a signed integer. The HTML is a numeric input box with no constraints.
* `UnsignedIntegerSetting`. A setting containing an unsigned integer. The HTML is a numeric input box with a minimum of 0.
* `FloatSetting`. A setting containing a floating-point value. The HTML is a numeric input box with no constraints.
* `ExclusiveOptionSetting`. A setting presented as a drop-down list.
* `ToggleSetting`. A setting containing a boolean; presented as a checkbox.

Example:
```
NoteSetting notes("These settings configure the MQTT server.<br>"
        "A blank server name disables MQTT operations.<br>"
        "MQTT topics will be:"
        "<ul>"
        "<li><em>prefix</em>/<em>identifier</em>/status"
        "<li><em>prefix</em>/<em>identifier</em>/state"
        "<li><em>prefix</em>/<em>identifier</em>/command"
        "</ul>"
        "Corresponding Home Assistant configurations will be published.");
StringSetting server_address("MQTT server name or IP", "server");
UnsignedIntegerSetting server_port("MQTT server port (standard 1883)", "port");
StringSetting username("The MQTT username (blank for anonymous)", "username");
PasswordSetting password("The MQTT password", "password");
ExclusiveOptionSetting communicationMode("Communication mode", { "Plain", "SSL"});

SettingsInterface::settings_list_t list{&notes, &server_address, &server_port, &username, &password, &communicationMode};

WebServer server;

void setup()
{
   server.setup([] {
      // Handle "save" here by extracting values from the settings.
   });
   server.add_setting_set("MQTT", list);
   // Add additional name/lists for each tab.
}

void loop()
{
   server.loop();
}
```
#pragma once

#include <ArduinoJson.h>
#include <WString.h>
#include <ESP8266WebServer.h>

#include "grmcdorman/Setting.h"

namespace grmcdorman
{
    class SettingPanel
    {
    public:
        SettingPanel(const String &name, const SettingInterface::settings_list_t &settings_set);
        String body() const;
        void on_post(ESP8266WebServer &server);
        DynamicJsonDocument as_json() const;
        String get_name() const
        {
            return name;
        }
    private:
        String container_name;
        String name;
        SettingInterface::settings_list_t settings;
    };
}

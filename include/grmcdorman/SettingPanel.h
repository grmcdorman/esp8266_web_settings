#pragma once

#include <ArduinoJson.h>
#include <WString.h>
#include <ESPAsyncWebServer.h>

#include "grmcdorman/Setting.h"

namespace grmcdorman
{
    class SettingPanel
    {
    public:
        SettingPanel(const String &name, const SettingInterface::settings_list_t &settings_set);
        String body() const;
        void on_post(AsyncWebServerRequest *request);
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

#include "grmcdorman/SettingPanel.h"

#include <ArduinoJson.h>

#include "grmcdorman/Setting.h"

namespace grmcdorman
{
    SettingPanel::SettingPanel(const String &name, const SettingInterface::settings_list_t &settings_set): name(name), settings(settings_set)
    {
    }

    String SettingPanel::body() const
    {
        String message("<table>");

        for(const auto &setting: settings)
        {
            message +=  setting->html(name);
        }
        message += "</table>";
        return message;
    }

    DynamicJsonDocument SettingPanel::as_json() const
    {
        if (settings.size() == 0)
        {
            return DynamicJsonDocument(1);
        }

        size_t stateSize = 512 + 1024 * settings.size();
        DynamicJsonDocument array(stateSize);
        for(auto &setting: settings)
        {
            if (setting->name().length() != 0 && setting->send_to_ui())
            {
                DynamicJsonDocument valueJson(1024);
                valueJson["name"] = name + "$" + setting->name();
                valueJson["value"] = setting->as_string();
                array.add(valueJson);
            }
        }
        return array;
    }

    void SettingPanel::on_post(AsyncWebServerRequest *request)
    {
        for(auto &setting: settings)
        {
            String argName = name + "$" + setting->name();
            Serial.println("Looking for " + argName);
            if (request->hasArg(argName.c_str()))
            {
                setting->set_from_post(request->arg(argName));
            }
            else if (setting->send_to_ui())
            {
                Serial.println("Set to default");
                setting->set_default();
            }
        }
    }
}
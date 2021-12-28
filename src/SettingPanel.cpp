#include "grmcdorman/SettingPanel.h"

#include <algorithm>
#include <ArduinoJson.h>

#include "grmcdorman/Setting.h"

namespace grmcdorman
{
    namespace {
        const char * PROGMEM NAME_STR = "name";
        const char * PROGMEM VALUE_STR = "value";
    }
    SettingPanel::SettingPanel(const __FlashStringHelper *name, const __FlashStringHelper *identifier, const SettingInterface::settings_list_t &settings_set):
        name(name),
        name_length(strlen_P(reinterpret_cast<const char *>(name))),
        identifier(identifier),
        identifier_length(strlen_P(reinterpret_cast<const char *>(identifier))),
        settings(settings_set)
    {
    }

    DynamicJsonDocument SettingPanel::as_json(const std::vector<const String *> &requested_settings) const
    {
        if (settings.size() == 0)
        {
            DynamicJsonDocument doc(JSON_ARRAY_SIZE(1));
            doc.to<JsonArray>();
            return doc;
        }

        // Sizing is critical here. Too small, and the document is incomplete;
        // too large, and it may not allocate at all - also resulting in an
        // incomplete document. Short of ad-hoc text as JSON or other hacks
        // to perform chunked output, this is not really scalable.
        bool all = requested_settings.empty();
        size_t stateSize = JSON_ARRAY_SIZE(settings.size()) + 512 * settings.size();
        DynamicJsonDocument doc(stateSize);
        doc.capacity();
        JsonArray array = doc.to<JsonArray>();

        for(auto &setting: settings)
        {
            auto len = strlen_P(reinterpret_cast<const char *>(setting->name()));
            if (len == 0 || !setting->send_to_ui())
            {
                continue;
            }

            if (!all)
            {
                if (!std::any_of(requested_settings.begin(), requested_settings.end(), [setting] (const String *s)
                    {
                        return strcmp_P(s->c_str(), reinterpret_cast<const char *>(setting->name())) == 0;
                    }))
                {
                    continue;
                }
            }

            auto value = setting->as_string();
            DynamicJsonDocument valueJson(len + value.length() + 128);
            valueJson[FPSTR(NAME_STR)] = setting->name();
            valueJson[FPSTR(VALUE_STR)] = value;
            array.add(valueJson);
        }

        return doc;
    }

    void SettingPanel::on_post(AsyncWebServerRequest *request)
    {
        for(auto &setting: settings)
        {
            String argName(get_identifier());
            argName += '$';
            argName += setting->name();
            if (request->hasArg(argName.c_str()))
            {
                setting->set_from_post(request->arg(argName));
            }
            else if (setting->send_to_ui())
            {
                setting->set_default();
            }
        }
    }
}
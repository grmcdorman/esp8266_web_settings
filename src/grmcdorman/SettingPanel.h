#pragma once

#include <ArduinoJson.h>
#include <WString.h>
#include <Stream.h>
#include <ESPAsyncWebServer.h>

#include "grmcdorman/Setting.h"

namespace grmcdorman
{
    /**
     * @brief The Setting Panel is the controller for a set of Setting.
     *
     * It manages reading the values in from
     * a POST request, and constructing the output JSON for the values when requested
     * by the UI.
     */
    class SettingPanel
    {
    public:
        /**
         * @brief Construct a new Setting Panel object.
         *
         * @param name          The setting panel name. This is used on UI elements.
         * @param identifier    The setting panel identifier. This is used in code.
         * @param settings_set  The set of settings for the panel. Held as a reference; do not destroy.
         */
        SettingPanel(const __FlashStringHelper *name, const __FlashStringHelper *identifier, const SettingInterface::settings_list_t &settings_set);
        /**
         * @brief Handle a POST set-values request.
         *
         * All settings that are updated from the incoming request; if a setting
         * does not appear, it is updated with its default (`set_default()`)
         * if it is a setting that is sent on requests (`send_to_ui()` is `true`).
         *
         * Passwords are the only present case where `send_to_ui()` is `false`,
         * meaning they are only set if explicitly included in the POST request.
         *
         * Note that some settings, notably note and info settings, ignore any attempt
         * to set a value from this call.
         * @param request
         */
        void on_post(AsyncWebServerRequest *request);
        /**
         * @brief Construct JSON containing all sendable settings.
         *
         * Settings that have `send_to_ui()` returing `false` will be omitted,
         * even if explicitly requested.
         *
         * The settings are inserted in the output document as an array under the key
         * containing the panel name.
         *
         * @param requested_settings    If not empty, include only the specific, named settings. Settings that do not exist are ignored.
         * @return JSON document containing all settings (if `specific_setting` is empty) or the requested settings that exist.
         */
        DynamicJsonDocument as_json(const std::vector<const String *> &requested_settings) const;
        /**
         * @brief Get the panel name.
         *
         * The name is used both as a label on the UI and as an ID for incoming fields in the POST request,
         * and for outgoing settings in the JSON.
         * @return Panel name.
         */
        const __FlashStringHelper * get_name() const
        {
            return name;
        }
        /**
         * @brief Get the name length.
         *
         * @return Length of the name, equivalent to String(get_name()).length() or similar.
         */
        size_t get_name_length() const
        {
            return name_length;
        }

        /**
         * @brief Get the panel identifier.
         *
         * The name is used both as a label on the UI and as an ID for incoming fields in the POST request,
         * and for outgoing settings in the JSON.
         * @return Panel name.
         */
        const __FlashStringHelper * get_identifier() const
        {
            return identifier;
        }

        /**
         * @brief Get the identifier length.
         *
         * @return Length of the identifier, equivalent to String(get_identifier()).length() or similar.
         */
        size_t get_identifier_length() const
        {
            return identifier_length;
        }

        /**
         * @brief Get the settings list.
         *
         * @return The wrapped settings list.
         */
        const SettingInterface::settings_list_t &get_settings() const
        {
            return settings;
        }
    private:
        const __FlashStringHelper * name;                   //!< The panel name, from the constructor.
        const size_t name_length;                           //!< The length of the name string.
        const __FlashStringHelper * identifier;             //!< The panel identifier, from the constructor.
        const size_t identifier_length;                           //!< The length of the identifier string.
        const SettingInterface::settings_list_t &settings;  //!< The set of settings contained in the panel.
    };
}

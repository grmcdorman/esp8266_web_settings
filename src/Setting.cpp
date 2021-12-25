#include "grmcdorman/Setting.h"
#include <Arduino.h>
#include <algorithm>

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace grmcdorman
{
    namespace {
        const char * PROGMEM DIV_STR = "<div>";
        auto DIV = FPSTR(DIV_STR);
        const char * PROGMEM NUMBER_STR = "number";
        auto NUMBER = FPSTR(NUMBER_STR);
    }
    SettingInterface::SettingInterface(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
        description(description),
        setting_name(setting_name)
    {
    }

    String SettingInterface::escape_value(const String &value) const
    {
        String escaped_value;
        escaped_value.reserve(value.length());
        for (const char ch: value)
        {
            switch (ch)
            {
            case '<':
                escaped_value += F("&lt;");
                break;
            case '>':
                escaped_value += F("&gt;");
                break;
            case '\"':
                escaped_value += F("&quot;");
                break;
            case '&':
                escaped_value += F("&amp;");
                break;
            default:
                escaped_value += ch;
                break;
            }
        }

        return escaped_value;
    }

    String SettingInterface::get_unique_id(const String &container_name) const
    {
        String result(container_name);
        result += '$';
        result += name();
        return result;
    }

    String SettingInterface::get_id_name_fields(const String &container_name) const
    {
        String id = get_unique_id(container_name);
        String result(static_cast<const char *>(nullptr));
        result.reserve(4 + // id=
                       2*id.length() +
                       8 + // " name="
                       1 // closing quote
        );
        result = F("id=\"");
        result += id;
        result += F("\" name=\"");
        result += std::move(id);
        result += '"';
        return result;
    }

    String SettingInterface::get_html_label(const String &container_name) const
    {
        String result(F("<label for=\""));
        result += get_unique_id(container_name);
        result += F("\">");
        result += get_description();
        result += F("</label>");
        return result;
    }

    String SettingInterface::get_make_html(const String &container_name, const String &element_html) const
    {
        String result(element_html);
        result += get_html_label(container_name);
        return result;
    }

    String SettingInterface::get_make_input(const __FlashStringHelper *type, const String &container_name, const String &value, const __FlashStringHelper *extra) const
    {
        // Note that while values are reloaded when the tab is selected,
        // initial values are required so that `save` doesn't get the wrong
        // data if the tab is never loaded.
        String result(F("<input type=\""));
        result += type;
        result += F("\" ");
        result += get_id_name_fields(container_name);
        if (extra != nullptr)
        {
            result += extra;
        }
        result += F(" />");
        result += get_html_label(container_name);
        return result;
    }

    String StringSetting::get_html(const String &container_name) const
    {
        return get_make_input(F("text"), container_name, escape_value(get()), nullptr);
    }

    String PasswordSetting::get_html(const String &container_name) const
    {
        // A password is very special.
        // In UI terms, the user can:
        //  * Not change the password.
        //  * Enter no password (password blank)
        //  * Enter a password
        // It may be adequate to have a checkbox saying 'change',
        // and the input field; a blank input means no password.
        // The solution, below, is to have the field disabled by default;
        // if checked, the field will become enabled, the user can enter values,
        // and it will be submitted.
        String result(F("<span class=\"password_group\">"));
        result += F("<input type=\"checkbox\" id=\"");
        result += container_name;
        result += F("$pw$");
        result += name();
        result += F("\" onchange='"
            "document.getElementById(\"");
        result += get_unique_id(container_name);
        result += F("\").disabled = !event.target.checked;"
            "'"
            "><input type=\"password\" ");
        result += get_id_name_fields(container_name);
        result += F(" disabled=\"true\"></span>");
        result += get_html_label(container_name);
        return result;
    }

    String SignedIntegerSetting::get_html(const String &container_name) const
    {
        return get_make_input(NUMBER, container_name, String(get()), nullptr);
    }

    String NoteSetting::get_html(const String &) const
    {
        String result(F("<div class=\"note\">"));
        result += get();
        static const char * PROGMEM DIV_END = "</div>";
        result += FPSTR(DIV_END);
        return result;
    }

    void SignedIntegerSetting::set_from_string(const String &new_value)
    {
        // n.b. does not handle invalid values in any meaningful way
        set(static_cast<value_type>(new_value.toInt()));
    }

    String UnsignedIntegerSetting::get_html(const String &container_name) const
    {
        return get_make_input(NUMBER, container_name, String(get()), F(" min=\"0\""));
    }

    void UnsignedIntegerSetting::set_from_string(const String &new_value)
    {
        // n.b. does not handle invalid values in any meaningful way
        set(static_cast<value_type>(std::strtoul(new_value.c_str(), nullptr, 10)));
    }

    String FloatSetting::get_html(const String &container_name) const
    {
        return get_make_input(NUMBER, container_name, String(get()), F(" step=\"0.1\""));
    }

    void FloatSetting::set_from_string(const String &new_value)
    {
        set(new_value.toFloat());
    }

    String ExclusiveOptionSetting::get_html(const String &container_name) const
    {
        String id(get_unique_id(container_name));
        String result(F("<select "));
        result += get_id_name_fields(container_name);
        result += F(" \">");
        uint16_t index = 0;
        for (const auto &option: names)
        {
            ++index;
            result += F("<option name=\"");
            result += id;
            result += F("_");
            result += index;
            result += F("\"");
            if (index == get() + 1)
            {
                result += F(" selected");
            }
            result += F(">");
            result += option;
            result += F("</option>");
        }
        result += F("</select>");
        result += get_html_label(container_name);
        return result;
   }

    void ExclusiveOptionSetting::set_from_string(const String &new_value)
    {
        auto where = std::find_if(names.begin(), names.end(), [&new_value] (const __FlashStringHelper *value)
            { return new_value == String(value); }
        );
        set(where != names.end() ? where - names.begin() : 0);
    }

    String ExclusiveOptionSetting::as_string() const
    {
        return names[std::max(static_cast<value_type>(0), std::min(static_cast<value_type>(names.size()), get()))];
    }

    String ToggleSetting::get_html(const String &container_name) const
    {
        return get_make_input(F("checkbox"), container_name, "", nullptr);
    }

    void ToggleSetting::set_from_string(const String &new_value)
    {
        set(strcasecmp(new_value.c_str(), "true") == 0 ||
            strcasecmp(new_value.c_str(), "on") == 0 ||
            new_value == "1");
    }

    String InfoSettingHtml::get_html(const String &container_name) const
    {
        String result(F("<span class=\"info\" "));
        result += get_id_name_fields(container_name);
        result += '>';
        result += F("</span>");
        result += get_html_label(container_name);
        return result;
   }

    String InfoSettingHtml::as_string() const
    {
        if (request_callback != nullptr)
        {
            request_callback(*this);
        }
        return get();
    }
}
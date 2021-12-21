#include "grmcdorman/Setting.h"
#include <Arduino.h>
#include <algorithm>

namespace grmcdorman
{
    SettingInterface::SettingInterface(const String & description, const String &setting_name):
        description(description),
        generated_name(setting_name)
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
                escaped_value += "&lt;";
                break;
            case '>':
                escaped_value += "&gt;";
                break;
            case '\"':
                escaped_value += "&quot;";
                break;
            case '&':
                escaped_value += "&amp;";
                break;
            default:
                escaped_value += ch;
                break;
            }
        }

        return escaped_value;
    }

    String SettingInterface::html_label(const String &input_name) const
    {
        return "<LABEL FOR=\"" + input_name + "\">" + get_description() + "</LABEL>";
    }

    String SettingInterface::make_html(const String &input_name, const String &element_html) const
    {
        return
            "<TR><TD>" +
            html_label(input_name) +
            "</TD><TD>" +
            element_html +
            "</TD></TR>\n";
    }

    String SettingInterface::make_input(const char *type, const String &input_name, const String &value, const String &extra) const
    {
        // Note that while values are reloaded when the tab is selected,
        // initial values are required so that `save` doesn't get the wrong
        // data if the tab is never loaded.
        return make_html(input_name,
            "<INPUT TYPE=\"" +
            String(type) +
            "\" NAME=\"" +
            input_name +
            "\" ID=\"" +
            input_name +
            "\" VALUE=\"" +
            escape_value(as_string()) +
            "\" " +
            extra +
            " />");
    }

    String StringSetting::html(const String &container_name) const
    {
        return make_input("TEXT", container_name + "$" + name(), escape_value(get()), " SIZE=\"45\"");
    }

    String PasswordSetting::html(const String &container_name) const
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
        String id = container_name + "$" + name();
        String html("<INPUT TYPE=\"CHECKBOX\" ID=\"");
        html += container_name + "@changepassword$" + name();
        html += "\" onchange='"
            "document.getElementById(\"" +  id + "\").disabled = !event.target.checked;"
            "'";
        html += "><INPUT TYPE=\"PASSWORD\" ID=\"" + id + "\" NAME=\"" + id + "\" DISABLED=\"TRUE\" SIZE=\"45\">";
        return make_html(id, html);
    }

    String SignedIntegerSetting::html(const String &container_name) const
    {
        return make_input("NUMBER", container_name + "$" + name(), String(get()), String());
    }

    String NoteSetting::html(const String &) const
    {
        return "<TR><TD COLSPAN=\"2\">" + get() + "</TD></TR>\n";
    }

    void SignedIntegerSetting::set_from_string(const String &new_value)
    {
        // n.b. does not handle invalid values in any meaningful way
        set(static_cast<value_type>(new_value.toInt()));
    }

    String UnsignedIntegerSetting::html(const String &container_name) const
    {
        return make_input("NUMBER", container_name + "$" + name(), String(get()), " MIN=\"0\"");
    }

    void UnsignedIntegerSetting::set_from_string(const String &new_value)
    {
        // n.b. does not handle invalid values in any meaningful way
        set(static_cast<value_type>(new_value.toInt()));
    }

    String FloatSetting::html(const String &container_name) const
    {
        return make_input("NUMBER", container_name + "$" + name(), String(get()));
    }

    void FloatSetting::set_from_string(const String &new_value)
    {
        set(new_value.toFloat());
    }

    String ExclusiveOptionSetting::html(const String &container_name) const
    {
        String result;
        String input_name = container_name + "$" + name();
        size_t reserve = 29 + input_name.length(); // size for <SELECT NAME="NAME"></SELECT>"
        for(const auto &option: names)
        {
            reserve += option.length() + input_name.length() + 3 +  26; // <option name="name_n">value</option>
        }
        result.reserve(reserve);
        result += "<SELECT NAME=\"" + input_name + "\" ID=\"" + input_name + "\">";
        uint16_t index = 0;
        for (const auto &option: names)
        {
            ++index;
            result += "<OPTION NAME=\"" + input_name + "_" + String(index) + "\"";
            if (index == get() + 1)
            {
                result += " SELECTED";
            }
            result += ">" + option + "</OPTION>";
        }
        result += "</SELECT>";
        return make_html(input_name, result);
    }

    void ExclusiveOptionSetting::set_from_string(const String &new_value)
    {
        auto where = std::find(names.begin(), names.end(), new_value);
        set(where != names.end() ? where - names.begin() : 0);
    }

    String ExclusiveOptionSetting::as_string() const
    {
        return names[std::max(static_cast<value_type>(0), std::min(static_cast<value_type>(names.size()), get()))];
    }

    String ToggleSetting::html(const String &container_name) const
    {
        return make_input("CHECKBOX", container_name + "$" + name(), "", get() ? " CHECKED" : "");
    }

    void ToggleSetting::set_from_string(const String &new_value)
    {
        String copy(new_value);
        copy.toLowerCase();
        set(new_value == "1" || copy == "true" || copy == "on");
    }
}
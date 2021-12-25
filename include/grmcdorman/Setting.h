#pragma once

#include <functional>
#include <vector>

#include <pgmspace.h>
#include <WString.h>

namespace grmcdorman
{
    /**
     * @brief The generic settings interface.
     *
     * This class provides a common interface for all settings.
     */
    class SettingInterface
    {
    public:
    protected:
        /**
         * @brief Construct a new Setting Interface object
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        SettingInterface(const __FlashStringHelper *description, const __FlashStringHelper *setting_name);
    public:
        typedef std::vector<SettingInterface *> settings_list_t;    //!< The container for a list of settings.
        /**
         * @brief Stream the HTML fragment for the setting.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return HTML for the setting.
         */
        virtual String get_html(const String &container_name) const = 0;
        /**
         * @brief Set the value from a string.
         *
         * Converts the value to the appropriate type and stores the value. Errors
         * are not detected or reported; the result of setting an invalid value
         * is typically undefined.
         *
         * @param new_value New value, as a string.
         */
        virtual void set_from_string(const String &new_value) = 0;

        /**
         * @brief Set the value from an HTML Post string.
         *
         * In most cases, this is the same as setting from a string;
         * it is provided to allow special-case handling, notably
         * checkbox data.
         *
         * @param new_value New value, as a string.
         */
        virtual void set_from_post(const String &new_value)
        {
            set_from_string(new_value);
        }
        /**
         * @brief Set to the default.
         *
         * This will typically be the value type's default,
         * e.g. `bool()` or `int()`.
         */
        virtual void set_default() = 0;
        /**
         * @brief Get the setting's name.
         *
         * This name is from the constructor, and should be
         * unique within its container.
         *
         * @return Unique setting name.
         */
        const __FlashStringHelper *name() const
        {
            return FPSTR(setting_name);
        }
        /**
         * @brief Get the description string.
         *
         * This is the description string provided to the constructor.
         *
         * @return Description string.
         */
        const __FlashStringHelper *get_description() const
        {
            return FPSTR(description);
        }
        /**
         * @brief Get the label HTML fragment for this setting.
         *
         * The label contains the description text. It is formatted
         * with a `FOR` field referring to the input or selection element
         * returned by the `html` method.
         *
         * @param input_name    The unique (system-wide) name for the input element.
         * @return HTML label.
         */
        String get_html_label(const String &input_name) const;
        /**
         * @brief Get the value as a string.
         *
         * This converts the value to a string using default formatting
         * rules where applicable (for example, floating-point will typically
         * have two decimal digits).
         *
         * @return Value, as a string.
         */
        virtual String as_string() const = 0;

        /**
         * @brief Whether to send the value to the UI on request.
         *
         * Password settings, and other settings with sensitive values,
         * should return `false` for this field so that they will not be
         * transmitted to the UI.
         *
         * @return true     Send the value to the UI.
         * @return false    Do not send the value to the UI.
         */
        virtual bool send_to_ui() const
        {
            return true;
        }

        /**
         * @brief Whether to persist this setting in flash.
         *
         * Some settings should not be saved, notably notes & info.
         *
         * @return true     if this setting should be saved
         */
        virtual bool is_persistable() const
        {
            return true;
        }

    protected:
        /**
         * @brief Output an INPUT field and a LABEL field.
         *
         * This constructs a full INPUT field of the given type. The
         * output HTML includes the constructed input field, followed
         * by the label for the field.
         *
         * @note Will produce syntatically correct but unusable output if
         * the setting's name is blank.
         *
         * @param type              The INPUT type, e.g. NUMBER.
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @param value             The initial value.
         * @param extra             Additional fields. `nullptr` if no fields are to be provided.
         * @return Constructed INPUT field.
         */
        String get_make_input(const __FlashStringHelper *type, const String &container_name, const String &value, const __FlashStringHelper * extra) const;
        /**
         * @brief Output the HTML for an arbitrary input and label.
         *
         * This outputs the text in `setting_html`, verbatim, followed by a LABEL field.
         *
         * @note Will produce syntatically correct but unusable output if
         * the setting's name is blank.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @param setting_html      The setting HTML; for example a SELECT list.
         * @return Constructed HTML.
         */
        String get_make_html(const String &container_name, const String &setting_html) const;
        /**
         * @brief Stream out the unique control ID.
         *
         * This streams out the container name, a '$', and the setting name. It
         * is used in other context where the control ID is required.
         *
         * @note Will produce syntatically correct but unusable output if
         * the setting's name is blank.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return HTML text.
         */
        String get_unique_id(const String &container_name) const;
        /**
         * @brief Return `id=` and `name=` attributes.
         *
         * @note Will produce syntatically correct but unusable output if
         * the setting's name is blank.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return The two fields.
         */
        String get_id_name_fields(const String &container_name) const;
        /**
         * @brief Escape a value appropriately for an input value field.
         *
         * @param value     Raw value.
         * @return Escaped value.
         */
        String escape_value(const String &value) const;
    private:
        const __FlashStringHelper *description;   //!< The description from the constructor.
        const __FlashStringHelper *setting_name;  //!< The name from the constructor.
    };

    /**
     * @brief A templated generic setting.
     *
     * This wraps a specific type. Some basic operations are filled in.
     * The contained value is initialized to the type's default constructor;
     * this is typically `0` for integer and floating point types, and `false` for
     * booleans.
     *
     * @tparam T Type to wrap.
     */
    template<typename T>
    class GenericSetting: public SettingInterface
    {
    public:
        /**
         * @brief Construct a new Generic Setting object
         *
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        GenericSetting(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
            SettingInterface(description, setting_name), value()
        {
        }

        typedef T value_type;   //!< The type of the wrapped value.

        /**
         * @brief Get the value.
         *
         * @return A const ref to the contained value.
         */
        const T &get() const
        {
            return value;
        }

        /**
         * @brief Set the value.
         *
         * @param new_value The new value.
         */
        void set(const T &new_value)
        {
            value = new_value;
        }

        /**
         * @brief Convert the value to a string.
         *
         * This returns the string representation of the value;
         * for most types this uses the default `String` class
         * to perform the conversion.
         *
         * @return Value, as a string.
         */
        String as_string() const override
        {
            return String(value);
        }

        /**
         * @brief Set to the default value.
         *
         * This sets the value to the type's default value.
         */
        void set_default() override
        {
            value = T();
        }
    protected:
        T value;    //!< The contained value.
    };

    /**
     * @brief A note setting.
     *
     * A note is a read-only setting that spans both columns of the
     * setting table. The text of the note is not escaped and can
     * contain HTML code, including Javascript. Because it is read-only,
     * no container name or setting name is used, and no identifier is set into the
     * constructed HTML.
     *
     * The note text can be changed after construction; this may not have any
     * effect on the web page.
     */
    class NoteSetting: public GenericSetting<const __FlashStringHelper *>
    {
    public:
        /**
         * @brief Construct a new Note Setting object
         *
         * @param value         The note value. Can contain HTML.
         */
        explicit NoteSetting(const __FlashStringHelper *value):
            GenericSetting(F("not used"), F(""))
        {
            set(value);
        }

        /**
         * @brief Return the HTML.
         *
         * In this case, it is simply the note value. No interpolation
         * or other processing is performed on the HTML.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return HTML text.
         */
        String get_html(const String &container_name) const override;

        /**
         * @brief Set the note value.
         *
         * For a note, this performs no operation; the note is not changed.
         *
         * @param new_value The new value; not used.
         */
        void set_from_string(const String &new_value) override
        {
            // This does not do anything. The string value
            // is the HTML note.
        }

        /**
         * @brief Whether to persist this setting in flash.
         *
         * For notes, this always returns `false`.
         * @return `false`: do not save the note in flash.
         */
        bool is_persistable() const override
        {
            return false;
        }
    };

    /**
     * @brief A string setting.
     *
     * This is a generic string setting. No limitations are placed upon the
     * input.
     */
    class StringSetting: public GenericSetting<String>
    {
    public:
        /**
         * @brief Construct a new String Setting object
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        StringSetting(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
            GenericSetting(description, setting_name)
        {
        }
        /**
         * @brief Return the HTML for the setting.
         *
         * For a string setting, this will be of the form `<INPUT TYPE="TEXT">`.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return HTML text..
         */
        String get_html(const String &container_name) const override;
        /**
         * @brief Set the value from a string.
         *
         * In this case, no interpretation is applied; the value is
         * set directly from the supplied string.
         *
         * @param new_value New string value.
         */
        void set_from_string(const String &new_value) override
        {
            set(new_value);
        }
    };

    /**
     * @brief A password setting.
     *
     * This is functionally similar to a string setting, except that
     * the field is flagged as a password, so that it is protected.
     * In addition, the password is never sent to the UI; in the UI,
     * the user must enable a toggle to indicate the password is
     * being entered. This allows blank passwords if necessary.
     *
     * If the toggle is not checked, the password input field is
     * disabled and is not sent by the browser; hence any saved
     * password is not changed.
     *
     */
    class PasswordSetting: public StringSetting
    {
    public:
        /**
         * @brief Construct a new Password Setting.
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        PasswordSetting(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
            StringSetting(description, setting_name)
        {
        }

        /**
         * @brief Whether to send the value to the UI on request.
         *
         * For a password setting, this will always return `false`.
         *
         * @return false    Do not send the value to the UI.
         */
        bool send_to_ui() const override
        {
            return false;
        }
        String get_html(const String &container_name) const override;
    };

    /**
     * @brief A signed integer setting.
     *
     * This is a 32-bit integer with no other restrictions on the value.
     *
     */
    class SignedIntegerSetting: public GenericSetting<int32_t>
    {
    public:
        /**
         * @brief Construct a new Signed Integer Setting object
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        SignedIntegerSetting(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
            GenericSetting(description, setting_name)
        {
        }
        /**
         * @brief Stream the HTML for the setting.
         *
         * For a signed integer, this will be a simple input of the form `<INPUT TYPE="NUMBER">`.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return HTML text.
         */
        String get_html(const String &container_name) const override;
        /**
         * @brief Set the value from a string.
         *
         * This does not report validation errors; non-numeric values
         * will simply result in a zero being stored.
         *
         * @param new_value New value, as a string.
         */
        void set_from_string(const String &new_value) override;
    };

    /**
     * @brief An unsigned integer setting.
     *
     * Identical to a signed integer, save that the input field is given a minimum value of 0.
     *
     */
    class UnsignedIntegerSetting: public GenericSetting<uint32_t>
    {
    public:
        /**
         * @brief Construct a new Unsigned Integer Setting object
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        UnsignedIntegerSetting(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
            GenericSetting(description, setting_name)
        {
        }
        /**
         * @brief Stream the HTML for the setting.
         *
         * For an unsigned integer, this will be a simple input of the form `<INPUT TYPE="NUMBER" MAX="0">`.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return HTML text.
         */
        String get_html(const String &container_name) const override;
        /**
         * @brief Set the value from a string.
         *
         * This does not report validation errors; non-numeric values
         * will simply result in a zero being stored. Too-large values
         * will also result in invalid values.
         *
         * @param new_value New value, as a string.
         */
        void set_from_string(const String &new_value) override;
    };

    /**
     * @brief A floating-point setting.
     *
     * From an HTML standpoint, identical to a signed integer, save that the input field is given a step of 0.1,
     * allowing one digit of precision.
     *
     */
    class FloatSetting: public GenericSetting<float>
    {
    public:
        /**
         * @brief Construct a new Float Setting object
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        FloatSetting(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
            GenericSetting(description, setting_name)
        {
        }
        /**
         * @brief Stream the HTML for the setting.
         *
         * For a floating point, this will be a simple input of the form `<INPUT TYPE="NUMBER" STEP="0.1">`.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return HTML text.
         */
        String get_html(const String &container_name) const override;
        /**
         * @brief Set the value from a string.
         *
         * This does not report validation errors; non-numeric values
         * will simply result in a zero being stored.
         *
         * @param new_value New value, as a string.
         */
        void set_from_string(const String &new_value) override;
    };

    /**
     * @brief A set of exclusive options.
     *
     * This is presented as a drop-down list, a.k.a. a combobox.
     *
     */
    class ExclusiveOptionSetting: public GenericSetting<uint16_t>
    {
    public:
        typedef std::vector<const __FlashStringHelper *> names_list_t;  //!< The container for a list of option names.
        /**
         * @brief Construct a new Exclusive Option Setting object.
         *
         * @param description   Description for the setting. Can include HTML.
         * @param setting_name  The name for the setting. Must be identifier-like.
         * @param optionNames   The set of option names. Can include HTML.
         */
        ExclusiveOptionSetting(const __FlashStringHelper *description, const __FlashStringHelper *setting_name,
            const names_list_t &optionNames):
            GenericSetting(description, setting_name), names(optionNames)
        {
        }

        /**
         * @brief Stream the exclusive-setting HTML.
         *
         * This constructs a `SELECT` HTML with the option list.
         *
         * Individual `OPTION` fields are given a `NAME` value of the container name,
         * a `$`, the setting name, an underscore, and an index starting from 1. For example, if the
         * container name is 'DH11', and the setting is 'pin', then the option names will
         * be `DH11$pin_1` and up.
         *
         * @note Option names are not escaped; it is possible to include HTML
         * in the name.
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return The constructed HTML.
         */
        String get_html(const String &container_name) const override; // dropdown or radio buttons
        /**
         * @brief Set the option from a string value.
         *
         * The value is expected to be one of the option names. Setting to a name
         * that does not exist will result in the first option being selected.
         *
         * @param new_value     New value; an option name.
         */
        void set_from_string(const String &new_value) override;
        /**
         * @brief Return the option, as a string.
         *
         * Returns the option name. Use `get()` to retrieve the index.
         *
         * @return Selected option name.
         */
        String as_string() const override;
    private:
        const names_list_t &names;      //!< The option names.
    };

    /**
     * @brief A toggle setting, otherwise known as a checkbox.
     *
     */
    class ToggleSetting: public GenericSetting<bool>
    {
    public:
        /**
         * @brief Construct a new Toggle Setting object
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        ToggleSetting(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
            GenericSetting(description, setting_name)
        {
        }
        /**
         * @brief Stream the toggle HTML.
         *
         * For a toggle, this will be of the form `<INPUT TYPE='CHECKBOX'>`.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return The constructed HTML.
         */
        String get_html(const String &container_name) const override;
        /**
         * @brief Set the value from a string.
         *
         * The string is treated as 'truthy': values of '1', 'true', 'on'
         * are treated as true, and other values as false.
         *
         * @param new_value New value, as a "truthy" string.
         */
        void set_from_string(const String &new_value) override;
        /**
         * @brief Set the value from an HTML Post string.
         *
         *  For toggles, this always sets the toggle to `true`,
         * as, in HTML forms, the mere presence of the toggle
         * in the form data means it's checked.
         *
         * The parameter is not used.
         */
        void set_from_post(const String &) override
        {
            set(true);
        }

        /**
         * @brief Return the toggle, as a string.
         *
         * This will return "1" for `true` and "0" for `false`.
         * @return String
         */

        String as_string() const override
        {
            return String(get() ? '1': '0');
        }
    };

    /**
     * @brief An info setting is a read-only string setting.
     *
     * On the UI, it is presented with the value simply
     * output as-is (no HTML encoding). The value
     * will be updated when values are fetched from
     * the server.
     *
     * The Web Server provides a mechanism for periodic updates;
     * in JavaScript, adding the tab name, or the tab name, an ampersand, and
     * the string "setting=" followed by the setting name to the array `periodicUpdateList` will result in
     * the value being fetched approximately every 5 seconds. For example,
     * if the tab is "Overview" and the setting is "uptime", then:
     *  * `window.addEventListener(\"load\", () => { periodicUpdateList.push("Overview"); });` will update all fields on the tab periodically;
     *  * `window.addEventListener(\"load\", () => { periodicUpdateList.push("Overview&setting=uptime"); });` will fetch and update the "uptime" field periodically.
     *
     * Avoid updating an entire tab if it contains input fields; the update will refresh the input
     * fields as well as the other fields.
     *
     * The value will also be requested on initial page load, and when the user clicks "Reset Form".
     */
    class InfoSettingHtml: public StringSetting
    {
    public:
        /**
         * @brief Construct a new Info Setting Html object
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        InfoSettingHtml(const __FlashStringHelper *description, const __FlashStringHelper *setting_name):
            StringSetting(description, setting_name), request_callback(nullptr)
        {
        }

        /**
         * @brief Set the request callback.
         *
         * The request callback is invoked just before the setting's value is provided
         * to a request by the UI. The callback can update the setting if necessary.
         *
         * A value of `nullptr` can be used to disable the callback.
         *
         * @param callback  Callback function to invoke before sending the value to UI.
         */
        void set_request_callback(const std::function<void(const InfoSettingHtml &)>& callback)
        {
            request_callback = callback;
        }
        /**
         * @brief Stream the info HTML.
         *
         * For an info setting, this will be a SPAN containing the value, and
         * a LABEL field for the SPAN.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return The constructed HTML.
         */
        String get_html(const String &container_name) const override;
        /**
         * @brief Set the value from an HTML Post string.
         *
         * For info settings, this is ignored; the setting cannot be changed
         * from HTML form submissions.
         *
         * The parameter is not used.
         */
        void set_from_post(const String &) override
        {
            // Ignored.
        }
        /**
         * @brief Set to the default.
         *
         * For info settings, this is ignored.
         *
         */
        void set_default() override
        {
            // Ignored. This is not sent from the front-end, but is set _to_ the front-end.
        }

        String as_string() const override;

        /**
         * @brief Whether to persist this setting in flash.
         *
         * For info settings, this always returns `false`.
         *
         * @return `false`: do not save the info setting in flash.
         */
        bool is_persistable() const override
        {
            return false;
        }
   private:
        std::function<void(const InfoSettingHtml &)> request_callback;
    };
}
#pragma once

#include <WString.h>
#include <vector>

namespace grmcdorman
{
    /**
     * @brief The generic settings interface.
     *
     * This class provides a common interface for all settings.
     */
    class SettingInterface
    {
    protected:
        /**
         * @brief Construct a new Setting Interface object
         *
         * @param description       The setting description. This is interpreted as HTML; format appropriately.
         * @param setting_name      The unique setting name. This must be unique for the container. It can be empty for notes.
         */
        SettingInterface(const String &description, const String &setting_name);
    public:
        typedef std::vector<SettingInterface *> settings_list_t;
        /**
         * @brief Return the HTML fragment for this setting.
         *
         * @param container_name    The unique container name (system -wide). Used to generate a unique field identifier.
         * @return String containing the HTML fragment.
         */
        virtual String html(const String &container_name) const = 0;
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
         * @brief Get the setting's unique name.
         *
         * This name is constructed from the container name and setting name.
         *.
         * @return Unique setting name.
         */
        const String &name() const
        {
            return generated_name;
        }
        /**
         * @brief Get the description string.
         *
         * This is the description string provided to the constructor.
         *
         * @return Description string.
         */
        const String &get_description() const
        {
            return description;
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
        String html_label(const String &input_name) const;
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
    protected:
        /**
         * @brief Make a label and an INPUT field.
         *
         * This constructs a full INPUT field of the given type. The
         * returned HTML includes the `html_label` followed by the constructed
         * input field.
         *
         * @param type          The INPUT type, e.g. NUMBER.
         * @param input_name    The unique (system-wide) name for the input element.
         * @param value         The initial value.
         * @param extra         Additional fields.
         * @return The input field.
         */
        String make_input(const char *type, const String &input_name, const String &value, const String &extra = String()) const;
        /**
         * @brief Make the HTML for a label and arbitrary input.
         *
         * This returns the `html_label` followed by the provided text
         * in `setting_html`, verbatim.
         *
         * @param input_name    The unique (system-wide) name for the input element.
         * @param setting_html  The setting HTML; for example a SELECT list.
         * @return The label followed by the setting html.
         */
        String make_html(const String &input_name, const String &setting_html) const;
        /**
         * @brief Escape a value appropriately for an input value field.
         *
         * @param value     Raw value.
         * @return Escaped value.
         */
        String escape_value(const String &value) const;
    private:
        String description;         /// The description string from the constructor.
        String generated_name;      /// The assigned unique name.
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
        GenericSetting(const String &description, const String &setting_name):
            SettingInterface(description, setting_name), value()
        {
        }

        typedef T value_type;   /// The type of the wrapped value.

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
    private:
        T value;    /// The contained value.
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
    class NoteSetting: public GenericSetting<String>
    {
    public:
        /**
         * @brief Construct a new Note Setting object
         *
         * @param value         The note value. Can contain HTML.
         */
        explicit NoteSetting(const String &value):
            GenericSetting("not used", String())
        {
            set(value);
        }

        /**
         * @brief Return the HTML.
         *
         * In this case, it is simply the note value.
         * @param container_name    The container name. Not used.
         * @return The note text.
         */
        String html(const String &container_name) const override;

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
        StringSetting(const String &description, const String &setting_name):
            GenericSetting(description, setting_name)
        {
        }
        /**
         * @brief Return the HTML for the setting.
         *
         * For a string setting, this will be `<INPUT TYPE="TEXT">`.
         *
         * @param container_name    The container name. Used to construct a unique field ID.
         * @return The HTML for a string input.
         */
        String html(const String &container_name) const override;
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
     *
     * @note The password is still sent to the web page; anyone who
     * can access the page will be able to retrieve the password.
     * In the future, this may be changed to provide broader
     * controls (e.g. a 'change password' toggle and two password inputs,
     * one for verification). External semantics may remain the same.
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
        PasswordSetting(const String &description, const String &setting_name):
            StringSetting(description, setting_name)
        {
        }
        bool send_to_ui() const override
        {
            return false;
        }
        String html(const String &container_name) const override;
    };

    class SignedIntegerSetting: public GenericSetting<int32_t>
    {
    public:
        SignedIntegerSetting(const String &description, const String &setting_name):
            GenericSetting(description, setting_name)
        {
        }
        String html(const String &container_name) const override;
        void set_from_string(const String &new_value) override;
    };

    class UnsignedIntegerSetting: public GenericSetting<uint32_t>
    {
    public:
        UnsignedIntegerSetting(const String &description, const String &setting_name):
            GenericSetting(description, setting_name)
        {
        }
        String html(const String &container_name) const override;
        void set_from_string(const String &new_value) override;
    };

    class FloatSetting: public GenericSetting<float>
    {
    public:
        FloatSetting(const String &description, const String &setting_name):
            GenericSetting(description, setting_name)
        {
        }
        String html(const String &container_name) const override;
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
        /**
         * @brief Construct a new Exclusive Option Setting object.
         *
         * @param description   Description for the setting. Can include HTML.
         * @param setting_name  The name for the setting. Must be identifier-like.
         * @param optionNames   The set of option names. Can include HTML.
         */
        ExclusiveOptionSetting(const String &description, const String &setting_name,
            const std::vector<String> &optionNames):
            GenericSetting(description, setting_name), names(optionNames)
        {
        }

        /**
         * @brief Return the exclusive-setting HTML.
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
         * @param container_name    Container name; used as a prefix for the SELECT ID.
         * @return The constructed HTML.
         */
        String html(const String &container_name) const override; // dropdown or radio buttons
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
        std::vector<String> names;
    };

    class ToggleSetting: public GenericSetting<bool>
    {
    public:
        ToggleSetting(const String &description, const String &setting_name):
            GenericSetting(description, setting_name)
        {
        }
        String html(const String &container_name) const override;
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
         * @brief Set the value from an HTML post.
         *
         * For toggles, _any_ value set - regardless of contents -
         * is `true`, and the field will be missing when `false`
         * (meaning `set_default` gets called instead).
         *
         */
        void set_from_post(const String &) override
        {
            set(true);
        }

        String as_string() const override
        {
            return get() ? "1": "0";
        }
    };

    class InfoSetting: public GenericSetting<String>
    {

    };
}

#include "grmcdorman/WebSettings.h"

#include <AsyncJson.h>
#include <LittleFS.h>
#include <WebAuthentication.h>

#include "grmcdorman/SettingPanel.h"

namespace grmcdorman
{
    namespace
    {
        const char * PROGMEM TEXT_HMTL_STR = "text/html";
        auto TEXT_HTML = FPSTR(TEXT_HMTL_STR);
        const char * PROGMEM TEXT_JSON_STR = "text/json";
        auto TEXT_JSON = FPSTR(TEXT_JSON_STR);
        const char * PROGMEM TEXT_PLAIN_STR = "text/plain";
        auto TEXT_PLAIN = FPSTR(TEXT_PLAIN_STR);
        const char * PROGMEM TEXT_END_DIV_STR = "</div>";
        auto TEXT_END_DIV = FPSTR(TEXT_END_DIV_STR);
        const char * PROGMEM status_div = "<div class=\"status\">";
        //!< The style sheet. This could be stored gzipp'd to save space
        //!< and sent to the client that way.
        const char style[] PROGMEM =
            ".tab {"
                "overflow: hidden;"
                "border: 1px solid #ccc;"
                "background-color: #f1f1f1;"
            "}"
            ".tab button {"
                "background-color: inherit;"
                "float: left;"
                "border: none;"
                "outline: none;"
                "cursor: pointer;"
                "padding: 14px 16px;"
                "transition: 0.3s;"
                "font-size: 12pt;"
            "}"
            ".tab button:hover {"
                "background-color: #ddd;"
            "}"
            ".tab button.active {"
                "background-color: #ccc;"
            "}"

            ".md_button {"
                "margin-top: -30px;"
                "position: relative;"
                "overflow: hidden;"
                "-webkit-transition: background 400ms;"
                "transition: background 400ms;"
                "color: #fff;"
                "background-color: #0066ff;"
                "padding: 0.25em 0.5em;"
                "font-family: 'Roboto', sans-serif;"
                "font-size: 1rem;"
                "outline: 0;"
                "border: 0;"
                "border-radius: 0.25rem;"
                "-webkit-box-shadow: 0 0 0.5rem rgba(0, 0, 0, 0.3);"
                "box-shadow: 0 0 0.5rem rgba(0, 0, 0, 0.3);"
                "cursor: pointer;"
                "margin: 0.25em;"
                "box-sizing: content-box;"
                "text-decoration:none;"
            "}"

            ".ripple {"
                "background-position: center;"
                "-webkit-transition: background 0.8s;"
                "transition: background 0.8s;"
            "}"

            ".ripple:hover {"
                "background: #3385ff radial-gradient(circle, transparent 1%, #3385ff 1%) center/15000%;"
            "}"

            ".ripple:active {"
                "background-color: #4d94ff;"
                "background-size: 100%;"
                "-webkit-transition: background 0s;"
                "transition: background 0s;"
            "}"

            ".md_button.red {"
                "background-color: #ff3300;"
            "}"
            ".ripple.red:hover {"
                "background: #ff3300 radial-gradient(circle, transparent 1%, #ff3300 1%) center/15000%;"
            "}"
            ".ripple.red:active {"
                "background-color: #ff5c33;"
            "}"

            ".status {"
                "padding:20px;"
                "margin:20px 0;"
                "border:1px solid #eee;"
                "border-left-width:5px;"
                "border-left-color:#777;"
            "}"

            ".disable_overlay {"
                "position: fixed;"
                "top: 0;"
                "right: 0;"
                "bottom: 0;"
                "left: 0;"
                "background-color:#000;"
                "opacity: .75;"
                "z-index: 9999999;"
                "display: none;"
            "}"

            ".tabcontent {"
                "display: none;"
                "padding: 6px 12px;"
                "border: 1px solid #ccc;"
                "border-top: none;"
                "margin-left: auto;"
                "margin-right: auto;"
            "}"
            ".tabcontent > input, .tabcontent > select, .tabcontent > span {"
                "clear: both;"
                "float: right;"
                "width: 70%;"
            "}"
            ".tabcontent > label {"
                "float: left;"
                "width: 25%;"
                "text-align: right;"
                "padding: 0.25em 1em 0 0;"
            "}"
            ".tabcontent > input[type=\"checkbox\"] {"
                "width: auto;"
                "float: left;"
                "margin: 0.5em 0.5em 0 30%;"
            "}"
            ".tabcontent > input[type=\"checkbox\"] + label {"
                "width: auto;"
                "text-align: left;"
            "}"
            ".password_group > input[type=\"checkbox\"] {"
                "float: left;"
            "}"
            ".password_group > input[type=\"password\"] {"
                "width: calc(100% - 35px);"
                "float: right;"
            "}"
            ".tabcontent.active {"
                "display: block;"
            "}"
            "@supports (display: grid) {"
                ".tabcontent.active {"
                    "display: grid;"
                "}"

                ".tabcontent {"
                    "grid-template-columns: 1fr 1em 3fr;"
                    "grid-gap: 0.3em 0.6em;"
                    "grid-auto-flow: dense;"
                    "align-items: center;"
                "}"

                ".tabcontent > div {"
                    "grid-column: 1 /4;"
                    "width: auto;"
                    "margin: 0;"
                "}"
                ".tabcontent > input, .tabcontent > select, .tabcontent > span {"
                    "grid-column: 2 / 4;"
                    "width: auto;"
                    "margin: 0;"
                "}"

                ".tabcontent > select {"
                    "margin-right: auto;"
                "}"

                ".tabcontent > input[type=\"checkbox\"] {"
                    "grid-column: 1 / 3;"
                    "justify-self: end;"
                    "margin: 0;"
                "}"

                ".tabcontent > label, .tabcontent > input[type=\"checkbox\"] + label {"
                    "width: auto;"
                    "padding: 0;"
                    "margin: 0;"
                "}"
            "}"
        ;

        //!< The Javascript sheet. This could be stored gzipp'd to save space
        //!< and sent to the client that way. At the moment this includes newlines
        //!< to allow debugging in the browser.
        const char javascript_text[] PROGMEM =
            "function openTab(evt, tabName) {\n"
                "var i, tabcontent, tablinks;\n"
                "tabcontent = document.getElementsByClassName(\"tabcontent\");\n"
                "for (i = 0; i < tabcontent.length; i++) {\n"
                    "tabcontent[i].className = tabcontent[i].className.replace(\" active\", \"\").replace(\" hidden\", \"\") +"\
                        "(tabcontent[i].id === tabName ? \" active\" : \" hidden\");\n"
                "}\n"
                "tablinks = document.getElementsByClassName(\"tablinks\");\n"
                "for (i = 0; i < tablinks.length; i++) {\n"
                    "tablinks[i].className = tablinks[i].className.replace(\" active\", \"\");\n"
                "}\n"
                "evt.currentTarget.className += \" active\";\n"
            "}\n"

            "var globalTabsToLoad = [];\n"
            "function reloadAllTabs() {\n"
                "var tabcontent = document.getElementsByClassName(\"tabcontent\");\n"
                "for (i = 0; i < tabcontent.length; i++) {\n"
                    "globalTabsToLoad.push(tabcontent[i].id);\n"
                "}\n"
                "loadNextTab();\n"
            "}\n"

            "function loadNextTab() {\n"
                "if (globalTabsToLoad.length === 0)\n"
                "{"
                    "return;"
                "}"
                "var tabToLoad = globalTabsToLoad.pop();"
                "var req = new XMLHttpRequest();\n"
                "req.overrideMimeType(\"application/json\");\n"
                "req.open(\"GET\", \"/settings/get?tab=\" + tabToLoad, true);\n"
                "req.onload = handleSettingsGet;\n"
                "req.send(null);\n"
            "}\n"

            "function handleSettingsGet() {\n"
                "var r = JSON.parse(this.responseText),\n"
                    "k = Object.keys(r);\n"
                "for (var i = 0; i < k.length; ++i)\n"
                "{"
                    "for (var j = 0; j < r[k[i]].length; ++j)\n"
                    "{"
                        "setControlValue(k[i], r[k[i]][j]);\n"
                    "}\n"
                "}\n"
                "setTimeout(loadNextTab, 500);\n"
            "}\n"

            "function reloadTab(t) {\n"
                "globalTabsToLoad = [t];\n"
                "setTimeout(loadNextTab, 500);\n"
            "}\n"

            "function setControlValue(t, json) {\n"
                "var element = document.getElementById(t + \"$\" + json.name),\n"
                    "tag,\n"
                    "type;\n"
                "if (element === null) {\n"     // Could be a note. Not updated (usually).
                    "return;\n"
                "}\n"
                "tag = element.tagName.toUpperCase();\n"
                "if (element.type !== undefined) {\n"
                    "type = element.type.toUpperCase() ;\n"
                "}\n"
                "if (tag === \"DIV\" || tag === \"SPAN\") {\n"
                    "element.innerHTML = json.value;"
                "} else if (tag === \"INPUT\" && type == \"NUMBER\") {\n"
                    "element.value = parseFloat(json.value);\n"
                "} else if (tag === \"INPUT\" && type == \"CHECKBOX\") {\n"
                    "element.checked = parseInt(json.value);\n"
                "} else {\n"
                    "element.value = json.value;\n"
                "}\n"
            "}\n"

            "window.addEventListener(\"load\", reloadAllTabs);\n"

            "function sendData(name) {\n"
                "document.getElementById(\"disable_overlay\").style.display = \"block\";"
                "var XHR = new XMLHttpRequest(),\n"
                "form = document.getElementById(name + \"_form\"),\n"
                "FD = new FormData( form );\n"
                "XHR.addEventListener(\"load\", function(event) {\n"
                    "if (this.status == 200)\n"
                    "{\n"
                        "alert(\"Saved settings\");\n"
                    "}\n"
                    "else if (this.status == 401)\n"
                    "{"
                        "alert(\"Settings not saved, authentication failed\");\n"
                    "}"
                    "else\n"
                    "{"
                        "alert(\"Settings not saved, server response: \" + this.statusText);\n"
                    "}"
                    "document.getElementById(\"disable_overlay\").style.display = \"none\";"
                "});\n"
                "XHR.addEventListener(\"error\", function( event ) {\n"
                    "alert(\"Submitting settings failed\");\n"
                    "document.getElementById(\"disable_overlay\").style.display = \"none\";"
                "});"
                "XHR.open(\"POST\", \"/\" + name + \"/set\");\n"
                "XHR.send( FD );\n"
            "}"

            "var periodicUpdateList = [];"

            // Update only the active tab, to minimize load.
            // Should work in Chrome, IE 9.0+, Edge, Firefox, Safari, Opera.
            "function periodicUpdate() {\n"
                "if (periodicUpdateList.length !== 0)\n"
                "{\n"
                    "var activeTab = document.getElementsByClassName(\"tabcontent active\")[0].id;\n"
                    "for (var i = 0; i < periodicUpdateList.length; ++i)\n"
                    "{"
                        "if (periodicUpdateList[i].substring(0, activeTab.length + 1) === (activeTab + \"&\") ||"
                            "periodicUpdateList[i] == activeTab)\n"
                        "{"
                            "globalTabsToLoad.push(periodicUpdateList[i]);"
                        "}"
                    "}"
                    "setTimeout(loadNextTab, 500);\n"
                "}\n"
            "}\n"

            "setInterval(periodicUpdate, 5000);\n"

            "function factoryReset() {\n"
                "if (confirm(\"Reset all to factory defaults: this will erase all settings, including WiFi\\n.Are you sure?\")) {"
                    "document.location = \"/factoryreset?confirm=true\";\n"
                "}"
            "}";
    }

    /**
     * @brief This templated function copies chunks of a string array to the buffer.
     *
     * It is used both when sending the main page and when sending the style sheet and JavaScript.
     *
     * @tparam N                        The array size; inferred from the arguments.
     * @param[in,out] buffer            The output buffer.
     * @param maxLen                    The maximum buffer size.
     * @param[in,out] size              On call, the used size of the buffer. Updated to the total consumed size on return.
     * @param string_buffer             The string array being sent.
     * @param[in,out] sent_static_size  The amount of the static string sent so far.
     * @return true     All data in the string has been copied to the buffer.
     * @return false    There is yet more data in the string to be copied to the buffer. Another chunk must be sent.
     */
    template<size_t N>
    bool send_static_string(uint8_t *buffer, size_t maxLen, size_t &size, const char (&string_buffer)[N], size_t &sent_static_size)
    {
        size_t remaining = N - sent_static_size - 1;
        if (remaining == 0)
        {
            return true;
        }

        size_t can_send = std::min(maxLen - size, remaining);
        memcpy_P(&buffer[size], &string_buffer[sent_static_size], can_send);
        size += can_send;
        sent_static_size += can_send;
        return can_send == remaining;
    }

    WebSettings::WebSettings(uint16_t port): server(port)
    {
        generate_new_authentication();
    }

    void WebSettings::setup(const notify_t &on_save_f, const notify_t &on_restart_f, const notify_t &on_factory_reset_f)
    {
        on_save = on_save_f;
        on_restart = on_restart_f;
        on_factory_reset = on_factory_reset_f;

        // Send the chunked main page.
        server.on("/", HTTP_GET, [this] (AsyncWebServerRequest *request)
        {
            // This allocation will be deleted when the chunked-write completes.
            MainPageChunkContext *context = new MainPageChunkContext;
            // WARNING: Do not pass local variables by reference to the lambda; this will not work properly
            // on anything but the first call.
            AsyncWebServerResponse *response = request->beginChunkedResponse(TEXT_HTML, [this, context] (uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                return on_main_page_chunk(buffer, maxLen, index, context);
            });
            request->send(response);
        });

        // Note that these two are embedded in the main page; however, doing
        // them also as separate URLs allows other pages to reference them.
        // The embedding is to maximize efficiency, in both memory usage and processing.
        server.on("/style.css", HTTP_GET, [this] (AsyncWebServerRequest *request)
        {
            // The server does not seem to be very happy sending massive chunks in one go.
            // This allocation will be deleted when the chunked-write completes.
            MainPageChunkContext *context = new MainPageChunkContext;
            // WARNING: Do not pass local variables by reference to the lambda; this will not work properly
            // on anything but the first call.
            AsyncWebServerResponse *response = request->beginChunkedResponse(TEXT_HTML, [this, context] (uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                if (index == 0)
                {
                    context->sent_static_size = 0;
                }
                size_t size = 0;
                send_static_string(buffer, maxLen, size, style, context->sent_static_size);
                if (size == 0)
                {
                    delete context;
                }
                return size;
            });
            request->send(response);
        });

        server.on("/script.js", HTTP_GET, [this] (AsyncWebServerRequest *request)
        {
            // The server does not seem to be very happy sending massive chunks in one go.
            // This allocation will be deleted when the chunked-write completes.
            MainPageChunkContext *context = new MainPageChunkContext;
            // WARNING: Do not pass local variables by reference to the lambda; this will not work properly
            // on anything but the first call.
            AsyncWebServerResponse *response = request->beginChunkedResponse(TEXT_HTML, [this, context] (uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                if (index == 0)
                {
                    context->sent_static_size = 0;
                }
                size_t size = 0;
                send_static_string(buffer, maxLen, size, javascript_text, context->sent_static_size);
                if (size == 0)
                {
                    delete context;
                }
                return size;
            });
            request->send(response);
        });

        server.on("/settings/set", HTTP_POST, [this](AsyncWebServerRequest *request)
        {
            if (!verify_authentication(request))
            {
                return;
            }
            for (auto &setting : setting_panels)
            {
                // Settings have unique IDs accross all tabs.
                setting->on_post(request);
            }
            if (on_save != nullptr)
            {
                on_save(*this);
            }
            // Response is JSON.
            request->send(200, TEXT_JSON, F("{\"saved\":true}"));
        });

        server.on("/settings/get", HTTP_GET, [this](AsyncWebServerRequest *request)
        {
            on_request_values(request);
        });

        if (on_restart != nullptr)
        {
            server.on("/reboot", HTTP_GET, [this] (AsyncWebServerRequest *request)
            {
                if (!verify_authentication(request))
                {
                    return;
                }
                request->send(200, TEXT_HTML, F("Device is rebooting. <a href=\"/\">Back to root (wait for reboot!)</a>"));
                delay(2000);
                on_restart(*this);
            });
        }

        if (on_factory_reset != nullptr)
        {
            server.on("/factoryreset", HTTP_GET, [this] (AsyncWebServerRequest *request)
            {
                if (!verify_authentication(request))
                {
                    return;
                }
                if (request->hasArg("confirm") && request->arg("confirm") == "true")
                {
                    request->send(200, TEXT_HTML, F("Device is resetting. You will need to reconnect to the soft AP to configure afterwards."));
                    delay(1000);
                    on_factory_reset(*this);
                }
                else
                {
                    request->send(200, TEXT_HTML, F("Reset to factory defaults not confirmed. <a href=\"/\">Back to root</a>"));
                }
            });
        }

        if (on_restart != nullptr)
        {
            server.on("/upload", HTTP_GET, [this] (AsyncWebServerRequest *request)
            {
                if (!verify_authentication(request))
                {
                    return;
                }
                on_request_upload(request);
            });

            server.on("/upload", HTTP_POST, [this] (AsyncWebServerRequest *request)
            {
                if (!verify_authentication(request))
                {
                    return;
                }
            },
            [this] (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
            {
                if (!verify_authentication(request))
                {
                    return;
                }
                handle_upload(request, filename, index, data, len, final);
            });
        }

        server.onNotFound([this](AsyncWebServerRequest *request) { on_not_found(request); });

        server.begin();
    }

    /**
     * @brief This templated function ensures a safe copy to the buffer.
     *
     * It will copy PROGMEM data from a source string to the target. Trailing
     * NULL is omitted. If the size exceeds the remaining available, nothing is done.
     *
     * @tparam N        Template parameter; automatically deduced from array size.
     * @param buffer    Output buffer.
     * @param string    The string to copy to the output buffer.
     * @param size      Current used size in the output buffer.
     * @param maxLen    Maximum buffer size.
     */
    template<size_t N>
    void buffer_append(uint8_t *buffer, const char (&string)[N], size_t &size, size_t maxLen)
    {
        if (size + N > maxLen)
        {
            // Would overflow.
            return;
        }
        memcpy_P(&buffer[size], string, N - 1);
        size += N - 1;
    }

    bool WebSettings::on_main_page_tabbutton_chunk(uint8_t *buffer, size_t maxLen, size_t &size, MainPageChunkContext &context)
    {
        static const char button_start[] PROGMEM = "<button class=\"tablinks";
        static const char active[] PROGMEM = " active";
        static const char onclick[] PROGMEM = "\" onclick=\"openTab(event, '";
        static const char onclick_end[] PROGMEM = "')\">";
        static const char button_end[] PROGMEM = "</button>";
        // Length of one button, without the two instances of the setting name.
        static constexpr size_t basic_length =
            sizeof(button_start) - 1 +
            sizeof(onclick) - 1 +
            sizeof(onclick_end) - 1 +
            sizeof(button_end) - 1;

        // Stuff as much as possible in the oiutput without overflowing.
        while (context.current_panel != setting_panels.end())
        {
            bool first = context.current_panel == setting_panels.begin();
            auto &panel = **context.current_panel;
            size_t name_length = panel.get_name_length();
            auto name = panel.get_name();
            size_t identifier_length = panel.get_identifier_length();
            auto identifier = panel.get_identifier();

            // Compute the expected text length.
            size_t len = basic_length + (first ? sizeof(active) -1 : 0) +
                name_length + identifier_length;

            if (len + size > maxLen)
            {
                // Does not fit.
                return false;
            }

            buffer_append(buffer, button_start, size, maxLen);
            if (first)
            {
                buffer_append(buffer, active, size, maxLen);
                first =- false;
            }
            buffer_append(buffer, onclick, size, maxLen);
            memcpy_P(&buffer[size], identifier, identifier_length);
            size += identifier_length;
            buffer_append(buffer, onclick_end, size, maxLen);
            memcpy_P(&buffer[size], name, name_length);
            size += name_length;
            buffer_append(buffer, button_end, size, maxLen);

            ++context.current_panel;
        }

        return true;    // All done.
    }

    bool WebSettings::on_main_page_tabbody_chunk(uint8_t *buffer, size_t maxLen, size_t &size, MainPageChunkContext &context)
    {
        while (context.current_panel != setting_panels.end())
        {
            static const char tab_start[] PROGMEM = "<div id=\"";
            static const char tab_class[] PROGMEM = "\" class=\"tabcontent";
            static const char active[] PROGMEM = " active";
            static const char end_start[] PROGMEM = "\">";
            static const char end_tab[] PROGMEM = "<div style=\"clear: both\"></div></div>";
            static const char first_tabbutton_begin[] PROGMEM =
                "</div>"
                "<form method=\"post\" id=\"settings_form\" action=\"/savesettings\">";

            bool first = context.current_panel == setting_panels.begin();
            const auto &panel = **context.current_panel;
            size_t identifier_length = panel.get_identifier_length();
            auto identifier = panel.get_identifier();

            // Were the preliminaries written?
            if (context.starting_tab)
            {
                // Yes. Will the preliminaries fit?
                size_t preliminary_len =
                    sizeof(tab_start) - 1 +
                    identifier_length +
                    sizeof (tab_class) - 1 +
                    (first ? sizeof(first_tabbutton_begin) -1  + sizeof(active) - 1 : 0) +
                    sizeof (end_start) - 1;

                if (preliminary_len + size > maxLen)
                {
                    // No. Continue in next chunk.
                    return false;
                }

                // Preliminaries fit.
                if (first)
                {
                    buffer_append(buffer, first_tabbutton_begin, size, maxLen);
                }

                buffer_append(buffer, tab_start, size, maxLen);

                memcpy_P(&buffer[size], identifier, identifier_length);
                size += identifier_length;

                buffer_append(buffer, tab_class, size, maxLen);
                if (first)
                {
                    buffer_append(buffer, active, size, maxLen);
                }
                buffer_append(buffer, end_start, size, maxLen);

                context.starting_tab = false;
                context.current_setting = panel.get_settings().begin();
            }

            // Now, try to stuff in settings until done or out of room.
            String panel_identifier(panel.get_identifier());
            String html;
            while (context.current_setting != panel.get_settings().end())
            {
                const auto &setting = *context.current_setting;
                html = setting->get_html(panel_identifier);
                if (html.length() + size > maxLen)
                {
                    // Can't fit this settting.
                    return false;
                }
                memcpy(&buffer[size], html.c_str(), html.length());
                size += html.length();

                ++context.current_setting;
            }

            // All settings have been included. Will the tab footer fit?
            if (sizeof (end_tab) + size - 1 > maxLen)
            {
                // Won't fit. Will come back in, `starting_tab` will be false,
                // and the current_setting will equal `end()`, so it'll come
                // straight to trying to do the tab footer again.
                return false;
            }

            buffer_append(buffer, end_tab, size, maxLen);
            // Can move to the next panel.
            ++context.current_panel;
            context.current_setting = context.current_panel != setting_panels.end() ? setting_panels.front()->get_settings().begin() :
                SettingInterface::settings_list_t::const_iterator();
            context.starting_tab = true;
        }

        // At this point all setting panels have been done.
        return true;
    }

    bool WebSettings::on_main_page_footer_chunk(uint8_t *buffer, size_t maxLen, size_t &size, MainPageChunkContext &context)
    {
        static const char footer_start[] PROGMEM =
                    "<input class=\"md_button ripple\" type=\"submit\" value=\"Save\">"
                    "<a class=\"md_button ripple\" onclick=\"reloadAllTabs()\">Reset Form</a>";
        static const char hr_text[] PROGMEM = "<hr>";
        static const char reboot_button[] PROGMEM =
            "<a class=\"md_button ripple red\" href=\"/reboot\">Reboot</a>";
        static const char factory_reset_button[] PROGMEM =
            "<a class=\"md_button ripple red\" onclick=\"factoryReset()\">Factory Defaults</a>";
        static const char upload_button[] PROGMEM =
            "<a class=\"md_button ripple red\" href=\"/upload\">Upload Firmware</a>";
        static const char footer_end[] PROGMEM =
                "</form>"
                "<script>"
                    "var form = document.getElementById(\"settings_form\");"
                    "form.addEventListener(\"submit\", function ( event ) {"
                        "event.preventDefault();"
                        "sendData(\"settings\");"
                    "})"
                "</script>"
            "</body>"
            "</html>";
        size_t len =
            sizeof(footer_start) - 1 +
            sizeof(footer_end);

        if (on_restart != nullptr)
        {
            len +=
                sizeof (hr_text) - 1 +
                sizeof (reboot_button) -1 +
                (on_factory_reset != nullptr ? sizeof (factory_reset_button) - 1 : 0) +
                sizeof (hr_text) +
                sizeof (upload_button) - 1;
        }
        else if (on_factory_reset != nullptr)
        {
            len +=
                sizeof (hr_text) - 1 +
                sizeof (factory_reset_button) - 1;
        }

        if (len + size > maxLen)
        {
            return false;
        }

        buffer_append(buffer, footer_start, size, maxLen);
        if (on_restart != nullptr)
        {
            buffer_append(buffer, hr_text, size, maxLen);
            buffer_append(buffer, reboot_button, size, maxLen);
            if (factory_reset_button != nullptr)
            {
                buffer_append(buffer, factory_reset_button, size, maxLen);
            }
            buffer_append(buffer, hr_text, size, maxLen);
            buffer_append(buffer, upload_button, size, maxLen);
        }
        else if (on_factory_reset != nullptr)
        {
            buffer_append(buffer, hr_text, size, maxLen);
            buffer_append(buffer, factory_reset_button, size, maxLen);
        }

        buffer_append(buffer, footer_end, size, maxLen);

        return true;
    }

    size_t WebSettings::on_main_page_chunk(uint8_t *buffer, size_t maxLen, size_t index, MainPageChunkContext *context_ptr)
    {
        auto & context = *context_ptr;
        size_t size(0);
        if (index == 0)
        {
            context.sent_static_size = 0;
        }

        switch (context.state)
        {
            case MainPageChunkState::BEGIN_PAGE:
                // This assumes that the page begin fits in the buffer. That's a pretty
                // safe assumption, this string is rather short.
                static const char main_page_begin_page[] PROGMEM =
                    "<!DOCTYPE html>"
                    "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge,chrome=1\">"
                    "<html>"
                        "<style>";
                send_static_string(buffer, maxLen, size, main_page_begin_page, context.sent_static_size);
                context.state = MainPageChunkState::STYLE_SHEET;
                context.sent_static_size = 0;
                // FALL THROUGH
            case MainPageChunkState::STYLE_SHEET:
                // The entire style sheet doesn't typically fit in one buffer.
                if (!send_static_string(buffer, maxLen, size, style, context.sent_static_size))
                {
                    break;
                }
                context.state = MainPageChunkState::PRE_JAVASCRIPT;
                context.sent_static_size = 0;
                // FALL THROUGH
            case MainPageChunkState::PRE_JAVASCRIPT:
                static const char transition_style_to_script[] PROGMEM = "</style><script language=\"javascript\">";
                if (!send_static_string(buffer, maxLen, size, transition_style_to_script, context.sent_static_size))
                {
                    break;
                }
                context.state = MainPageChunkState::JAVASCRIPT;
                context.sent_static_size = 0;
                // FALL THROUGH
            case MainPageChunkState::JAVASCRIPT:
                if (!send_static_string(buffer, maxLen, size, javascript_text, context.sent_static_size))
                {
                    break;
                }
                context.state = MainPageChunkState::POST_JAVASCRIPT;
                context.sent_static_size = 0;
                // FALL THROUGH
            case MainPageChunkState::POST_JAVASCRIPT:
                static const char transition_script_to_body[] PROGMEM =
                    "</script><body>"
                        "<div id=\"disable_overlay\" class=\"disable_overlay\"></div>"
                        "<div class=\"tab\">";
                if (!send_static_string(buffer, maxLen, size, transition_script_to_body, context.sent_static_size))
                {
                    break;
                }
                context.state = MainPageChunkState::TABBUTTON_HEADER;
                context.sent_static_size = 0;
                context.current_panel = setting_panels.begin();
                context.current_setting = context.current_panel != setting_panels.end() ? setting_panels.front()->get_settings().begin() :
                    SettingInterface::settings_list_t::const_iterator();
                // FALL THROUGH
            case MainPageChunkState::TABBUTTON_HEADER:
                if (!on_main_page_tabbutton_chunk(buffer, maxLen - 1, size, context))
                {
                    break;
                }
                context.state = MainPageChunkState::TAB_BODY;
                context.sent_static_size = 0;
                context.current_panel = setting_panels.begin();
                context.current_setting = context.current_panel != setting_panels.end() ? setting_panels.front()->get_settings().begin() :
                    SettingInterface::settings_list_t::const_iterator();
                context.starting_tab = true;
                break;
            case MainPageChunkState::TAB_BODY:
                if (!on_main_page_tabbody_chunk(buffer, maxLen - 1, size, context))
                {
                    break;
                }
                context.state = MainPageChunkState::FOOTER;
                // Not necessary to reset other state values.
                // FALL THROUGH.
            case MainPageChunkState::FOOTER:
                if (!on_main_page_footer_chunk(buffer, maxLen - 1, size, context))
                {
                    break;
                }
                context.state = MainPageChunkState::DONE;
                break;

            case MainPageChunkState::DONE:
                // Will return 0.
                if (size == 0)
                {
                    delete context_ptr;
                }
                break;

            default:
                // Whoops.
                static const char failure[] PROGMEM = "Internal error";
                buffer_append(buffer, failure, size, maxLen);
                context.state = MainPageChunkState::DONE;
                break;
        }

        return size;
    }

    void WebSettings::add_setting_set(const __FlashStringHelper *name, const __FlashStringHelper *identifier, const SettingInterface::settings_list_t &list)
    {
        setting_panels.emplace_back(std::make_unique<SettingPanel>(name, identifier, list));
    }

    void WebSettings::loop()
    {
        // Asynchronous, nothing to see here. This is not the loop you are looking for.
    }

    void WebSettings::on_request_upload(AsyncWebServerRequest *request)
    {
        //!< @TODO This should have better styling.
        request->send(200, TEXT_HTML, F("<!DOCTYPE html>"
            "<link rel=\"stylesheet\" href=\"/style.css\">"
            "<html><body><H1>Upload New Firmware</H1>"
            "<form id='form' method='POST' action='/upload' enctype='multipart/form-data'>"
            "<input type='file' name='file' id='file' class='md_button ripple' accept='.bin,application/octet-stream' onchange=\"document.getElementById('sbmt').disabled = false\">"
            "<br><br>"
            "<input type='submit' disabled='true' value='Upload' id='sbmt' class='md_button ripple'>"
            "<button type='button' onclick='document.location = \"/\"' class='md_button ripple red'> Cancel </button>"
            "</form>"));
    }

    void WebSettings::handle_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
    {
        // Shamelessly adapted from tzapu/WiFiManager for Async web server

        // handler for the file upload, gets the sketch bytes, and writes
        // them through the Update object

        // UPLOAD START
        if (index == 0)
        {
            uint32_t maxSketchSpace;

#ifdef ESP8266
            maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#elif defined(ESP32)
            // Think we do not need to stop WiFIUDP because we haven't started a listener
            // maxSketchSpace = (ESP.getFlashChipSize() - 0x1000) & 0xFFFFF000;
            // #define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF // include update.h
            maxSketchSpace = UPDATE_SIZE_UNKNOWN;
#endif
            // Required to run from AsyncWebServer.
            Update.runAsync(true);

            if (!Update.begin(maxSketchSpace))
            {                 // start with max available size
                on_update_failed(request);
                Update.end();
                return;
            }
        }

        // UPLOAD WRITE
        if (len != 0)
        {
            if (Update.write(data, len) != len)
            {
                on_update_failed(request);
                Update.end();
                return;
            }
        }

        // UPLOAD FILE END
        if (final)
        {
            if (!Update.end(true))
            {
                on_update_failed(request);
                return;
            }

            // This also checks for general update errors as well.
            on_update_done(request);
        }

        delay(0);
    }

    void WebSettings::on_update_failed(AsyncWebServerRequest *request)
    {
        String page = F("<!DOCTYPE html><html>"
            "<link rel=\"stylesheet\" href=\"/style.css\">"
            "<body><h1>Upload Failed</h1>");
        page += FPSTR(status_div);
        page += F("<strong>Update Failed.</strong><Br/>Rebooting may clear the issue.<br/>");
#ifdef ESP32
        page += "OTA Error: " + (String)Update.errorString();
#else
        page += F("Update Error Code: ");
        page += Update.getError();
#endif
        page += TEXT_END_DIV;

        page += F("</body></html>");

        request->send(500, TEXT_HTML, page);
    }

    void WebSettings::on_update_done(AsyncWebServerRequest *request)
    {
        if (Update.hasError())
        {
            on_update_failed(request);
            return;
        }

        // Shamelessly taken from tzapu/WiFiManager

        String page = F("<!DOCTYPE html><html>"
            "<link rel=\"stylesheet\" href=\"/style.css\">"
            "<body><h1>Upload completed</h1>");
        page += FPSTR(status_div);
        page += F("Update completed; device is rebooting.</div></body></html>");

        request->send(200, TEXT_HTML, page);

        // Tell the main loop to restart; otherwise this won't
        // send the reply.
        on_restart(*this);
    }

    void WebSettings::on_request_values(AsyncWebServerRequest *request)
    {
        if (!request->hasArg("tab"))
        {
            request->send(400, TEXT_PLAIN, F("Query parameter 'tab' missing"));
        }

        // The request->send() deletes the response when done.
        auto response = new AsyncJsonResponse(false, 2048);
        auto & root = response->getRoot();
        auto & tab = request->arg("tab");
        // Collect all 'setting' arguments.
        std::vector<const String *> requested_settings;
        requested_settings.reserve(request->args());
        int tabParameterCount(0);
        for (size_t i = 0; i < request->args(); ++i)
        {
            if (request->argName(i) == "setting")
            {
                requested_settings.push_back(&request->arg(i));
            }
            if (request->argName(i) == "tab")
            {
                ++tabParameterCount;
            }
        }
        if (tabParameterCount != 1)
        {
            request->send(400, TEXT_PLAIN, F("More than one query parameter 'tab' is not supported"));
        }
        for (auto &setting_panel: setting_panels)
        {
            if (tab == setting_panel->get_identifier())
            {
                root[setting_panel->get_identifier()] = setting_panel->as_json(requested_settings);
            }
        }
        response->setLength();
        response->addHeader("Cache-Control", "no-cache");
        request->send(response);
    }

    void WebSettings::on_not_found(AsyncWebServerRequest *request)
    {
        // In soft AP mode redirect to the root document.
        if (WiFi.softAPgetStationNum() != 0)
        {
            String apURL = F("http://");
            apURL += WiFi.softAPIP().toString();
            if (!request->header(F("host")).startsWith(apURL))
            {
                request->redirect(apURL);
            }
        }
        else
        {
            request->send(404, TEXT_HTML, F("<!DOCTYPE html><html><body><H1>404 Page Not Found</H1><br><A HREF=\"/\">Return to root</A></body></html>"));
        }
    }

    void WebSettings::generate_new_authentication()
    {
        // Authentication realm may exclude some characters,
        // for simplicity generate using the following set.
        static const char realm_chars[] PROGMEM = "!#$%&0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        for (uint16_t i = 0; i < sizeof(auth_realm) - 1; ++i)
        {
            auth_realm[i] = static_cast<char>(pgm_read_byte(realm_chars + random(0, sizeof(realm_chars) - 2)));
        }

        auth_realm[sizeof(auth_realm) - 1] = '\0';

        if (!auth_user.isEmpty())
        {
            last_auth_digest = std::move(generateDigestHash(auth_user.c_str(), auth_password.c_str(), auth_realm));
        }
    }

    bool WebSettings::verify_authentication(AsyncWebServerRequest *request)
    {
        if (!auth_user.isEmpty())
        {
            if (!request->authenticate(last_auth_digest.c_str()))
            {
                request->requestAuthentication(auth_realm);
                return false;
            }
        }

        generate_new_authentication();
        return true;
    }
}

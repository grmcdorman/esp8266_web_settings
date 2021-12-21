
#include "grmcdorman/WebServer.h"

#include <LittleFS.h>

#include "grmcdorman/SettingPanel.h"

namespace grmcdorman
{
    namespace
    {
        const char *style PROGMEM =
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
            ".tabcontent {"
                "display: none;"
                "padding: 6px 12px;"
                "border: 1px solid #ccc;"
                "border-top: none;"
            "}"
            ".styled_button {"
                "display: inline-block;"
                "padding: 0.3em 1.2em;"
                "margin: 0 0.3em 0.3em 0;"
                "border-radius: 2em;"
                "box-sizing: border-box;"
                "text-decoration: none;"
                "font-size: 12pt;"
                "color: #FFFFFF;"
                "background-color: #4EB5F1;"
                "text-align: center;"
                "transition: all 0.2s;"
                "border-style: outset;"
                "font-weight: bold;"
                "font-family: sans;"
            "}"
            ".styled_button:hover {"
                "background-color: #4095C6;"
            "}"
            "a.link_button {"
                "display: inline-block;"
                "padding: 0.3em 1.2em;"
                "margin: 0 0.3em 0.3em 0;"
                "border-radius: 2em;"
                "box-sizing: border-box;"
                "text-decoration: none;"
                "color: #FFFFFF;"
                "background-color: #4EB5F1;"
                "text-align: center;"
                "transition: all 0.2s;"
                "border-style: outset;"
                "font-size: 12pt;"
                "font-weight: bold;"
                "font-family: sans;"
            "}"
            "a.link_button:hover {"
                "background-color: #4095C6;"
            "}";
        const char *javascript_text PROGMEM =
            "function openTab(evt, tabName) {\n"
                "var i, tabcontent, tablinks;\n"
                "tabcontent = document.getElementsByClassName(\"tabcontent\");\n"
                "for (i = 0; i < tabcontent.length; i++) {\n"
                    "tabcontent[i].style.display = tabcontent[i].id === tabName ? \"block\" : \"none\";\n"
                "}\n"
                "tablinks = document.getElementsByClassName(\"tablinks\");\n"
                "for (i = 0; i < tablinks.length; i++) {\n"
                    "tablinks[i].className = tablinks[i].className.replace(\" active\", \"\");\n"
                "}\n"
                "evt.currentTarget.className += \" active\";\n"
            "}\n"

            "function reloadAllTabs() {\n"
                "var req = new XMLHttpRequest();\n"
                "req.overrideMimeType(\"application/json\");\n"
                "req.open(\"GET\", \"/settings/get\", true);\n"
                "req.onload = handleSettingsGet;\n"
                "req.send(null);\n"
            "}\n"

            "function handleSettingsGet() {\n"
                "var jsonResponse = JSON.parse(this.responseText);\n"
                "Object.keys(jsonResponse).forEach(tabName => {\n"
                    "jsonResponse[tabName].forEach(setting => {\n"
                        "setControlValue(setting);\n"
                    "})\n"
                "});\n"
            "}\n"

            "function setControlValue(json) {\n"
                "var element = document.getElementById(json.name),\n"
                    "tag,\n"
                    "type;\n"
                "if (element === null) {\n" // Notes are not updateable (usually)
                    "return;\n"
                "}\n"
                "tag = element.tagName.toUpperCase();\n"
                "type = element.type.toUpperCase() ;\n"
                "if (tag === \"INPUT\" && type == \"NUMBER\") {\n"
                    "element.value = parseFloat(json.value);\n"
                "} else if (tag === \"INPUT\" && type == \"CHECKBOX\") {\n"
                    "element.checked = parseInt(json.value);\n"
                "} else {\n"
                    "element.value = json.value;\n"
                "}\n"
            "}\n"

            "//window.addEventListener(\"load\", reloadAllTabs);\n"

            "function sendData(name) {\n"
                "var XHR = new XMLHttpRequest(),\n"
                "form = document.getElementById(name + \"_form\"),\n"
                "FD = new FormData( form );\n"
                "XHR.addEventListener(\"load\", function(event) {\n"
                    "alert(\"Saved settings\");\n"
                "});\n"
                "XHR.addEventListener(\"error\", function( event ) {\n"
                    "alert(\"Submitting settings failed\");\n"
                "});"
                "XHR.open(\"POST\", \"/\" + name + \"/set\");\n"
                "XHR.send( FD );\n"
            "}"

            "function factoryReset() {\n"
                "if (confirm(\"Reset all to factory defaults: this will erase all settings, including WiFi\\n.Are you sure?\")) {"
                    "document.location = \"/factoryreset?confirm=true\";\n"
                "}"
            "}";
        const char * PROGMEM page_footer = "<input class=\"styled_button\" type=\"submit\" value=\"Save\">"
            "<button class=\"styled_button\" type=\"button\" onclick=\"reloadAllTabs()\">Reset Form</button>"
            "<a class=\"link_button\" href=\"/reboot\">Reboot</a>"
            "<a class=\"link_button\" onclick=\"factoryReset()\">Factory Defaults</a>"
            "<a class=\"link_button\" href=\"/upload\">Upload Firmware</a>"
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
    }

    WebServer::WebServer()
    {
    }

    void WebServer::setup(WebServer::notify_save_t on_save)
    {
        constexpr static const char *mainPagePath = "/main.html";
        if (!LittleFS.begin())
        {
            Serial.println("Unable to set up LittleFS");
            server.on("/", HTTP_GET, [this] () {
                server.send(500, "text/plain", "Failed to set up main page (could initialize LittleFS)");
            });
            return;
        }

        auto mainPageFile = LittleFS.open(mainPagePath, "w");
        if (!mainPageFile)
        {
            Serial.println("Unable to set up LittleFS");
            server.on("/", HTTP_GET, [this] () {
                server.send(500, "text/plain", "Failed to set up main page (could not open file)");
            });
            return;
        }

        mainPageFile.println(
            "<!DOCTYPE html>"
            "<html>"
            "<script>");
        mainPageFile.println(FPSTR(javascript_text));
        mainPageFile.println("</script>"
                    "<style>");
        mainPageFile.print(FPSTR(style));
        mainPageFile.println(
            "</style>"
            "<body>"
            "<div class=\"tab\">");
        for (auto &setting_set : settings_set_list)
        {
            mainPageFile.print("<button class=\"tablinks active\"onclick=\"openTab(event, '");
            mainPageFile.print(setting_set.first);
            mainPageFile.print("')\">");
            mainPageFile.print(setting_set.first);
            mainPageFile.println("</button>");
        }
        mainPageFile.println(
            "</div>"
            "<form method=\"post\" id=\"settings_form\" action=\"/savesettings\">");
        for (auto &setting_set : settings_set_list)
        {
            if (setting_set.second.empty())
            {
                continue;
            }

            mainPageFile.println("<div id=\"" + setting_set.first + "\" class=\"tabcontent\">");
            auto setting = new SettingPanel(setting_set.first, setting_set.second);
            mainPageFile.println(setting->body());
            mainPageFile.println("</div>\n");

            setting_panels.emplace_back(setting);
        }
        mainPageFile.println(FPSTR(page_footer));

        mainPageFile.close();
        server.serveStatic("/", LittleFS, mainPagePath);

        server.on("/settings/set", HTTP_POST, [this, on_save]()
                  {
                      for (auto &setting : setting_panels)
                      {
                          // Settings have unique IDs accross all tabs.
                          setting->on_post(server);
                      }
                      if (on_save != nullptr)
                      {
                          on_save(*this);
                      }
                      // The response from the save is the values set.
                      on_request_values();
                  });

        server.on("/settings/get", HTTP_GET, [this]()
                  {
                      on_request_values();
                  });

        server.on("/reboot", HTTP_GET, [this] ()
        {
            server.send(200, "text/html", "Device is rebooting. <a href=\"/\">Back to root (after reboot)</a>");
            delay(1000);
            ESP.restart();
        });

        server.on("/factoryreset", HTTP_GET, [this] ()
        {
            Serial.println("Reset arg=" + server.arg("confirm"));
            if (server.hasArg("confirm") && server.arg("confirm") == "true")
            {
                server.send(200, "text/html", "Device is resetting. You will need to reconnect to the soft AP to configure afterwards.");
                delay(1000);
                // Clear file system.
                LittleFS.format();
                // Erase configuration
                ESP.eraseConfig();
                // Reset (not reboot, that may save current state)
                ESP.reset();
            }
            else
            {
                server.send(200, "text/html", "Reset to factory defaults not confirmed. <a href=\"/\">Back to root</a>");
            }
        });

        server.on("/upload", HTTP_GET, [this] {
            on_request_upload();
        });

        server.on("/u", HTTP_POST, [this] {
            on_do_upload();
        },
        [this] {
            on_upload_done();
        });

        server.onNotFound([this]() { on_not_found(); });

        server.begin();
    }

    void WebServer::add_setting_set(const String &name, const SettingInterface::settings_list_t &list)
    {
        settings_set_list.emplace_back(name, list);
    }

    void WebServer::loop()
    {
        server.handleClient();
    }

    void WebServer::on_request_upload()
    {
        // The original text included a hard-coded redirect to a browser for the captive portal;
        // it assumes 192.168.4.1 for the device IP.
        server.send(200, "text/html", "Upload New Firmware<br/>"
            "<form method='POST' action='u' enctype='multipart/form-data' onchange=\"(function(el){document.getElementById('uploadbin').style.display = el.value=='' ? 'none' : 'initial';})(this)\">"
            "<input type='file' name='update' accept='.bin,application/octet-stream'>"
            "<button id='uploadbin' type='submit' class='h D'>Update</button></form>");
    }

    void WebServer::on_do_upload()
    {
        // Shamelessly taken from tzapu/WiFiManager

        // handler for the file upload, get's the sketch bytes, and writes
        // them through the Update object
        HTTPUpload &upload = server.upload();

        // UPLOAD START
        if (upload.status == UPLOAD_FILE_START)
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

            if (!Update.begin(maxSketchSpace))
            {                 // start with max available size
                Update.end(); // Not sure the best way to abort, I think client will keep sending..
            }
        }
        // UPLOAD WRITE
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            {
            }
        }
        // UPLOAD FILE END
        else if (upload.status == UPLOAD_FILE_END)
        {
            if (Update.end(true))
            { // true to set the size to the current progress
            }
            else
            {
                Update.printError(Serial);
            }
        }
        // UPLOAD ABORT
        else if (upload.status == UPLOAD_FILE_ABORTED)
        {
            Update.end();
        }
        delay(0);
    }

    void WebServer::on_upload_done()
    {
        // Shamelessly taken from tzapu/WiFiManager
        String page = "<!DOCTYPE html><html><body>Upload accepted.<br>";

        if (Update.hasError())
        {
            page += "<div style='padding:20px;margin:20px 0;border:1px solid #eee;border-left-width:5px;border-left-color:#777;'><strong>Update Failed!</strong><Br/>Reboot device and try again<br/>";
#ifdef ESP32
            page += "OTA Error: " + (String)Update.errorString();
#else
            page += "OTA Error: " + (String)Update.getError();
#endif
            page += "</div>";
        }
        else
        {
            page += "<div style='padding:20px;margin:20px 0;border:1px solid #eee;border-left-width:5px;border-left-color:#777;'>Update completed; device is rebooting.</div>";
        }

        page += "</body></html>";

        server.send(200, "text/html", page);

        if (!Update.hasError())
        {
            delay(1000); // send page
            ESP.restart();
        }
    }

    void WebServer::on_request_values()
    {
        // This is a very rough guess at the size needed.
        DynamicJsonDocument json(setting_panels.size()*512 + 64);

        for (auto &setting_panel: setting_panels)
        {
            json[setting_panel->get_name()] = setting_panel->as_json();
        }

        auto size = measureJson(json) + 2;
        std::unique_ptr<char[]> buffer(new char[size]);
        buffer.get()[0] = 0;
        serializeJson(json, buffer.get(), size);
        server.send(200, "text/json", buffer.get());
    }

    void WebServer::on_not_found()
    {
        // In soft AP mode redirect to the root document.
        if (WiFi.softAPgetStationNum() != 0 && !server.hostHeader().startsWith("http://" + WiFi.softAPIP().toString()))
        {
            String URL = "http://" + WiFi.softAPIP().toString() + "/";
            server.sendHeader("Location", URL, true);
            server.send(302, "text/plain", "");
        }
        else
        {
            server.send(404, "text/html", "<!DOCTYPE html><html><body><H1>404 Page Not Found</H1><br><A HREF=\"/\">Return to root</A></body></html>");
        }
    }
}

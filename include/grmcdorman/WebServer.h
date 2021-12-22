#pragma once

#include <list>
#include <unordered_map>
#include <memory>

#include <WString.h>
#include <ESPAsyncWebServer.h>

#include "grmcdorman/SettingPanel.h"

namespace grmcdorman
{
    class Device;

    class WebServer
    {
    public:
        typedef void (*notify_save_t)(WebServer &);
        WebServer();
        void setup(notify_save_t on_save);
        void loop();
        void add_setting_set(const String &name, const SettingInterface::settings_list_t &setting_set);
    private:
        void on_not_found(AsyncWebServerRequest *request);
        void on_request_values(AsyncWebServerRequest *request);
        void on_request_upload(AsyncWebServerRequest *request);
        void on_do_upload(AsyncWebServerRequest *request);
        void handle_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        void on_update_failed(AsyncWebServerRequest *request);
        void on_update_done(AsyncWebServerRequest *request);

        notify_save_t on_save;
        AsyncWebServer server;
        std::list<std::pair<String, SettingInterface::settings_list_t>> settings_set_list;
        std::list<std::unique_ptr<SettingPanel>> setting_panels;
    };
}
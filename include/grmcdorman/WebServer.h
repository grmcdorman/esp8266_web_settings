#pragma once

#include <list>
#include <unordered_map>
#include <memory>

#include <WString.h>
#include <ESP8266WebServer.h>

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
        void on_not_found();
        void on_request_values();
        void on_request_upload();
        void on_do_upload();
        void on_upload_done();

        notify_save_t on_save;
        ESP8266WebServer server;
        std::list<std::pair<String, SettingInterface::settings_list_t>> settings_set_list;
        std::list<std::unique_ptr<SettingPanel>> setting_panels;
    };
}
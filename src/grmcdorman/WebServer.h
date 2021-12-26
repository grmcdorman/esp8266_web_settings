#pragma once

#include <list>
#include <unordered_map>
#include <memory>

#include <WString.h>
#include <ESPAsyncWebServer.h>

#include "grmcdorman/SettingPanel.h"

namespace grmcdorman
{
    /**
     * @mainpage esp8266_web_settings Detailed Documentation
     *
     * @section intro_sec Introduction
     * An implementation of a Web Server supporting setting panels.
     *
     * HTTPS is not supported. Minimal authentication - a single id/password - is supported for save, reboot, factory defaults, and upload.
     *
     * @section overview_sec Overview
     * The server supports the following pages or requests:
     *  * "/": The root page. This contains the setting panels, and five buttons, `Save`, `Reset Form`, `Reboot`, `Factory Defaults`, and `Uplaod Firmware`.
     *  * "/style.css": CSS styles for the root page.
     *  * "/script.js": JavaScript for the root page.
     *  * "/settings/get": This path requires at least one parameter, the setting tab name. The values for that tab are returned as JSON.
     *     Example: "/settings/get?tab=Overview"
     *  * "/settings/set": Handles POST of the form data from the main page. When all data has been transferred to the settings, the `on_save` callback is invoked.
     *  * "/reboot": Call the `on_restart` callback. Performs no other action. Unprotected.
     *  * "/factoryreset": Call the `on_factory_reset` callback. Performs no other action. Unprotected.
     *  * "/upload": Show the upload page; this allows firmware uploads. Unprotected.
     *  * "/upload": POST request; upload firmware. Unprotected.
     *
     * If the `on_restart` or `on_factory_reset` callbacks are not provided (i.e. are null), the associated URLs will
     * not be registered. The '/upload' URL will also not be registered if the `on_restart` callback is null.
     *
     * The main page is written to storage (TinyFS) in the `setup` method, and served directly from storage thereafter.
     * Sufficient storage must be available for the complete main page.
     *
     * A 404 handler is also installed. When SoftAP mode is detected, this will return a 302 response redirecting
     * the page to the root page. This makes the server function as a captive portal in this mode. When the system
     * is not in SoftAP mode, a simple 404 page is returned.
     *
     * The `on_save` callback should save and apply settings.
     *
     * The `on_reboot` callback should set a flag to indicate a reboot has been requested, and perform this in the
     * main loop after a short delay (e.g. 100ms) by calling `ESP.restart()`.
     *
     * The `on_factory_reset` callback should set a flag to indcate a factory reset has been requested, and perform
     * this in the main loop after a short delay. To reset, the following operations should be performed:
     *   `LittleFS.format()` and/or `TinyFS.format()`, depending on which file systems are in use.
     *  - `ESP.eraseConfig()`.
     *  - `ESP.reset()`.
     *
     * This uses the ESP Asynch Web Server to serve the page.
     */

    /**
     * @brief The WebServer class.
     *
     * This is the primary class for the library; it handles the web pages
     * and manages the sets of settings.
     *
     */
    class WebServer
    {
    public:
        typedef void (*notify_t)(WebServer &);      //!< The callback definition.

        /**
         * @brief Construct a new Web Server object.
         *
         * @param port The server port; default is 80.
         */
        explicit WebServer(uint16_t port = 80);
        /**
         * @brief Set up the web server.
         *
         * This registers URL handlers with the server.
         * @param on_save           Callback to invoke following handing a POST of /settings/set. If null, not called, but settings still saved in memory.
         * @param on_restart        Callback to invoke on a GET of /reboot. If null, the associated URL is not registered.
         * @param on_factory_reset  Callback to invoke on a GET of /factoryreset. If null, the associated URL is not registered.
         */
        void setup(const notify_t &on_save, const notify_t &on_restart, const notify_t &on_factory_reset);
        /**
         * @brief Loop handling.
         *
         * This presently does not do anything; it is provided for future use.
         *
         */
        void loop();
        /**
         * @brief Add a setting set.
         *
         * This registers a setting set which will be wrapped in a `SettingPanel` and presented
         * on the main page. The first set registered will be the default set shown when the
         * page is first loaded.
         *
         * Settings can be added after `setup` is called; current pages in browsers will not be updated, however..
         *
         * @param name          The name for the set; also the name shown on the tab.
         * @param setting_set   The set of settings. Held as a reference; do not destroy the object passed in.
         */
        void add_setting_set(const __FlashStringHelper *name, const SettingInterface::settings_list_t &setting_set);

        /**
         * @brief Get the server.
         *
         * Returns a reference to the contained AsyncWebServer.
         *
         * @return AsyncWebServer& reference.
         */
        AsyncWebServer &get_server()
        {
            return server;
        }

        /**
         * @brief Set the credentials for modifying operations.
         *
         * This applies to Save, Reboot, Factory Defaults, and Upload.
         * If the user is blank, no credentials are requested.
         *
         * A unique realm is used for every request, meaning
         * the browser should re-request authentication information
         * every time. This prevents credentials saved in the browser
         * session from being reused; a minor annoyance for the user
         * but a win for reboot, factory defaults, and upload firmware.
         *
         * @param user      User ID.
         * @param password  Password.
         */
        void set_credentials(const String &user, const String &password)
        {
            auth_user = user;
            auth_password = password;
            generate_new_authentication();
        }
    private:
        typedef std::list<std::unique_ptr<SettingPanel>> setting_panel_list_t;

        void on_not_found(AsyncWebServerRequest *request);          //!< Handle page not found; either 404 or 302 redirect, depending on SoftAP mode.
        void on_request_values(AsyncWebServerRequest *request);     //!< Handle a request for values.
        void on_request_upload(AsyncWebServerRequest *request);     //!< Handle a request to upload firmware. Presents a page to allow a file upload.

        /**
         * @brief Handle upload of a firmware file segment.
         *
         * This writes each segment to flash memory, using the Upload object. When complete,
         * it calls `on_restart` to request a system reset in the main loop.
         *
         * @param request   The upload request; can be used to sent responses.
         * @param filename  Filename. Not used.
         * @param index     Segment index. Used only to identify the first segment.
         * @param data      Block of data.
         * @param len       Length of data.
         * @param final     When `true`, this is the last block and the upload is complete.
         */
        void handle_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        /**
         * @brief Handle an upload failure.
         *
         * This sends an upload failure response via the request. This is a 500 status response.
         *
         * @param request   The upload request.
         */
        void on_update_failed(AsyncWebServerRequest *request);
        /**
         * @brief Handle the upload successful completion.
         *
         * Sends a response indicating the upload was successful, and then calls `on_restart`.
         *
         * @param request   The upload request.
         */
        void on_update_done(AsyncWebServerRequest *request);

        //!< States for the main page chunk transmission.
        enum class MainPageChunkState {
            BEGIN_PAGE,         //!< Sending the initial portion.
            TABBUTTON_HEADER,   //!< Sending the tab button header.
            TAB_BODY,           //!< Sending the tab bodies.
            FOOTER,             //!< Sending the footer.
            DONE                //!< Completed seting.
        };

        //!< This structure holds tracking context for sending the main page.
        struct MainPageChunkContext
        {
            MainPageChunkState state = MainPageChunkState::BEGIN_PAGE;          //!< The current state.
            setting_panel_list_t::const_iterator current_panel;                 //!< Where applicable, the panel being processed.
            SettingInterface::settings_list_t::const_iterator current_setting;  //!< Where applicable, the setting in the panel being processed.
            bool starting_tab = true;                                           //!< If `true`, a tab body is to be started.
        };

        /**
         * @brief Handle main page chunks.
         *
         * This writes each chunk into the buffer, and then returns the size written.
         *
         * @param buffer[in,out]    Buffer to receive output data.
         * @param maxLine           Maximum capacity of `buffer`.
         * @param index             Chunk buffer index; appears to be the number of bytes sent so far. Zero on first call. Not used.
         * @param context           Context for sending chunks.
         * @return Size of the chunk sent.
         */
        size_t on_main_page_chunk(uint8_t *buffer, size_t maxLen, size_t index, MainPageChunkContext *context);

        /**
         * @brief Handle a main page tab button chunk.
         *
         * This writes as a many complete tab buttons as possible to the buffer.
         * When it returns `true`, processing can move to the next state.
         *
         * @param buffer[in,out]    Buffer to receive output data.
         * @param maxLine           Maximum capacity of `buffer`.
         * @param size              Current used buffer capacity.
         * @param context           Context for sending chunks.
         * @return `true` if all tab buttons have been written to the buffer.
         */
        bool on_main_page_tabbutton_chunk(uint8_t *buffer, size_t maxLen, size_t &size, MainPageChunkContext &context);

        /**
         * @brief Handle a main page tab body chunk.
         *
         * This writes as a many complete tab bodies to the buffer. It
         * does so at a granularity of individual settings within the setting tab.
         * When it returns `true`, processing can move to the next state.
         *
         * @param buffer[in,out]    Buffer to receive output data.
         * @param maxLine           Maximum capacity of `buffer`.
         * @param size              Current used buffer capacity.
         * @param context           Context for sending chunks.
         * @return `true` if all tab panels have been written to the buffer.
         */
        bool on_main_page_tabbody_chunk(uint8_t *buffer, size_t maxLen, size_t &size, MainPageChunkContext &context);

        /**
         * @brief Handle the main page footer chunk.
         *
         * Unlike other processing, this requires all footer to fit in the buffer.
         * It will return `true` if all data has been written, and `false` otherwise.
         *
         * @param buffer[in,out]    Buffer to receive output data.
         * @param maxLine           Maximum capacity of `buffer`.
         * @param size              Current used buffer capacity.
         * @param context           Context for sending chunks.
         * @return `true` if the page footer was written to the buffer.
         */
        bool on_main_page_footer_chunk(uint8_t *buffer, size_t maxLen, size_t &size, MainPageChunkContext &context);

        /**
         * @brief Verify request authentication, if enabled.
         *
         * @param request Request that may be requiring authentication.
         * @return `true` if authentication is not needed, or if it succeeded.
         */
        bool verify_authentication(AsyncWebServerRequest *request);

        /**
         * @brief Generate a new authentication string.
         *
         * This is called when credentials are set, and after each
         * successful authentication.
         */
        void generate_new_authentication();

        notify_t on_save;           //!< The on-save callback. Can be null.
        notify_t on_restart;        //!< The on restart callback. Can be null.
        notify_t on_factory_reset;  //!< The on factory reset callback. Can be null.

        AsyncWebServer server;      //!< The web server.

        setting_panel_list_t setting_panels;    //!< The setting panels.

        String auth_user;           //!< The authentication name.
        String auth_password;       //!< The authentication password.
        char auth_realm[17];        //!< Set to a random string in the constructor, and after every successfull authentication.
        String last_auth_digest;    //!< Last authentication digest. Generated whenever auth_realm changes.

    };
}
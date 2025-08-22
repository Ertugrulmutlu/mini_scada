// utils.hpp içinden bir kesit: (class Utils {...} içine)
#include <string>
#include <cctype>
#include "pico/stdlib.h"
#include "hardware/adc.h"

class Utils {
public:
    inline static const char* DEVICE_ID = "PICO-FIELD-01";
    inline static uint32_t g_report_period_ms = 1000;
    inline static bool     g_auto_log         = false;

    static float read_internal_temp_c() {
        uint16_t raw = adc_read();
        float volts = raw * (3.3f / 4095.0f);
        return 27.0f - (volts - 0.706f) / 0.001721f;
    }

    static void print_status(bool with_prefix = true) {
        bool led_state = gpio_get(PICO_DEFAULT_LED_PIN);
        float t = read_internal_temp_c();
        if (with_prefix) printf("[STATUS] ");
        printf("id=%s temp=%.2fC led=%s period=%ums\r\n",
               DEVICE_ID, t, led_state ? "ON":"OFF", g_report_period_ms);
    }

    static void print_help() {
        printf(
            "Commands:\r\n"
            "  help               - show this help\r\n"
            "  id                 - print device id\r\n"
            "  status             - one-shot status (temp, led, period)\r\n"
            "  log on|off         - periodic status logging\r\n"
            "  rate <ms>          - set status/log period in ms (>=100)\r\n"
            "  led on|off|toggle  - control onboard LED\r\n"
            "  temp               - read temperature once\r\n"
            "\r\n"
        );
    }

private:
    // --- helpers ---
    static inline void ltrim(std::string& s){
        size_t i=0; while(i<s.size() && std::isspace((unsigned char)s[i])) ++i;
        s.erase(0,i);
    }
    static inline void rtrim(std::string& s){
        while(!s.empty() && std::isspace((unsigned char)s.back())) s.pop_back();
    }
    static inline void trim(std::string& s){ ltrim(s); rtrim(s); }

    static inline void split_cmd_arg(const std::string& line, std::string& cmd, std::string& arg){
        size_t sp = line.find_first_of(" \t");
        if (sp == std::string::npos) { cmd = line; arg.clear(); }
        else { cmd = line.substr(0, sp); arg = line.substr(sp+1); trim(arg); }
    }

public:
    static void handle_command(const std::string& raw_line) {
        std::string line = raw_line;
        trim(line);
        if (line.empty()) return;

        std::string cmd, arg;
        split_cmd_arg(line, cmd, arg);

        // normalize cmd to lower
        for (auto& c : cmd) c = (char)std::tolower((unsigned char)c);

        if (cmd == "help") {
            print_help();

        } else if (cmd == "id") {
            printf("%s\r\n", DEVICE_ID);

        } else if (cmd == "status") {
            print_status(false);

        } else if (cmd == "temp") {
            printf("%.2f\r\n", read_internal_temp_c());

        } else if (cmd == "led") {
            for (auto& c : arg) c = (char)std::tolower((unsigned char)c);
            if (arg == "on") {
                gpio_put(PICO_DEFAULT_LED_PIN, 1); printf("OK\r\n");
            } else if (arg == "off") {
                gpio_put(PICO_DEFAULT_LED_PIN, 0); printf("OK\r\n");
            } else if (arg == "toggle") {
                gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN)); printf("OK\r\n");
            } else {
                printf("ERR usage: led on|off|toggle\r\n");
            }

        } else if (cmd == "log") {
            for (auto& c : arg) c = (char)std::tolower((unsigned char)c);
            if (arg == "on")  { g_auto_log = true;  printf("OK log on\r\n"); }
            else if (arg == "off") { g_auto_log = false; printf("OK log off\r\n"); }
            else { printf("ERR usage: log on|off\r\n"); }

        } else if (cmd == "rate") {
            // rate <ms>
            if (arg.empty()) { printf("ERR usage: rate <ms>\r\n"); return; }
            unsigned ms = 0;
            // sadece pozitif sayı kabul et
            bool ok = true;
            for (char ch : arg) if (!std::isdigit((unsigned char)ch)) { ok=false; break; }
            if (ok) ms = (unsigned)std::stoul(arg);
            if (!ok || ms < 100) {
                printf("ERR ms must be >= 100\r\n");
            } else {
                g_report_period_ms = ms;
                printf("OK %u\r\n", ms);
            }

        } else {
            printf("ERR unknown cmd (type 'help')\r\n");
        }
    }
};

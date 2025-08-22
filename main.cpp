#include <cstdio>
#include <cstring>
#include <string>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"    
#include "hardware/adc.h"      
#include "utils.hpp"           
//----Config

int main(){
    stdio_init_all();

    while (!stdio_usb_connected()) { sleep_ms(100); }
    setvbuf(stdout, NULL, _IONBF,0);
    printf("BOOT OK\r\n");

    //boot adc
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // internal temp

    // boot led
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    printf("== Mini SCADA Console ==\r\n");
    printf("Device: %s\r\n", Utils::DEVICE_ID);
    Utils::print_help();


    absolute_time_t next_report = make_timeout_time_ms(Utils::g_report_period_ms);
    std::string buf; buf.reserve(128);


    while (true) //main loop
    {
        int ch = getchar_timeout_us(0); // non-blocking
        if(ch != PICO_ERROR_TIMEOUT){
            if (ch == '\r' || ch == '\n'){
                if(!buf.empty()){
                    Utils::handle_command(buf);
                    buf.clear();
                }
            }else if (buf.size() < 120) {
                buf.push_back(static_cast<char>(ch));
            } 
        }
        if (Utils::g_auto_log && absolute_time_diff_us(get_absolute_time(), next_report) <= 0) {
            Utils::print_status(true);
            next_report = make_timeout_time_ms(Utils::g_report_period_ms);
        }        
    }
}
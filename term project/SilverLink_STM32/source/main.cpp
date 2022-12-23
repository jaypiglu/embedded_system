/* Sockets Example
 * Copyright (c) 2016-2020 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "Thread.h"
#include "Callback.h"
#include "stdio.h"
#include "math.h"
#include "wifi_helper.h"
#include "mbed-trace/mbed_trace.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"



#if MBED_CONF_APP_USE_TLS_SOCKET
#include "root_ca_cert.h"

#ifndef DEVICE_TRNG
#error "mbed-os-example-tls-socket requires a device which supports TRNG"
#endif
#endif // MBED_CONF_APP_USE_TLS_SOCKET

InterruptIn button(BUTTON1);
volatile int sos_status = 0;
volatile int heartrate = 0;
Thread t;

void button_pressed(){
    sos_status = 1;
}

void button_released(){
    sos_status = 0;
}

AnalogIn pulse_sensor(A1);

void h() {
    //uint16_t samples[1024];
    //cout<<endl;
    clock_t b;
    const int samples = 5;  // 樣本數
    const int sample_rate = 1000;  // 採樣率（每秒採樣次數）
    float pulse_val;  // pulse sensor的值
    int adc_val;  // pulse sensor的ADC值
    int pulse_sum = 0;  // pulse sensor的總和
    int pulse_sum2 = 0;
    int pulse_count = 0;  // pulse sensor的樣本數
    int pulse=0;
    int ex_pulse=0;
    int num=20000;
    int pulse_nxt[1000]={0};
    int n=0;
    int count=1;
    int output=0;
    int heart=0;
    int th=2070;
    int counter=0;
    int flag=0;
    //int samples[1024];
    while(1){
    while (pulse_count < samples) {
        pulse_val = pulse_sensor.read();
        adc_val = pulse_val * 4096;
        pulse_sum += adc_val;

        pulse_count++;

        
        //wait(1000 / sample_rate);
        ThisThread::sleep_for(10);
    }
    ex_pulse=pulse;
    

    //pulse_nxt[n] = pulse_sum / pulse_count;
    pulse = pulse_sum / pulse_count;

    if(ex_pulse<th &&pulse>th && count>5 ){
        printf("no average%d\n",1200/count);
        heart = 1200/count;
        
        if(heart < 50){
            th -= 2;
            if(flag==1){
                output+=count;
                counter++;
            }
            flag=1;
        }
        else if(heart > 100){
            th += 1;
            flag=1;
            if(flag==1){
                output+=count;
                counter++;
            }
        }
        else{
            flag=0;
            output+=count;
            counter++;
        }
        if(counter==5){
            printf("average %d\n",6000/output);
            heartrate = 6000/output;
            counter=0;
            output=0;
        }
        count=0;
    }
    count++;
    pulse_count=0;
    pulse_sum=0;
    ++n;
    //printf("%d, ", pulse);

    //ThisThread::sleep_for (100);
    }
}

void heartratemonitor() {
    //uint16_t samples[1024];
    clock_t b;
    const int samples = 5;  // 樣本數
    const int sample_rate = 1000;  // 採樣率（每秒採樣次數）
    float pulse_val;  // pulse sensor的值
    int adc_val;  // pulse sensor的ADC值
    int pulse_sum = 0;  // pulse sensor的總和
    int pulse_sum2 = 0;
    int pulse_count = 0;  // pulse sensor的樣本數
    int pulse=0;
    int ex_pulse=0;
    int num=10000;
    int pulse_nxt[1000]={0};
    int n=0;
    int count=1;
    int th=2070;
    //int samples[1024];

    while(1){
    while (pulse_count < samples) {
        pulse_val = pulse_sensor.read();
        adc_val = pulse_val * 4096;
        pulse_sum += adc_val;

        pulse_count++;

        
        //wait(1000 / sample_rate);
        ThisThread::sleep_for(10);
    }
    ex_pulse=pulse;
    

    pulse_nxt[n] = pulse_sum / pulse_count;
    pulse = pulse_sum / pulse_count;
    
    if(ex_pulse<th &&pulse>th && count>5 ){
        heartrate = 1200/count;
        printf("%d\n", heartrate);
        if(heartrate < 50){
            th -= 1;
        }
        if(heartrate > 110){
            th += 1;
        }
        count=0;
    }

    count++;
    pulse_count=0;
    pulse_sum=0;
    ++n;
    //printf("%d, ", pulse);

    //ThisThread::sleep_for (100);
    }
}

class SocketDemo {
    static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 50;
    static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

#if MBED_CONF_APP_USE_TLS_SOCKET
    static constexpr size_t REMOTE_PORT = 443; // tls port
#else
    static constexpr size_t REMOTE_PORT = 80; // standard HTTP port
#endif // MBED_CONF_APP_USE_TLS_SOCKET

public:
    SocketDemo() : _net(NetworkInterface::get_default_instance())
    {
    }

    ~SocketDemo()
    {
        if (_net) {
            _net->disconnect();
        }
    }

    void run()
    {
        if (!_net) {
            printf("Error! No network interface found.\r\n");
            return;
        }

        /* if we're using a wifi interface run a quick scan */
        if (_net->wifiInterface()) {
            /* the scan is not required to connect and only serves to show visible access points */
            wifi_scan();

            /* in this example we use credentials configured at compile time which are used by
             * NetworkInterface::connect() but it's possible to do this at runtime by using the
             * WiFiInterface::connect() which takes these parameters as arguments */
        }

        /* connect will perform the action appropriate to the interface type to connect to the network */

        printf("Connecting to the network...\r\n");

        nsapi_size_or_error_t result = _net->connect();
        if (result != 0) {
            printf("Error! _net->connect() returned: %d\r\n", result);
            return;
        }

        print_network_info();

        /* opening the socket only allocates resources */
        result = _socket.open(_net);
        if (result != 0) {
            printf("Error! _socket.open() returned: %d\r\n", result);
            return;
        }

#if MBED_CONF_APP_USE_TLS_SOCKET
        result = _socket.set_root_ca_cert(root_ca_cert);
        if (result != NSAPI_ERROR_OK) {
            printf("Error: _socket.set_root_ca_cert() returned %d\n", result);
            return;
        }
        _socket.set_hostname(MBED_CONF_APP_HOSTNAME);
#endif // MBED_CONF_APP_USE_TLS_SOCKET

        /* now we have to find where to connect */

        SocketAddress address("HOST IP address");
        address.set_port(6531);

        result = _socket.connect(address);

        int sample_num=0;
        char acc_json[1000];
        int16_t pDataXYZ[3] = {0};
        float pGyroDataXYZ[3] = {0};
        int16_t acc = 0;

        BSP_ACCELERO_Init();
        BSP_GYRO_Init();

        while (1){
            
            BSP_ACCELERO_AccGetXYZ(pDataXYZ);
            BSP_GYRO_GetXYZ(pGyroDataXYZ);

            acc = sqrt(pow(pDataXYZ[0],2)+pow(pDataXYZ[1],2)+pow(pDataXYZ[2],2));


            /*printf("\nACCELERO_X = %d\n", pDataXYZ[0]);
            printf("ACCELERO_Y = %d\n", pDataXYZ[1]);
            printf("ACCELERO_Z = %d\n", pDataXYZ[2]);
            printf("acc = %d\n", acc);
            printf("\nGYRO_X = %.2f\n", pGyroDataXYZ[0]);
            printf("GYRO_Y = %.2f\n", pGyroDataXYZ[1]);
            printf("GYRO_Z = %.2f\n", pGyroDataXYZ[2]);*/
            int len = sprintf(acc_json,
            "{\"sos\":%d,\"acc\":%d,\"acc_x\":%d,\"acc_y\":%d,\"acc_z\":%d, \"gyro_x\":%.2f,\"gyro_y\":%.2f,\"gyro_z\":%.2f,\"heartrate\":%d}",
            sos_status,acc, pDataXYZ[0], pDataXYZ[1], pDataXYZ[2],pGyroDataXYZ[0], pGyroDataXYZ[1], pGyroDataXYZ[2], heartrate);
            
            
            int response = _socket.send(acc_json,len);
            ThisThread::sleep_for(500);
        } 



        printf("Demo concluded successfully \r\n");
    }

private:
    bool resolve_hostname(SocketAddress &address)
    {
        const char hostname[] = MBED_CONF_APP_HOSTNAME;

        /* get the host address */
        printf("\nResolve hostname %s\r\n", hostname);
        nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);
        if (result != 0) {
            printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
            return false;
        }

        printf("%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None") );

        return true;
    }

    bool send_http_request()
    {
        /* loop until whole request sent */
        const char buffer[] = "GET / HTTP/1.1\r\n"
                              "Host: ifconfig.io\r\n"
                              "Connection: close\r\n"
                              "\r\n";

        nsapi_size_t bytes_to_send = strlen(buffer);
        nsapi_size_or_error_t bytes_sent = 0;

        printf("\r\nSending message: \r\n%s", buffer);

        while (bytes_to_send) {
            bytes_sent = _socket.send(buffer + bytes_sent, bytes_to_send);
            if (bytes_sent < 0) {
                printf("Error! _socket.send() returned: %d\r\n", bytes_sent);
                return false;
            } else {
                printf("sent %d bytes\r\n", bytes_sent);
            }

            bytes_to_send -= bytes_sent;
        }

        printf("Complete message sent\r\n");

        return true;
    }

    bool receive_http_response()
    {
        char buffer[MAX_MESSAGE_RECEIVED_LENGTH];
        int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;
        int received_bytes = 0;

        /* loop until there is nothing received or we've ran out of buffer space */
        nsapi_size_or_error_t result = remaining_bytes;
        while (result > 0 && remaining_bytes > 0) {
            result = _socket.recv(buffer + received_bytes, remaining_bytes);
            if (result < 0) {
                printf("Error! _socket.recv() returned: %d\r\n", result);
                return false;
            }

            received_bytes += result;
            remaining_bytes -= result;
        }

        /* the message is likely larger but we only want the HTTP response code */

        printf("received %d bytes:\r\n%.*s\r\n\r\n", received_bytes, strstr(buffer, "\n") - buffer, buffer);

        return true;
    }

    void wifi_scan()
    {
        WiFiInterface *wifi = _net->wifiInterface();

        WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

        /* scan call returns number of access points found */
        int result = wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

        if (result <= 0) {
            printf("WiFiInterface::scan() failed with return value: %d\r\n", result);
            return;
        }

        printf("%d networks available:\r\n", result);

        for (int i = 0; i < result; i++) {
            printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n",
                   ap[i].get_ssid(), get_security_string(ap[i].get_security()),
                   ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
                   ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5],
                   ap[i].get_rssi(), ap[i].get_channel());
        }
        printf("\r\n");
    }

    void print_network_info()
    {
        /* print the network info */
        SocketAddress a;
        _net->get_ip_address(&a);
        printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_netmask(&a);
        printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_gateway(&a);
        printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    }

private:
    NetworkInterface *_net;

#if MBED_CONF_APP_USE_TLS_SOCKET
    TLSSocket _socket;
#else
    TCPSocket _socket;
#endif // MBED_CONF_APP_USE_TLS_SOCKET
};

int main() {
    printf("\r\nStarting socket demo\r\n\r\n");

#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
#endif

    button.fall(&button_pressed); // Change led delay
    button.rise(&button_released); // Change led
    t.start(heartratemonitor);
    SocketDemo *example = new SocketDemo();
    MBED_ASSERT(example);
    example->run();

    return 0;
}
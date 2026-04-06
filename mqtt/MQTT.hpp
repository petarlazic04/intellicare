#pragma once

#include <mosquitto.h>
#include <string>
#include <functional>
#include <chrono>
#include <iostream>

class MQTT {
public:
    using MsgHandler = std::function<void(const std::string& topic, const std::string& msg)>;

    MQTT(const std::string& broker, int port = 1883) 
        : broker(broker), port(port), is_connected(false) {
        
        mosquitto_lib_init();
        
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        client_id = "cpp_mqtt_" + std::to_string(now);
        
        mosq = mosquitto_new(client_id.c_str(), true, this);
        
        mosquitto_connect_callback_set(mosq, on_connect_wrapper);
        mosquitto_disconnect_callback_set(mosq, on_disconnect_wrapper);
        mosquitto_message_callback_set(mosq, on_message_wrapper);
    }

    ~MQTT() {
        if (is_connected) disconnect();
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    }

    bool connect() {
        int rc = mosquitto_connect(mosq, broker.c_str(), port, 60);
        if (rc != MOSQ_ERR_SUCCESS) {
            std::cerr << "Connection failed: " << mosquitto_strerror(rc) << std::endl;
            return false;
        }
        
        mosquitto_loop_start(mosq);
        return true;
    }

    void disconnect() {
        mosquitto_loop_stop(mosq, true);
        mosquitto_disconnect(mosq);
        is_connected = false;
    }

    bool connected() const {
        return is_connected;
    }

    void on_message(MsgHandler handler) {
        msg_handler = handler;
    }

    void subscribe(const std::string& topic, int qos = 0) {
        mosquitto_subscribe(mosq, nullptr, topic.c_str(), qos);
    }

    void publish(const std::string& topic, const std::string& msg, int qos = 0) {
        mosquitto_publish(mosq, nullptr, topic.c_str(), msg.length(), 
                          (const uint8_t*)msg.c_str(), qos, false);
    }

    void loop(int timeout_ms = 100) {
        mosquitto_loop(mosq, timeout_ms, 1);
    }

private:
    struct mosquitto *mosq;
    MsgHandler msg_handler;
    std::string broker, client_id;
    int port;
    bool is_connected;

    static void on_connect_wrapper(struct mosquitto *m, void *obj, int rc) {
        MQTT *mqtt = (MQTT *)obj;
        if (rc == 0) {
            mqtt->is_connected = true;
            std::cout << "Connected to broker" << std::endl;
        } else {
            std::cout << "Connection failed: " << mosquitto_connack_string(rc) << std::endl;
        }
    }

    static void on_disconnect_wrapper(struct mosquitto *m, void *obj, int rc) {
        MQTT *mqtt = (MQTT *)obj;
        mqtt->is_connected = false;
        std::cout << "Disconnected" << std::endl;
    }

    static void on_message_wrapper(struct mosquitto *m, void *obj, const struct mosquitto_message *msg) {
        MQTT *mqtt = (MQTT *)obj;
        if (mqtt->msg_handler) {
            std::string topic(msg->topic);
            std::string payload((char *)msg->payload, msg->payloadlen);
            mqtt->msg_handler(topic, payload);
        }
    }
};
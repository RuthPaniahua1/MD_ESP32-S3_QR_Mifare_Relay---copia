#ifndef MQTT_CONNECT
#define MQTT_CONNECT

#include <PubSubClient.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>

using namespace std;

class MqttConnect
{
  private:

  public:
    const IPAddress MQTT_HOST();
    const int MQTT_PORT = 1883;
    
    AsyncMqttClient mqttClient;

};
#endif
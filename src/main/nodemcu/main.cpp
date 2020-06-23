#ifdef NODEMCU
#ifndef UNIT_TEST

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "parser/parser.h"
#include "engine/engine.h"
#include "debug.h"
#include "network.h"
#include "commands.h"
#include "jsonMessage.h"

// ================
// ENGINE PINS
// ================

// https://www.youtube.com/watch?v=7h2bE2vNoaY

constexpr uchar PIN_RIGHT_FRONT = D0;
constexpr uchar PIN_RIGHT_BACK = D1;
constexpr uchar PIN_LEFT_FRONT = D3;
constexpr uchar PIN_LEFT_BACK = D2;

constexpr uchar PIN_LEFT_SPEED = D4;
constexpr uchar PIN_RIGHT_SPEED = D5;

constexpr uint DEF_SPEED = 500U;
constexpr uint SPEED_CHANGE_INTERVAL = 5U;
constexpr uchar SPEED_CHANGE_DOWN = 0U;
constexpr uchar SPEED_CHANGE_STEADY = 1U;
constexpr uchar SPEED_CHANGE_UP = 2U;

uchar changingSpeed = 1U;

// ================
// VARIABLES
// ================

Engine left(PIN_LEFT_FRONT, PIN_LEFT_BACK, PIN_LEFT_SPEED, DEF_SPEED);
Engine right(PIN_RIGHT_FRONT, PIN_RIGHT_BACK, PIN_RIGHT_SPEED, DEF_SPEED);
Parser parser;

WiFiClient espClient;
PubSubClient client(espClient);

char BUFFER[300];
constexpr size_t TEMPERATURES = 2U;
constexpr size_t REST = 13U;


// ================
// SETTING UP PINS
// ================

static void InitEnginePins()
{
  pinMode(PIN_RIGHT_FRONT, OUTPUT);
  pinMode(PIN_RIGHT_BACK, OUTPUT);
  pinMode(PIN_LEFT_SPEED, OUTPUT);
  pinMode(PIN_LEFT_FRONT, OUTPUT);
  pinMode(PIN_LEFT_BACK, OUTPUT);
  pinMode(PIN_RIGHT_SPEED, OUTPUT);
}

// ==============
// FUNCTIONS TO EVENTS
// ==============

void UpdateSpeed()
{
  static unsigned long lastUpdate = millis();
  if (millis() - lastUpdate > SPEED_CHANGE_INTERVAL)
  {
    if (changingSpeed == SPEED_CHANGE_DOWN)
    {
      if (left.CurrentSpeed() > 0U)
        left.ChangeSpeed(left.CurrentSpeed() - 1);
      if (right.CurrentSpeed() > 0U)
        right.ChangeSpeed(right.CurrentSpeed() - 1);
      //client.publish(Network::CALLBACK_TOPIC, "Slower");
    }
    else if (changingSpeed == SPEED_CHANGE_UP)
    {
      left.ChangeSpeed(left.CurrentSpeed() + 1);
      right.ChangeSpeed(right.CurrentSpeed() + 1);
      //client.publish(Network::CALLBACK_TOPIC, "Faster");
    }
    lastUpdate = millis();
  }
}

void MoveToNano(const CommandBuffer &b)
{
  const char *commandToSend = b.WordAt(1);
  if (commandToSend)
  {
    // get index of message we want to send
    size_t index = (size_t)(commandToSend - b.C_Ptr());
    // send entire messege
    for (size_t i = index; i < b.Length(); i++)
    {
      char toSend = b.C_Ptr()[i];
      if (toSend == b.NULL_CHAR)
        Serial.write(' ');
      else
        Serial.write(toSend);
    }
    // to stop communation
    Serial.write('\n');
  }
}

auto ForwardLeftFun = [](const CommandBuffer &b) { left.Forward(); };
auto ForwardRightFun = [](const CommandBuffer &b) { right.Forward(); };

auto BackwardLeftFun = [](const CommandBuffer &b) { left.Backward(); };
auto BackwardRightFun = [](const CommandBuffer &b) { right.Backward(); };

auto StopLeftFun = [](const CommandBuffer &b) { left.Stop(); };
auto StopRightFun = [](const CommandBuffer &b) { right.Stop(); };

auto FasterFun = [](const CommandBuffer &b) { changingSpeed = SPEED_CHANGE_UP; };
auto SlowerFun = [](const CommandBuffer &b) { changingSpeed = SPEED_CHANGE_DOWN; };
auto SteadyFun = [](const CommandBuffer &b) { changingSpeed = SPEED_CHANGE_STEADY; };

// ==============
// WIFI
// ==============

void Callback(const char *topic, byte *payload, unsigned int length)
{
  for (size_t i = 0; i < length; i++)
    parser.GetBuff().PushBack(payload[i]);

  parser.ExecuteMessege();

#if DEBUG
  client.publish(Network::CALLBACK_TOPIC, "Messege received!");
#endif
}

void Reconnect()
{
  while (!client.connected())
  {
    if (client.connect(Network::ID))
      client.subscribe(Network::SUBSCRIBE_TOPIC);
    delay(5000);
  }
  client.publish(Network::DEBUG_TOPIC, "Tank has been connected!");

  LOG_NL("Connected to broker");
}

void SendOverMQTT(const CommandBuffer &b)
{

  Float temperetures[TEMPERATURES];
  Integer rest[REST];

  for (size_t i = 0; i < TEMPERATURES + REST; i++)
  {
    if (i < TEMPERATURES)
    {
      // +1 to ignore the command
      temperetures[i] = b.FloatAt(i + 1);
    }
    else
    {
      // -2 to offset temperatures
      rest[i - TEMPERATURES] = b.IntAt(i + 1);
    }
  }
  // check if all numbers were succesfully read
  bool validData = true;

  for (size_t i = 0; i < TEMPERATURES; i++)
    if (!temperetures[i].success)
      validData = false;

  for (size_t i = 0; i < REST; i++)
    if (!rest[i].success)
      validData = false;

  if (validData)
  {
    // format message
    sprintf(BUFFER, JSON_MESSAGE,
            temperetures[0].value,
            temperetures[1].value,
            rest[0].value,
            rest[1].value,
            rest[2].value,
            rest[3].value,
            rest[4].value,
            rest[5].value,
            rest[6].value,
            rest[7].value,
            rest[8].value,
            rest[9].value,
            rest[10].value,
            rest[11].value,
            rest[12].value,
            left.Direction(),
            right.Direction(),
            left.CurrentSpeed(),
            right.CurrentSpeed());

    // and finally send it over mqtt
    client.publish(Network::STATUS_TOPIC, BUFFER);
  }
}

// ==============
// SETUP AND LOOP
// ==============

void setup()
{
  InitEnginePins();
  client.setServer(Network::MQTT_SERVER, Network::MQTT_PORT);
  client.setCallback(Callback);
  WiFi.mode(WIFI_STA);
  WiFi.begin(Network::SSID, Network::PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  // move command to nano
  parser.AddEvents(Command::Mcu::MOVE, MoveToNano);
  parser.AddEvents(Command::Mcu::TANK_FORWARD_L, ForwardLeftFun);
  parser.AddEvents(Command::Mcu::TANK_FORWARD_R, ForwardRightFun);
  parser.AddEvents(Command::Mcu::TANK_BACKWARD_L, BackwardLeftFun);
  parser.AddEvents(Command::Mcu::TANK_BACKWARD_R, BackwardRightFun);
  parser.AddEvents(Command::Mcu::TANK_STOP_L, StopLeftFun);
  parser.AddEvents(Command::Mcu::TANK_STOP_R, StopRightFun);

  parser.AddEvents(Command::Mcu::TANK_FASTER, FasterFun);
  parser.AddEvents(Command::Mcu::TANK_SLOWER, SlowerFun);
  parser.AddEvents(Command::Mcu::TANK_STEADY, SteadyFun);
  parser.AddEvents(Command::Mcu::SEND, SendOverMQTT);

  Serial.begin(115200);
}

void loop()
{
  client.loop();
  if (!client.connected())
  {
    left.Stop();
    right.Stop();
    Reconnect();
  }
  UpdateSpeed();

  if (parser.ReadStream(&Serial))
    parser.ExecuteMessege();
}

#endif // UNIT_TEST
#endif // NODEMCU
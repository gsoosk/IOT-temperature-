
//*************************************************************** Digital I/O ******************************************************************#

#define STATUS_LED D3

#define ON 1
#define OFF 0
#define LED1 3
#define LED2 4
#define LED3 5
#define OUT4  4
#define OUT3  5
#define OUT2  6
#define OUT1  7

int ledState; 
unsigned long ledCurrentMillis;
const long interval = 1000; 
void builtin_led_init()
{
  pinMode(LED_BUILTIN,OUTPUT); 
//   pinMode(LED1,OUTPUT); 
//    pinMode(LED2,OUTPUT); 
//     pinMode(LED3,OUTPUT); 
     
  delay(10);
  digitalWrite(LED_BUILTIN,LOW);
}

void status_led_init()
{
  pinMode(STATUS_LED,OUTPUT); 
  int ledState = LOW; 
  digitalWrite(STATUS_LED, ledState);
}

void blink_status_led()
{
 
    digitalWrite(STATUS_LED, !ledState);
    ledState = !ledState;
    timer0_write(ESP.getCycleCount() + 80000000); //8Mhz -> 8*10^6 = 1 second
   
  
  
//  if (millis() > ledCurrentMillis + interval) {
//    ledCurrentMillis = millis();
//    if (ledState == LOW) {
//      ledState = HIGH;
//    } else {
//      ledState = LOW;
//    }
//    digitalWrite(STATUS_LED, ledState);
//  }
}

void status_led_task()
{
  blink_status_led();
}

//********************************************************* serial communication ******************************************************************#

#define BAUD_RATE 115200

void serial_communication_init()
{
  Serial.begin(BAUD_RATE);
}

//********************************************************** Temperature Sensor ******************************************************************#

#define ANALOG_INPUT A0
#define ADC_REFERENCE_VOLTAGE 3300 //mv
#define ADC_RESOLUTION 1023.0
#define OFFSET_VALUE -15

float temperature_value_celsius;

void calculate_temperature()
{
  temperature_value_celsius = (float) analogRead(ANALOG_INPUT) * 500.0 / 1023.0  + OFFSET_VALUE;
}

void send_temperature_to_pc()
{
  Serial.println(temperature_value_celsius);
}

void temperature_task()
{
  calculate_temperature();
  send_temperature_to_pc();
}

//************************************************************* shift register ******************************************************************#


#define CLK D0
#define LATCH D1
#define SERIAL_DATA D2

#define SET 1
#define RESET 0

bool output[8];

void shift_register()
{
  for(int i=0; i<8; i++)
  {
    digitalWrite(SERIAL_DATA, output[i]);
    digitalWrite(CLK, RESET);
    digitalWrite(CLK, SET);
    
  }
  digitalWrite(LATCH, RESET);
  digitalWrite(LATCH, SET);
  
}

void shift_register_init()
{

  
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);

  for(int i=0; i<8; i++)
    output[i] = RESET;
  
  shift_register();
  
  
}

void set_shift_register_output(int output_id,bool output_state)
{
  output[output_id] = output_state;
  shift_register();
  
}

//****************************************************************** Wifi ************************************************************************#


#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define WIFI_NAME "Farzad"
#define WIFI_PASS "13771377"

WiFiClient client;

void wifi_init()
{
  // Set WiFi to station mode and disconnect from an AP if it was Previously
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  //attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(WIFI_NAME);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}


//*************************************************************** Thing Speak ******************************************************************#

#include "ThingSpeak.h"

#define CHANNEL_WRITE_API_KEY "3K51H4STGKFBM0V8"

unsigned long channel_number;
unsigned int channel_field;

long thing_speak_post_delay;
long thing_speak_last_post_time = 0;  

void thing_speak_init()
{
  channel_number = 400681;
  channel_field = 1;

  thing_speak_post_delay=9; //9000 ms = 15 min
  
  ThingSpeak.begin(client);
}

void send_temperature_to_thing_speak()
{
 if (millis()  > thing_speak_last_post_time + thing_speak_post_delay)  
  {

 
    String temp_string = String(temperature_value_celsius,3);
    ThingSpeak.writeField(channel_number, channel_field, temp_string, CHANNEL_WRITE_API_KEY);
    thing_speak_last_post_time = millis() ;
    
  }
}

void thing_speek_task()
{
  send_temperature_to_thing_speak();
}

//****************************************************************** Telegram ******************************************************************

#include <UniversalTelegramBot.h>

#define BOT_TOKEN "//your code goes here"  




UniversalTelegramBot bot(BOT_TOKEN, client);

int telegram_seen_delay;
long telegram_last_seen;  
bool start;

void telegram_init()
{
  telegram_seen_delay = 1000; //ms
  telegram_last_seen=0;
  start = false;
}

void handle_new_messages(int numNewMessages) 
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
 
  for (int i=0; i<numNewMessages; i++) 
  {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    
    if (text == "/start") 
    {
     bot.sendChatAction(chat_id, "typing");
     delay(500);
     bot.sendMessage(chat_id, "Ready :))");
    }
    else if (text == "/read_temp") 
    {
     String temp_string = String(temperature_value_celsius,3);
     bot.sendMessage(chat_id, temp_string, "");
    }
    else if (text == "/relay_on") 
    {
     set_shift_register_output(3,SET);
     bot.sendMessage(chat_id, "RELAY is ON!", "");
     
    }
    else if (text == "/relay_off") 
    {
     set_shift_register_output(3,RESET);
     bot.sendMessage(chat_id, "RELAY is OFF!", "");
     
    }
    else if(text == "/led_1_on")
    {
      digitalWrite(LED1, HIGH);
      set_shift_register_output(LED1,SET);
      bot.sendMessage(chat_id, "LED1 is ON!", "");
    }
    else if(text == "/led_2_on")
    {
      digitalWrite(LED2, HIGH);
      set_shift_register_output(LED2,SET);
      bot.sendMessage(chat_id, "LED2 is ON!", "");
    }
    else if(text == "/led_3_on")
    {
      digitalWrite(LED3, HIGH);
      set_shift_register_output(LED3,SET);
      bot.sendMessage(chat_id, "LED3 is ON!", "");
    }
    else if(text == "/led_1_off")
    {
      digitalWrite(LED1, HIGH);
      set_shift_register_output(LED1,RESET);
      bot.sendMessage(chat_id, "LED1 is OFF!", "");
    }
    else if(text == "/led_2_off")
    {
      digitalWrite(LED2, HIGH);
      set_shift_register_output(LED2,RESET);
      bot.sendMessage(chat_id, "LED2 is OFF!", "");
    }
    else if(text == "/led_3_off")
    {
      digitalWrite(LED3, HIGH);
      set_shift_register_output(LED3,RESET);
      bot.sendMessage(chat_id, "LED3 is OFF!", "");
    }
    else if (text == "/out_1_on")
    {
      set_shift_register_output(OUT1, HIGH);
      bot.sendMessage(chat_id, "OUT1 is ON!");
    }
    else if (text == "/out_1_off")
    {
      set_shift_register_output(OUT1, LOW);
      bot.sendMessage(chat_id, "OUT1 is OFF!");
    }
    else if (text == "/out_2_on")
    {
      set_shift_register_output(OUT2, HIGH);
      bot.sendMessage(chat_id, "OUT2 is ON!");
    }
    else if (text == "/out_2_off")
    {
      set_shift_register_output(OUT2, LOW);
      bot.sendMessage(chat_id, "OUT2 is OFF!");
    }
    else if (text == "/out_3_on")
    {
      set_shift_register_output(OUT3, HIGH);
      bot.sendMessage(chat_id, "OUT3 is ON!");
    }
    else if (text == "/out_3_off")
    {
      set_shift_register_output(OUT3, LOW);
      bot.sendMessage(chat_id, "OUT3 is OFF!");
    }
    else if (text == "/out_4_on")
    {
      set_shift_register_output(OUT4, HIGH);
      bot.sendMessage(chat_id, "OUT4 is ON!");
    }
    else if (text == "/out_4_off")
    {
      set_shift_register_output(OUT4, LOW);
      bot.sendMessage(chat_id, "OUT4 is OFF!");
    }
    else
    {
      bot.sendMessage(chat_id, "YOU USED WRONG COMMAND", "");
    }
  }
}

void telegram_task()
{
  if (millis() > telegram_last_seen + telegram_seen_delay)  
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
 
    while(numNewMessages) {
      Serial.println("got response");
      handle_new_messages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
 
    telegram_last_seen = millis();
  }
}

//************************************************************** Timer0 and Interrupt **********************************************************

void timer1_init()
{
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(blink_status_led);
  //timer0_write(INTERRUPT_TIME);
  timer0_write(ESP.getCycleCount() + 80000000); //80Mhz -> 80*10^6 = 1 second
  interrupts();
}

//******************************************************** WatchDog timer **********************************************************


#define _500MILISEC   WDTO_500MS
#define _2SEC         WDTO_2S
#define _4SEC         WDTO_2S
#define _8SEC         WDTO_8S

void watch_dog_timer_init()
{
  wdt_enable(_4SEC);
}

void watch_dog_timer_task()
{
  wdt_reset();
}

//****************************************************************** Setup **********************************************************************

void setup() 
{
  serial_communication_init();
  wifi_init();
  telegram_init();
  thing_speak_init();
  shift_register_init();
  builtin_led_init();
  status_led_init();
  timer1_init();
  watch_dog_timer_init();
}
//****************************************************************** Main loop ****************************************************************** 
void loop() 
{
  status_led_task();
  temperature_task();
  telegram_task();
  thing_speek_task();
  watch_dog_timer_task;
   
  
}

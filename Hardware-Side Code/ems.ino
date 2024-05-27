#include "Ticker.h"
#include "HTTPClient.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include <queue> // data struct implementation
#include "ACS712.h"


WiFiSSLClient client;
HTTPClient httpClient;

const char* ssid = _SSID;
const char* password = _PASS;
const char* MONGODB_API_KEY = API_KEY;
const char* collection = COLLECTIONS;
const char* database = DATABASE;
const char* dataSource = DATASOURCE;

// BUTTON SETTINGS
#define DEBOUNCE_DELAY 3000 //ms [ this is to set a delay of 3 seconds to prevent spam whenever a new data is pushed in to queue ]

// Variables that will change values:

unsigned long lastDebounceTime = 0; // to know when was the last time it pushed data to queue to prevent spam insertion
// end

const double rate_per_kwh = 0.12; // assuming the rate is .12 pesos

void insertToMongoDB();
//params: function, timer in Milliseconds
Ticker pushToMongo(insertToMongoDB, 7000, 0); // a ticker(scheduler) to push the collected data from the queue into MongoDB collection

// a way to group several related variables into one place. each variable in the structure is known as a member of the structure.
// unlike an array, a structure can contain many different data types
struct HttpRequest
{
  String current;
  String voltage;
  String power;
  String energy;
  String cost;
};

std::queue<HttpRequest> requestQueue; // initialize queue

//  Arduino UNO has 5.0 volt with a max ADC value of 1023 steps
//  ACS712 5A  uses 185 mV per A
//  ACS712 20A uses 100 mV per A
//  ACS712 30A uses  66 mV per A
ACS712  ACS(A0, 5.0, 1023, 100); // A0 pin, 5V, 1023 analog to digital convert, 100mV sensitivity

void setup()
{
  Serial.begin(115200);

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(1000);
      Serial.print(".");
  }
  Serial.println();
  Serial.println("Successfully connected to WiFi!");

  ACS.autoMidPoint(); // calibrate

  // attach tickers/timers
  pushToMongo.start(); // pushes data collected from the queue into MongoDB every 5 sec.
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected!");
    return;
  }
  pushToMongo.update(); // updater to know what time it has pushed data to mongo 

  unsigned long currTime = millis();
  if ((currTime - lastDebounceTime) >= DEBOUNCE_DELAY) // made to check for every 2 seconds interval to prevent spam insertion to mongodb
  {
    int curr_mA = ACS.mA_DC(); // get data handled by the library
    //Serial.print("Current mA: ");
    Serial.println(curr_mA);

    if (curr_mA > 1)
    {
      double curr_A = curr_mA / 1000.0; // convert miliAmperes to Amperes

      double power = 220 * curr_A;  // assuming in 220V  //in watts // P = V * I
      double energy = power / 1000;  // convert to kWh assuming 1 hour interval
      double cost = energy * rate_per_kwh; // cost based on the energy consumption

      Serial.print("Current: ");
      Serial.println(curr_A);

      Serial.print("Power: ");
      Serial.println(power);
      
      Serial.print("Energy: ");
      Serial.println(energy);

      Serial.print("Cost: ");
      Serial.println(cost);

      pushData(String(curr_A), "220", String(power), String(energy), String(cost));
    }
    lastDebounceTime = currTime;
  }

  delayMicroseconds(50);
}

void pushData(const String& cu, const String& v, const String& p, const String& e, const String& c)
{
  HttpRequest newRequest;
  newRequest.current = cu;
  newRequest.voltage = v;
  newRequest.power = p;
  newRequest.energy = e;
  newRequest.cost = c;

  requestQueue.push(newRequest);
  Serial.println("A new data pushed to queue");
  //Serial.println(data);
}

/*
 * @Description: triggers every 5 seconds to push all data together that is in queue
 * when tiggered, it checks if the queue is not empty, then checks the front data of the queue, then post request, and pop. Repeats until cleared.
 */
void insertToMongoDB()
{
  while (!requestQueue.empty())
  {
    HttpRequest nextRequest = requestQueue.front();

    StaticJsonDocument<200> doc;
    doc["collection"] = collection;
    doc["database"] = database;
    doc["dataSource"] = dataSource;

    JsonObject document = doc.createNestedObject("document");
    document["ApplianceName"] = "9V Battery";
    document["current"] = nextRequest.current;
    document["voltage"] = nextRequest.voltage;
    document["power"] = nextRequest.power;
    document["energy"] = nextRequest.energy;
    document["cost"] = nextRequest.cost;

    String requestBody;
    serializeJson(doc, requestBody);

    httpClient.begin(client, "https://ap-southeast-1.aws.data.mongodb-api.com/app/data-jdqtkvn/endpoint/data/v1/action/insertOne", 443);
    httpClient.setTimeout(3000);
    httpClient.setHeader("User-Agent: Arduino UNO");
    httpClient.setHeader("Content-Type: application/ejson");
    httpClient.setHeader("Accept: application/json");
    httpClient.setHeader("apiKey: " + String(MONGODB_API_KEY));

    int responseNum = httpClient.POST(requestBody);
    if (responseNum > 0)
    {
      Serial.println("Successfully sent data to MongoDB");
      String body = httpClient.getBody();
      Serial.println(body);
      Serial.println("Response code: " + String(responseNum));
    } else {
      Serial.println("Request Failed: " + String(responseNum));
    }
    httpClient.close();

    requestQueue.pop();
  }
}
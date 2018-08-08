#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "FS.h"

extern "C" {
  #include "user_interface.h"
}

// Struct for holding JSON configuration
struct Configuration {
  String apSSID;
  String apPassword;
  String clientSSID;
  String clientPassword;
  String hostname;
};

String permitted_domain_characters[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-"};

// Html string that holds the page configuration
String page_config_html = "<title>I have been hack</title>"
                    "<font face=\"verdana\">"
                    "<style type=\"text/css\">"
                    "#submit {"
                    "  background-color: #c10000;"
                    "  padding: .5em;"
                    "  -moz-border-radius: 5px;"
                    "  -webkit-border-radius: 5px;"
                    "  border-radius: 6px;"
                    "  color: #black;"
                    "  font-family: 'verdana';"
                    "  font-size: 20px;"
                    "  text-decoration: none;"
                    "  border: none;"
                    "}"

                    "#submit:hover {"
                    "  border: none;"
                    "  background: cyan;"
                    "  box-shadow: 0px 0px 1px #777;"
                    "}"
                    "</style>"
                    "<style>"
                    "body"
                    "{"
                    "  color: #c10000;"
                    "  background-color: #000000;"
                    "}"
                    "</style>";

// Html string that holds the menu
String menu_html =  "<style>"
                    ".topnav {"
                    "  background-color: #2f3136;"
                    "  border-left: solid #c10000 5px;"
                    "  border-radius: 3px;"
                    "  overflow: hidden;"
                    "}"
                    ""
                    ".topnav a {"
                    "    float: left;"
                    "    color: #bfbfbb;"
                    "    text-align: center;"
                    "    padding: 14px 16px;"
                    "    text-decoration: none;"
                    "    font-size: 17px;"
                    "}"
                    ""
                    ""
                    ".topnav a:hover {"
                    "    background-color: #c10000;"
                    "    color: black;"
                    "}"
                    ""
                    ".topnav-right {"
                    "  float: right;"
                    "}"
                    ""
                    "</style>"
                    ""
                    "<div class=\"topnav\">"
                    "  <a href=\"/\">Home</a>"
                    "  <a href=\"settings\">Settings</a>"
                    "  <a href=\"api/settings\">API</a>"
                    "  <div class=\"topnav-right\">"
                    "    <a href=\"about\">About</a>"
                    "  </div>"
                    "</div>";

// Header for html pages
String h1_html = "<style>"
                    "h1 {"
                    "    font-size: 1.7rem;"
                    "    margin-top: 1rem;"
                    "    margin-left: auto;"
                    "    margin-right: auto;"
                    "    background: #2f3136;"
                    "    color: #bfbfbb;"
                    "    padding: 0.2em 1em;"
                    "    border-radius: 3px;"
                    "    border-left: solid #c10000 5px;"
                    "    font-weight: 100;"
                    "}" 
                    "</style>"
                    "";

// Reset html page
String reset_html = h1_html +
                    page_config_html +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"restart\">Restart</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>Module restarting...</span></center>";

// Settings html page for web server
String settings_html = h1_html + 
                    page_config_html +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"settings\">Settings</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>I made a thing do some stuff</span></center>"
                    "<br><br>"
                    "<html><body>"
                    "  <center>"                    
                    "  <form  name='frm' method='get'>"
                    "    <span style=\"font-size: +15px\"/>WiFi Connection Settings</span><br>"
                    "    <input type='text' name='new_target_ssid' placeholder=\"SSID\"><br>"
                    "    <input type='password' name='new_target_password' placeholder=\"Password\"><br>"
                    "    <br><br>"
                    "    <span style=\"font-size: +15px\"/>Access Point Settings</span><br>"
                    "    <input type='text' name='new_ssid' placeholder=\"SSID\"><br>"
                    "    <input type='password' name='new_password' placeholder=\"Password\"><br>"
                    "    <br><br>"
                    "    <input type='text' name='new_hostname' placeholder=\"hostname\">"
                    "    <br><br>"
                    "    <input type='submit' id='submit' value='Submit'>"
                    "  </form>"
                    "  <br><br>"
                    ""
                    "  <form action=\"/restart_esp8266\">"
                    "    <input type=\"submit\" id=\"submit\" value=\"Restart\"/>"
                    "  </form>"
                    "  </center>"
                    "</body></html>";
                    
                    
// About html page
String about_html = h1_html +
                    page_config_html +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"about\">About</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>About</span></center>";


// Main index html page for web server
String index_html = h1_html +
                    page_config_html +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"home\">Home</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>I made a thing do some stuff</span></center>";
                    
                    
ESP8266WebServer server(80);
//WiFiServer server(80);

struct Configuration global_conf;

// Function to convert the hostname to a domain name
// Compare each charater to the list of acceptable characters
// If it is not in the list, it will not be in the domain name
// No Args
// Return domain name string
String ConvertHostname()
{
  String domain_name;
  String comp_char;
  bool character_match;

  // For loop to iterate through the hostname
  for (int i = 0; i < global_conf.hostname.length(); i++)
  {
    character_match = false;

    // For loop to iterate through the list of acceptable characters
    for (int x = 0; x < 37; x++)
    {
      comp_char = (String)global_conf.hostname.charAt(i);
      comp_char.toLowerCase();
      if (comp_char == permitted_domain_characters[x])
      {
        character_match = true;
        break;
      }
      else if ((String)global_conf.hostname.charAt(i) == " ")
      {
        domain_name = domain_name + "-";
        break;
      }
    }
    if (character_match == true)
      domain_name = domain_name + comp_char;
  }

  Serial.print("mDNS Domain Name: ");
  Serial.println(domain_name);
  
  return domain_name;
}





// Function to start an AP if it cant connect to one
// SSID and Password Args
// Return start AP bool
bool startAP(const char* apssid, const char* password)
{
  WiFi.hostname(global_conf.hostname);
  Serial.println("Configuring Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP((const char*)apssid, (const char*)password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  if (!MDNS.begin((const char*)ConvertHostname().c_str()))
  {
    Serial.println("Could not configure mDNS");
    return false;
  }

  server.on("/", HandleClient);
  server.on("/restart_esp8266", RestartESP);
  server.on("/settings", SettingsESP);
  server.on("/about", AboutESP);
  server.on("/api/settings", APIESP);
  
  server.begin();

  MDNS.addService("http", "tcp", 80);

  return true;
}





// Function to join wifi after config load
// SSID and Password Args
// Return join wifi bool
bool joinWiFi(const char* ssid, const char* password)
{
  WiFi.hostname(global_conf.hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin((const char*)ssid, (const char*)password);

  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    if (i >= 10)
      return false;
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin((const char*)ConvertHostname().c_str()))
  {
    Serial.println("Could not configure mDNS");
    return false;
  }

  server.on("/", HandleClient);
  server.on("/restart_esp8266", RestartESP);
  server.on("/settings", SettingsESP);
  server.on("/about", AboutESP);
  server.on("/api/settings", APIESP);
  
  server.begin();

  MDNS.addService("http", "tcp", 80);
  
  return true;
}





// Function to load the JSON file from SPIFFS into a config struct
// Returns config struct
struct Configuration loadConfig() {
  struct Configuration conf;
  
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return conf;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return conf;
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return conf;
  }

  // Load JSON into local variables
  const char* apSSID = json["apSSID"];
  const char* apPassword = json["apPassword"];
  const char* clientSSID = json["clientSSID"];
  const char* clientPassword = json["clientPassword"];
  const char* hostname = json["hostname"];

  // Load local variables into struct
  conf.apSSID = apSSID;
  conf.apPassword = apPassword;
  conf.clientSSID = clientSSID;
  conf.clientPassword = clientPassword;
  conf.hostname = hostname;

  // Output struct
  Serial.print("Loaded apSSID: ");
  Serial.println(conf.apSSID);
  Serial.print("Loaded apPassword: ");
  Serial.println(conf.apPassword);
  Serial.print("Loaded clientSSID: ");
  Serial.println(conf.clientSSID);
  Serial.print("Loaded clientPassword: ");
  Serial.println(conf.clientPassword);
  Serial.print("Loaded hostname: ");
  Serial.println(conf.hostname);
  
  return conf;
}





// Function to save the configuration settings to a JSON file in SPIFFS
// No Args
// Return save bool
bool saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["apSSID"] = global_conf.apSSID;
  json["apPassword"] = global_conf.apPassword;
  json["clientSSID"] = global_conf.clientSSID;
  json["clientPassword"] = global_conf.clientPassword;
  json["hostname"] = global_conf.hostname;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);

  Serial.println("Saved new configuration");
  
  return true;
}





// Function to handle client input when at /settings
// No Args
// No Return
void SettingsESP()
{
  server.send(200, "text/html", settings_html);
  if (server.args() > 0)
  {
    Serial.println("Server arguments received");
    for (uint8_t i = 0; i < server.args(); i++)
    {
      Serial.println(server.argName(i));

      // BRANCH STATEMENTS TO CHANGE CONFIGURATION
      if (server.argName(i) == "new_target_ssid" && server.arg(i).length() > 0)
      {
        Serial.print("New Access Point to connect to: ");
        Serial.println(server.arg(i));
        global_conf.clientSSID = server.arg(i);
      }
      else if (server.argName(i) == "new_target_password" && server.arg(i).length() > 0)
      {
        Serial.print("New Access Point Password to connect with: ");
        Serial.println(server.arg(i));
        global_conf.clientPassword = server.arg(i);
      }
      else if (server.argName(i) == "new_ssid" && server.arg(i).length() > 0)
      {
        Serial.print("New Access Point to create: ");
        Serial.println(server.arg(i));
        global_conf.apSSID = server.arg(i);
      }
      else if (server.argName(i) == "new_password" && server.arg(i).length() > 0)
      {
        Serial.print("New Access Point password to create with: ");
        Serial.println(server.arg(i));
        global_conf.apPassword = server.arg(i);
      }
      else if (server.argName(i) == "new_hostname" && server.arg(i).length() > 0)
      {
        Serial.print("New Hostname: ");
        Serial.println(server.arg(i));
        global_conf.hostname = server.arg(i);
      }
    }
    saveConfig();
  }
}





// Function to handle client at /api/settings
// Displays settings in json format
// No Args
// No Return
void APIESP()
{
  String json_string = "{"
                "    \"apSSID\": \"" + global_conf.apSSID + "\","
                "    \"apPassword\": \"" + global_conf.apPassword + "\","
                "    \"clientSSID\": \"" + global_conf.clientSSID + "\","
                "    \"clientPassword\": \"" + global_conf.clientPassword + "\","
                "    \"hostname\": \"" + global_conf.hostname + "\""
                "}";
  server.send(200, "application/json", json_string);
}





// Function to handle client at /reset_esp8266
// No Args
// No Return
void RestartESP()
{
  server.send(200, "text/html", reset_html);
  ESP.reset();
}





// Function to handle client at /
// No Args
// No Return
void HandleClient()
{
  server.send(200, "text/html", index_html);
}





// Function to handle client at /about
// No Args
// No Return
void AboutESP()
{
  server.send(200, "text/html", about_html);
}






void setup() {
  //struct Configuration conf;

  Serial.begin(115200);
  Serial.println("------------------------------------");
  delay(1000);
  Serial.println("Mounting FS...");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  global_conf = loadConfig();

  // Load config and try to join WiFi or create AP
  if (global_conf.clientSSID.length() == 0 || global_conf.clientPassword.length() == 0)
  {
    Serial.println("Could not load configuration");
  }
  else
  {
    Serial.println("Config Loaded");
    if (!joinWiFi((const char*)global_conf.clientSSID.c_str(), (const char*)global_conf.clientPassword.c_str()))
    {
      Serial.print("\nUnable to join ");
      Serial.println(global_conf.clientSSID);
      if (startAP((const char*)global_conf.apSSID.c_str(), (const char*)global_conf.apPassword.c_str()))
      {
        Serial.print("Access Point configured: ");
        Serial.println(global_conf.apSSID);
      }
    }
  }

  Serial.print("http://");
  Serial.print(WiFi.localIP());
}





void loop() {
  server.handleClient();
}

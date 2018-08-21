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
  String www_username;
  String www_password;
};

ESP8266WebServer server(80); // Initialize web server

// Setup global vars
int esp12led = 2;
int nodemculed = 16;
struct Configuration global_conf;
String domain_name_prefix = "rota";
String permitted_domain_characters[] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-"};



// CSS Colors
String color1 = "#c10000"; // Accent
String color2 = "#000000"; // Background
String color3 = "#2f3136"; // Bar
String color4 = "#bfbfbb"; // Text
String color5 = "#ffe500"; // Important Bar



// Html string that holds the page configuration
String page_style_css = "<title>I have been hack</title>"
                    "<font face=\"verdana\">"
                    "<style type=\"text/css\">"
                    "    #submit {"
                    "        background-color: " + color1 + ";"
                    "        padding: .5em;"
                    "        -moz-border-radius: 5px;"
                    "        -webkit-border-radius: 5px;"
                    "        border-radius: 6px;"
                    "        color: " + color2 + ";"
                    "        font-family: 'verdana';"
                    "        font-size: 20px;"
                    "        text-decoration: none;"
                    "        border: none;"
                    "    }"
                    "    #submit:hover {"
                    "        border: none;"
                    "        background: cyan;"
                    "        box-shadow: 0px 0px 1px #777;"
                    "  color: black;"
                    "    }"
                    "    body"
                    "    {"
                    "        color: " + color1 + ";"
                    "        background-color: " + color2 + ";"
                    "    }"
                    "    .topnav {"
                    "        background-color: " + color3 + ";"
                    "        border-left: solid " + color1 + " 5px;"
                    "        border-radius: 3px;"
                    "        overflow: hidden;"
                    "    }"
                    "    .topnav a {"
                    "        float: left;"
                    "        color: " + color4 + ";"
                    "        text-align: center;"
                    "        padding: 4px 10px;"
                    "        text-decoration: none;"
                    "        font-size: 17px;"
                    "        border-radius: 3px;"
                    "        margin-top: 0.5rem;"
                    "        margin-left: 0.5rem;"
                    "        margin-right: 0.5rem;"
                    "        margin-bottom: 0.5rem;"
                    "    }"
                    "    .topnav a:hover {"
                    "        background-color: " + color1 + ";"
                    "        color: black;"
                    "    }"
                    "    .topnav-right {"
                    "        float: right;"
                    "    }"
                    "    h1 {"
                    "        font-size: 1.7rem;"
                    "        margin-top: 1rem;"
                    "        margin-left: auto;"
                    "        margin-right: auto;"
                    "        background: " + color3 + ";"
                    "        color: " + color4 + ";"
                    "        padding: 0.2em 1em;"
                    "        border-radius: 3px;"
                    "        border-left: solid " + color1 + " 5px;"
                    "        font-weight: 100;"
                    "    }"
                    "    h2 {"
                    "        font-size: 1rem;"
                    "        margin-top: 1rem;"
                    "        margin-left: auto;"
                    "        margin-right: auto;"
                    "        background: " + color3 + ";"
                    "        color: " + color4 + ";"
                    "        padding: 0.2em 1em;"
                    "        border-radius: 3px;"
                    "        border-left: solid " + color1 + " 5px;"
                    "        font-weight: 100;"
                    "    }"
                    "    h3 {"
                    "        font-size: 1rem;"
                    "        margin-top: 1rem;"
                    "        margin-left: auto;"
                    "        margin-right: auto;"
                    "        background: " + color5 + ";"
                    "        color: " + color2 + ";"
                    "        padding: 0.2em 1em;"
                    "        border-radius: 3px;"
                    "        font-weight: 100;"
                    "    }"
                    "    .column {"
                    "        margin-left: 10rem;"
                    "        margin-right: 10rem;"
                    "    }"
                    "    .column input {"
                    "        float: right;"
                    "        margin-top: 0.5rem;"
                    "        background: transparent;"
                    "        color: " + color4 + ";"
                    "        outline: 0;"
                    "        border: 0;"
                    "        border-bottom: solid " + color1 + " 2px;"
                    "        font-size: 14px;"
                    "    }"
                    "    .column input {"
                    "        clear: both;"
                    "    }"
                    "    .column span {"
                    "        margin-top: 0.5rem;"
                    "        display: inline-block;"
                    "    }"
                    "    .column about {"
                    "      font-size: +12px;"
                    "    }"
                    ""    
                    "   .column spanr {"
                    "        margin-top: 0.5rem;"
                    "    }"
                    ""    
                    "    .prefix {"
                    "      display: flex;"
                    "        justify-content: space-between;"
                    "    }"
                    ""    
                    "    hr {"
                    "      border-color: " + color3 + ";"
                    "        background-color: " + color3 + ";"
                    "        height: 2px;"
                    "        border: none;"
                    "    }"
                    "</style>";

// Html string that holds the menu
String menu_html =  "<div class=\"topnav\">"
                    "  <a href=\"/\">Home</a>"
                    "  <a href=\"settings\">Settings</a>"
                    "  <a href=\"api/settings\">API</a>"
                    "  <div class=\"topnav-right\">"
                    "    <a href=\"about\">About</a>"
                    "  </div>"
                    "</div>";

// Reset html page
String reset_html = page_style_css +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"restart\">Restart</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>Module restarting...</span></center>";

                    
                    
// About html page
String about_html = page_style_css +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"about\">About</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>About</span></center>";


// Main index html page for web server
String index_html = page_style_css +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"home\">Home</h1>"
                    "<br>"
                    "<center><span style=\"font-size: +100px\"/>ESP8266 Access Point</span></center>"
                    "<br>"
                    "<center><span style=\"font-size: +50px\"/>I made a thing do some stuff</span></center>";
                    
                    






// Settings html page for web server
// Function to generate settings page based on current configuration
// No args
// Return 1 String
String GenSettingsHTML()
{
  return page_style_css +
                    menu_html +
                    "<h1 class=\"header\" data-translate=\"settings\">Settings</h1>"
                    "<html><body>"
                    "    <div class=\"column\">"
                    "    <form action=\"/restart_esp8266\">"
                    "        <input type=\"submit\" id=\"submit\" value=\"Restart\"/>"
                    "    </form>"
                    "    <br><br><br>"
                    "    <form name='frm' method='get'>"
                    ""
                    "            <h2 class=\"header\" data-translate=\"settings\">Admin Settings</h2>"
                    "            <about/>Use these settings when logging in to this web page.</about>"
                    "            <hr>"
                    "            <br>"
                    "            <span style=\"font-size: +14px\"/>Admin Username</span>"
                    "            <input type='text' name='www_username' value='" + global_conf.www_username + "' placeholder=\"Username\"><br>"
                    "            <span style=\"font-size: +14px\"/>Admin Password</span>"
                    "            <input type='password' name='www_password' value='" + global_conf.www_password + "' placeholder=\"Password\"><br>"
                    "            <br><br>"
                    ""
                    "            <h2 class=\"header\" data-translate=\"settings\">WiFi Connection Settings</h2>"
                    "            <about/>Use these settings to connect this device to a WiFi Network in your area.</about>"
                    "            <hr>"
                    "            <br>"
                    "            <span style=\"font-size: +14px\"/>WiFi Connection SSID</span>"
                    "            <input type='text' name='new_target_ssid' value='" + global_conf.clientSSID + "' placeholder=\"SSID\"><br>"
                    "            <span style=\"font-size: +14px\"/>WiFi Connection Password</span>"
                    "            <input type='password' name='new_target_password' value='" + global_conf.clientPassword + "' placeholder=\"Password\"><br>"
                    "            <br><br>"
                    ""
                    "            <h2 class=\"header\" data-translate=\"settings\">Access Point Settings</h2>"
                    "            <about/>Use these settings to configure this device's access point. These settings will be used by other wireless clients when they connect to this device.</about>"
                    "            <hr>"
                    "            <br>"
                    "            <span style=\"font-size: +14px\"/>Access Point SSID</span>"
                    "            <input type='text' name='new_ssid' value='" + global_conf.apSSID + "' placeholder=\"SSID\"><br>"
                    "            <span style=\"font-size: +14px\"/>Access Point Password</span>"
                    "            <input type='password' name='new_password' value='" + global_conf.apPassword + "' placeholder=\"Password\"><br>"
                    "            <br><br>"
                    ""
                    "            <h2 class=\"header\" data-translate=\"settings\">mDNS Settings</h2>"
                    "            <about/>The mDNS settings are used to ensure other clients on the same network can access this device using a domain name rather than an IP address.</about>"
                    "            <br>"
                    "            <about/><b>Note:</b> The domain name will have a default prefix of \"" + domain_name_prefix + "\".</about>"
                    "            <hr>"
                    "            <br>"
                    "            <div class='prefix'>"
                    "                <span style='font-size: +14px'/>mDNS Domain Name</span>"
                    "                <div style='text-align: right'>"
                    "                    <span style='font-size: +14px'/>" + domain_name_prefix + "-</span>"
                    "                    <input type='text' name='new_hostname' value='" + global_conf.hostname + "' placeholder='Hostname'>"
                    "                </div>"
                    "            </div>"
                    "            <h3 class='header' data-translate='settings'><spon style='font-size: +14px'>URL</spon><spon style='float: right;font-size: +14px'>http://" + ConvertHostname() + ".local</spon></h3>"
                    "            <br><br>"
                    "            <input type='submit' id='submit' value='Apply'>"
                    "      </form>"
                    ""
                    "    </div>"
                    "</body></html>";
}






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

  domain_name = domain_name_prefix + "-" + domain_name;

  Serial.print("mDNS Domain Name: ");
  Serial.println(domain_name);
  
  return domain_name;
}




// Function to tell server how to behave for each address
// No args
// No Return
void SetServerBehavior()
{
  if (!MDNS.begin((const char*)ConvertHostname().c_str()))
  {
    Serial.println("Could not configure mDNS");
    return;
  }
  
  //server.on("/", HandleClient);
  //server.on("/restart_esp8266", RestartESP);
  //server.on("/settings", SettingsESP);
  //server.on("/about", AboutESP);
  //server.on("/api/settings", APIESP);
  //server.on("/api/set", ProcessJSONPost);

  // Check for authentication on all pages
  server.on("/", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    HandleClient();
  });
  server.on("/restart_esp8266", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    RestartESP();
  });
  server.on("/settings", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    SettingsESP();
  });
  server.on("/about", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    AboutESP();
  });
  server.on("/api/settings", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    APIESP();
  });
  server.on("/api/set", [](){
    if(!server.authenticate((const char*)global_conf.www_username.c_str(), (const char*)global_conf.www_password.c_str()))
      return server.requestAuthentication();
    ProcessJSONPost();
  });
  
  server.begin();

  MDNS.addService("http", "tcp", 80);
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

  SetServerBehavior();

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
  while (WiFi.status() != WL_CONNECTED) { // Wait for the WiFi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    if (i >= 10)
      return false;
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  SetServerBehavior();
  
  return true;
}





// Function to load the JSON file from SPIFFS into a config struct
// No args
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

  StaticJsonBuffer<300> jsonBuffer;
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
  const char* www_username = json["www_username"];
  const char* www_password = json["www_password"];

  // Load local variables into struct
  conf.apSSID = apSSID;
  conf.apPassword = apPassword;
  conf.clientSSID = clientSSID;
  conf.clientPassword = clientPassword;
  conf.hostname = hostname;
  conf.www_username = www_username;
  conf.www_password = www_password;

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
  Serial.print("Loaded username: ");
  Serial.println(conf.www_username);
  Serial.print("Loaded password: ");
  Serial.println(conf.www_password);
  
  return conf;
}





// Function to save the configuration settings to a JSON file in SPIFFS
// No Args
// Return save bool
bool saveConfig() {
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["apSSID"] = global_conf.apSSID;
  json["apPassword"] = global_conf.apPassword;
  json["clientSSID"] = global_conf.clientSSID;
  json["clientPassword"] = global_conf.clientPassword;
  json["hostname"] = global_conf.hostname;
  json["www_username"] = global_conf.www_username;
  json["www_password"] = global_conf.www_password;

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
      else if (server.argName(i) == "www_username" && server.arg(i).length() > 0)
      {
        Serial.print("New Username: ");
        Serial.println(server.arg(i));
        global_conf.www_username = server.arg(i);
      }
      else if (server.argName(i) == "www_password" && server.arg(i).length() > 0)
      {
        Serial.print("New Password: ");
        Serial.println(server.arg(i));
        global_conf.www_password = server.arg(i);
      }
    }
    saveConfig();
  }
  // Show response AFTER setting new config
  server.send(200, "text/html", GenSettingsHTML());
}





// Function to convert shit to *
// Takes 1 String arg
// Returns 1 String
String HideString(String plain)
{
  String hidden = "";
  
  for (int i = 0; i < plain.length(); i++)
    hidden = hidden + "*";

  return hidden;
}




// Function to handle client at /api/settings
// Displays settings in json format
// No Args
// No Return
void APIESP()
{ 
  String json_string = "{"
                "    \"apSSID\": \"" + global_conf.apSSID + "\","
                "    \"clientSSID\": \"" + global_conf.clientSSID + "\","
                "    \"hostname\": \"" + global_conf.hostname + "\","
                "    \"www_username\": \"" + global_conf.www_username + "\""
                "}";
  server.send(200, "application/json", json_string);
}



// Function to handle JSON API post data
// No Args
// No Return
void ProcessJSONPost()
{
  server.send(200, "application/json", "{\"success\": true}");
  if (server.args() > 0)
  {
    Serial.println("Server arguments received");
    for (uint8_t i = 0; i < server.args(); i++)
    {
      Serial.println(server.argName(i));

      // When JSON API is used
      if (server.argName(i) == "json_post" && server.arg(i).length() > 0)
      {
        Serial.print("JSON API Post: ");
        Serial.println(server.arg(i));

        StaticJsonBuffer<300> jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(server.arg(i));
      
        // Load JSON into local variables
        const char* apSSID = json["apSSID"];
        const char* apPassword = json["apPassword"];
        const char* clientSSID = json["clientSSID"];
        const char* clientPassword = json["clientPassword"];
        const char* hostname = json["hostname"];
        const char* www_username = json["www_username"];
        const char* www_password = json["www_password"];
      
        // Load local variables into struct and output
        if (json.containsKey("apSSID"))
        {
          global_conf.apSSID = apSSID;
          Serial.print("Loaded apSSID: ");
          Serial.println(global_conf.apSSID);
        }
        if (json.containsKey("apPassword"))
        {
          global_conf.apPassword = apPassword;
          Serial.print("Loaded apPassword: ");
          Serial.println(global_conf.apPassword);
        }
        if (json.containsKey("clientSSID"))
        {
          global_conf.clientSSID = clientSSID;
          Serial.print("Loaded clientSSID: ");
          Serial.println(global_conf.clientSSID);
        }
        if (json.containsKey("clientPassword"))
        {
          global_conf.clientPassword = clientPassword;
          Serial.print("Loaded clientPassword: ");
          Serial.println(global_conf.clientPassword);
        }
        if (json.containsKey("hostname"))
        {
          global_conf.hostname = hostname;
          Serial.print("Loaded hostname: ");
          Serial.println(global_conf.hostname);
        }
        if (json.containsKey("www_username"))
        {
          global_conf.www_username = www_username;
          Serial.print("Loaded www_username: ");
          Serial.println(global_conf.www_username);
        }
        if (json.containsKey("www_password"))
        {
          global_conf.www_password = www_password;
          Serial.print("Loaded www_password: ");
          Serial.println(global_conf.www_password);
        }
      
      
        // Save the new config
        saveConfig();
      }
    }
  }
}





// Function to handle client at /reset_esp8266
// No Args
// No Return
void RestartESP()
{
  server.send(200, "text/html", reset_html);
  delay(5000);
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
  pinMode(esp12led, OUTPUT);
  pinMode(nodemculed, OUTPUT);

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
  Serial.println(WiFi.localIP());
}





void loop() {
  server.handleClient();

  // Let the user know we are in main loop
  digitalWrite(esp12led, LOW);
  delay(500);
  digitalWrite(esp12led, HIGH);
  delay(500);  
}

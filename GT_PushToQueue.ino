#include <SPI.h>
#include <Ethernet.h>

const int NO_GAME = 0;
const int SETUP = 1;
const int GAME_IN_PLAY = 2;
const int POSSIBLE_GAME_OVER = 3;

int gameState = NO_GAME;
boolean gameInPlay = false;
int gameInPlay_LED = 8;
int optionsInput = 2;
int helpInput = 3;
int optionsVal = 0;
int helpVal = 0;
unsigned long timeOff = 0;
unsigned long now = 0;

//10-24-80-59-51-F8
byte mac[] = { 0x10, 0x24, 0x80, 0x59, 0x51, 0xF8 };
char server[] = "sqs.us-west-2.amazonaws.com";
IPAddress ip(192,168,0,177);
boolean pushingToQueue = false;

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(gameInPlay_LED, OUTPUT);
  pinMode(optionsInput, INPUT);
  pinMode(helpInput, INPUT);

  Serial.begin(9600);

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
}

// the loop routine runs over and over again forever:
void loop() {
  optionsVal = digitalRead(optionsInput);
  helpVal = digitalRead(helpInput);
  now = millis();
  
  if (gameState == NO_GAME && optionsVal == LOW && helpVal == LOW) {
      gameState = GAME_IN_PLAY;
      gameInPlay = true;
      Serial.print("Game In Play\n");
      delay(100);
      pushToQueue("Game In Play");
  }
  
  
  if (gameState == GAME_IN_PLAY && optionsVal == HIGH && helpVal == HIGH) {
    gameState = POSSIBLE_GAME_OVER;
    timeOff = millis();
    Serial.print("Possible Game Over\n");
  }
  
  
  if (gameState == POSSIBLE_GAME_OVER && (optionsVal == LOW || helpVal == LOW)) {
    gameState = GAME_IN_PLAY;
    gameInPlay = true;
    timeOff = 0;
  }
  
  
  if (gameState == POSSIBLE_GAME_OVER && now - timeOff > 5000) {
    gameState = NO_GAME;
    gameInPlay = false;
    Serial.print("Game Over\n");
    pushToQueue("Game Over"); 
  }
  
  if (gameInPlay) {
    digitalWrite(gameInPlay_LED, HIGH);
  } else {
    digitalWrite(gameInPlay_LED, LOW);
  }
  
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  
  // if the server's disconnected, stop the client:
  if (pushingToQueue && !client.connected()) {
    
    Serial.println();
    Serial.println("disconnecting");
    client.stop();
    pushingToQueue = false;    
  }
  
  
}

void pushToQueue(char* msg) {  
  String urlEncMsg = URLEncode(msg);
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("GET /040291695885/twgoldentee?Action=SendMessage&MessageBody=" + urlEncMsg + " HTTP/1.1");
    client.println("Host: sqs.us-west-2.amazonaws.com");
    client.println("Connection: close");
    client.println();
    pushingToQueue = true;
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  } 
}

String URLEncode(const char* msg)
{
    const char *hex = "0123456789abcdef";
    String encodedMsg = "";

    while (*msg!='\0'){
        if( ('a' <= *msg && *msg <= 'z')
                || ('A' <= *msg && *msg <= 'Z')
                || ('0' <= *msg && *msg <= '9') ) {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 15];
        }
        msg++;
    }
    return encodedMsg;
}


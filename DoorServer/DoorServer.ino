

/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */

#include <sha256.h>
#include <SPI.h>
#include <Ethernet.h>
#include "password.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
unsigned long currentNumber = 0;
String password = PASSWORD;

int seedPin = 4;
int doorPin = 3;
unsigned long maxRandom = 4294967295UL;
int bufferLength = 100;
boolean doorbell = false;

byte mac[] = {
  0x90, 0xA2, 0xDA, 0x0F, 0x04, 0x0C
};
IPAddress ip(10, 13, 37, 40);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // Open serial communications and wait for port to open:
  pinMode(doorPin, OUTPUT);
  digitalWrite(doorPin, HIGH);
//  Sha1.init();
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  randomSeed(analogRead(seedPin));

}


void loop() {

  if (doorbell)
  {
    
  }
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String lineBuffer = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        if (lineBuffer.length() < bufferLength)
        {
          lineBuffer += c;
        }
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          
          Serial.println();
          Serial.println();
          Serial.println("This is now my stuff");
          Serial.println();
          
          client.println("HTTP/1.1 200 OK");

//          if (lineBuffer[0] == 'O')
//          {
//            Serial.println("Ooh, an options header");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Access-Control-Allow-Headers: origin, x-csrftoken, content-type, accept, Authorization");
            client.println("Access-Control-Allow-Methods: POST, OPTIONS");
            client.println("Content-Type: text/html");
//          }
          if (lineBuffer[0] == 'O')
          {
            client.println("Connection: keep-alive");  // the connection will be closed after completion of the response
            client.println("Content-Length: 0");
          }
          if (lineBuffer[0] == 'G')
          {
            client.println("Connection: close");  // the connection will be closed after completion of the response
            
//            char currentCharacter = 'a';
//            char previousCharacter = 'a';
//            while(previousCharacter != '\n' && currentCharacter != '\n')
//            {
//              previousCharacter = currentCharacter;
//              currentCharacter = (client.read());
//              Serial.write(currentCharacter);
//            }
            client.println("Content-Type: application/json;charset=utf-8");
            client.println();
            String body;
            if (lineBuffer.indexOf("key=-1&epoc=") != -1)
            {
              Serial.println("\nRequested a new number");
              currentNumber = random() * maxRandom;
              Serial.println("{\"key\": " + String(currentNumber) + "}");
              body = "{\"key\": " + String(currentNumber) + "}";
            }
            else
            {
              
              Serial.print("Key supplied: ");
              String userAnswer = lineBuffer.substring(lineBuffer.indexOf("/?key=") + String("/?key=").length(), lineBuffer.indexOf("&epoc="));
              String time = lineBuffer.substring(lineBuffer.indexOf("&epoc=") + String("&epoc=").length(), lineBuffer.indexOf(" HTTP"));
              Serial.println(userAnswer);
              Sha256.init();
              Sha256.print(currentNumber + password + time);
              uint8_t* desired = Sha256.result();
              printHash(desired);
              Serial.println(makeHash(desired));
              String correctAnswer = makeHash(desired);
              Serial.print("Linebuffer was: ");
              Serial.println(lineBuffer);
              delay(100);
              if(correctAnswer == userAnswer)
              {
              Serial.println("Door is open!");
              openDoor();
              body = "{\"open\": true}";
              }
              else
              {
                Serial.println("Wrong answer!");
                body = "{\"open\": false}";
              }
            }
            body += "\n\n";
            Serial.println("Body is: " + body);
            Serial.print("Content-Length: ");
            Serial.println(body.length());
//            client.print("Content-Length: ");
//            client.println(body.length());
            client.println();
            client.println(body);
//            Sha1.print(number);
//            if (number == Sha1.result())
//            {
//              Serial.println("Expected: " + number);
//              Serial.println("Got: " + Sha1.result());      
//            }
            client.stop();
            
          }
//          break;
          client.println();
          client.println();
          lineBuffer = "";
          
        }
        if (c == '\n') {
          // you're starting a new line
            currentLineIsBlank = true;          
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");

  }
}

void openDoor()
{
  digitalWrite(doorPin, LOW);
  delay(1000);
  digitalWrite(doorPin, HIGH);
}

void printHash(uint8_t* hash) {
  int i;
  for (i=0; i<32; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

String makeHash(uint8_t* hash) {
  int i;
  String returnValue = "";
  for (i=0; i<32; i++) {
    returnValue += "0123456789abcdef"[hash[i]>>4];
    returnValue += ("0123456789abcdef"[hash[i]&0xf]);
  }
  return returnValue;
}

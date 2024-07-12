#include "ESPWEB.h"

ESPWEB::ESPWEB()
{
    
}

void ESPWEB::port()
{
    server = new WiFiServer (80);
}

void ESPWEB::Sbegin()
{
    server->begin();
}

bool ESPWEB::ClientConnected(int countQRValido, int counQRIni, int countQRInv, int countErrWifi, int counQRSoli, int counWifiC, int counErrServ, int counErrServT, int countQRValidoT, String ipFija,String DateTime)
{
  String datetime;
    WiFiClient  client = server->available();
    if (client)
    {
     Serial.println("New Client.");          // 
     String currentLine = "";
     while(client.connected())
     {
      if (client.available()) 
      {             // si hay bytes para leer desde el cliente
      
        char c = client.read();             // lee un byte
        Serial.write(c);                    // imprime ese byte en el monitor serial
        header += c;
        if (c == '\n') 
        {                    // si el byte es un caracter de salto de linea
          // si la nueva linea está en blanco significa que es el fin del 
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) 
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Botton reset
            // if (header.indexOf("GET /reset")>= 0)
            // {
            //   headers = 0;
            //   Serial.println("Reseteo");
            //   Values.reset();
            // }
    

            // Muestra la página web
              if (header.indexOf("GET /?value=reset")>= 0)
              {
                Serial.println("Reseteo");
                datetime = DateTime;
                Serial.println(datetime);
                Values.putDateTime(datetime);
                Values.reset();
                //client.connect(iparray,80);
              }
            
            String dt = Values.getDateTime();
            Serial.println(dt);
            //Values.getCountVariables();
            client.println(paginaInicio + " DESDE: " +  dt + " HASTA: ");
            client.println(paginaInicio2 + " Inicios de ESP32: " + pagina2 + String(counQRIni));
            client.println(pagina1 + " Lecturas de QR: " + pagina2 + String(counQRSoli));
            client.println(pagina1 + " QR validos: " + pagina2 + String(countQRValido));
            client.println(pagina1 + " QR invalidos: " + pagina2 + String(countQRInv));
            client.println( pagina1 + " Fallas del servidor: " + pagina2 + String(counErrServ));
            client.println(pagina1 + " Conexiones Wifi: " + pagina2 + String(counWifiC));
            client.println(pagina1 +" Desconexines Wifi: " + pagina2 + String(countErrWifi) + paginaintermedio);
            client.println( " QR validos: " + pagina2 + String(countQRValidoT) );
            client.println( pagina1 + " Fallas del servidor: " + pagina2 + String(counErrServT) + paginaintermediofin);
            client.println(" IP " + String(ipFija) + paginaFin);
            client.println(); // la respuesta HTTP temina con una linea en blanco
            break;
            }
          
          else 
          { // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
     }
     // Limpiamos la variable header
     header = "";
     // Cerramos la conexión
     client.stop();
     Serial.println("Client disconnected.");
     Serial.println("");    
    return true;
    }
    else 
    {
        return false;
    }
}


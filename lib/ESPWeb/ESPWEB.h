#ifndef ESP_WEB
#define ESP_WEB

#include <WiFi.h>
#include <StatisticsValues.h>
#include "WebServiceComunication.h"
using namespace std;

class ESPWEB
{
    public:
        explicit ESPWEB();

        bool ClientConnected(int countQRValido, int counQRIni, int countQRInv, int countErrWifi, int counQRSoli, int counWifiC, int counErrServ, int counErrServT, int countQRValidoT, String ipFija, String DateTime);
        
        StatisticsValues Values;

        WebServiceComunication myServerComun;
        
        void port();

        void Sbegin();

        WiFiServer *server;

        String paginaInicio = "<!DOCTYPE html>"
        "<html lang='es-US'>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta http-equiv='X-UA-Compatible' content='IE=edge'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "</head>"
        "<style type='text/css' media='screen'>"
        ".reloj{"
        "margin-top: 25px;"
        "font-size: 25px;"
        "font-weight: 900;"
        "display: flex; flex-direction: row;"
        "}"
        "html{"
        "background-color: rgb(221, 221, 221);"
        "}"
        ".container{"
        "display: flex;"
        "justify-content: center;"
        "align-items: center;"
        "flex-direction: column;"
        "text-align: center;"
        "}"
        ".tabla{"
        "background-color: rgb(0, 0, 0);"
        "margin-bottom: 20px;"
        "}"
        "td{"
        "text-align: left;"
        "background-color: rgb(255, 255, 255);"
        "padding: 10px;"
        "font-weight: 900;"
        "font-size: 20px;"
        "}"
        ".res{"
        "height: 20px;"
        "width: 150px;"
        "text-align: right;"
        "}"
        ".c_tabla{"
        "border:solid 2px rgb(0, 0, 0);"
        "background-color: rgb(59, 201, 95);"
        "padding: 5px;"
        "}"
        ".boton{"
        "color: black;"
        "text-decoration: none;"
        "margin-top: 20px;"
        "border: solid 2px black;"
        "background-color: aliceblue;"
        "height: 20px;"
        "width: 150px;"
        "text-decoration: none;"
        "cursor:pointer;"
        "}"
        ".boton:hover{"
        "padding: 0px 5px 0px 5px;"
        "background-color: rgb(0, 0, 0);"
        "color: aliceblue;"
        "}"
        "@media (max-width:1000px) {"
        "td{"
        "padding: 5px;"
        "font-weight: 800;"
        "font-size: 15px;"
        "}"
        "}"
        "@media (max-width:800px) {"
        "td{"
        "padding: 2px;"
        "font-weight: 800;"
        "font-size: 10px;"
        "}"
        "}"
        "</style>"
        "<body>"
        "<div class='container'>"
        "<h1 class='titulo'>Registro de Eventos</h1>"
        "<div style='display: flex; flex-direction: row;'><p class='reloj'>";

        String paginaInicio2 = "</p><p class='reloj' id='hora_fin'></p></div>" 
        "<div class='c_tabla'>"
        "<table class='tabla'>"
        "<tr>"
        "<td>";

        String pagina1 = "</td><tr><td>";
        String pagina2 = "</td> <td class='res'>";

        String paginaintermedio = "</td>"
        "</tr>"
        "</table>"
        "<h1 class='titulo'>Historico</h1>"
        "<table class='tabla'>"
        "<tr>"
        "<tr>"
        "<td>";

        String paginaintermediofin = "</td></tr>"
        "</tr>"
        "</table>"
        "<a id='boton'class='boton'>RESET</a>"
        "</div>"
        "<div class= 'reloj'><p id='reloj'>00 : 00 : 00 &nbsp 00/00/0000</p></div>"
        "<div class='reloj'>";

        String paginaFin = "</div>"
        "</div>"
        "<script>"
        "function actual() {"
        "fecha=new Date();"
        "dia=fecha.getDate();"
        "mes=fecha.getMonth()+1;"
        "año=fecha.getFullYear();"
        "hora=fecha.getHours();"
        "minuto=fecha.getMinutes();"
        "segundo=fecha.getSeconds();"
        "if (hora<10) {"
        "hora='0'+hora;"
        "}"
        "if (minuto<10) {"
        "minuto='0'+minuto;"
        "}"
        "if (segundo<10) {"
        "segundo='0'+segundo;"
        "}"
        "mireloj = hora+':'+minuto+':'+segundo;"	
        "mifecha = mireloj+' '+dia+'/'+mes+'/'+año;"
        "return mifecha;" 
        "}"
        "function actualizar() {"
        "mihora=actual();"
        "mireloj=document.getElementById('reloj');"
        "mireloj.innerHTML=mihora;"
        "}"
        "setInterval(actualizar,1000);"
        "function hora(){"
        "mihora=actual();"
        "horafin=document.getElementById('hora_fin');" 
        "horafin.innerHTML=mihora;"
        "}"
        "hora();"
        "var boton = document.getElementById('boton');"
        "boton.onclick=()=>{"
        "if(confirm('¿Deseas Resetera las estadisticas?')==true)"
        "{"
        "window.location.href='?value=reset';"
        "}"
        "else{"
        "window.location.href='';"
        "}};"
        "function v_r(){"
        "var url_r = window.location.href;"
        "if(url_r.includes('reset')!=false){"
        "setTimeout(()=>{"
        "window.location.href='?value=';"
        "},500)"
        "}"
        "}"
        "v_r();"
        "</script>"
        "</body>"
        "</html>";

        String header;

        int headers;

    private:
};
#endif
/******************************************************************************************
Control de Acceso RFID con lector ID-20
Camilo Nuñez Fernandez
Correo: camilo.nunez@cnf.cl
CNF Chile
30 de Diciembre del 2013
Santiago de Chile

Esto funciona, gracias de un servidor de base de datos MySQL el cual instalaremos 
en nuestro YUN, esta base de datos almacenara dos tablas, una para usuarios asociados 
a un RFID determinado y otra tabla para mantener registros de todas las tarjetas que 
han pasado por nuestro lector. A la vez utilizaremos los lenguajes de programación PHP
y Python para realizar algunos scripts y trabajar con nuestra base datos.

Además de ello, en el aérea del hardware, utilizaremos un lector ID-20 junto a un LCD 
Serial de 16 caracteres, mas un led RGB el cual será nuestro informador.

Control de Acceso con Arduino YUN por Camilo Núñez Fernández se distribuye bajo 
una Licencia Creative Commons Atribución-NoComercial 4.0 Internacional.
Este documente es de libre publicación y cuenta con la licencia Commons Creative, 
no se permite su uso comercial y esta exento de modificaciones.

******************************************************************************************/

#include <SoftwareSerial.h>
#include <Bridge.h>
#include <Process.h>

SoftwareSerial rfid(10,11); // Definimos el puerto Serial para el lector ID-20 RX,TX (el RX es para el ID-20 y el TX es para el LCD Serial).

int ledDenegado = 9;  //
int ledAceptado = 8;  //  Estos son pines del led RGB, para indicarnos las señasles de estado.
int ledControl = 7;   //

Process date; //inicio del procedo del tiempo EN el Arduino.


void setup() {
  
    Serial.begin(9600); // Iniciamos el puerto Serial
    rfid.begin(9600); //Iniciamos el puerto para el ID-20 y el LCD Serial
    Bridge.begin(); // Iniciamos todos los procesos de Bridge
    while (!Serial); // OJO esto es necesario par aque parte el void loop cuando nos conetamos por el monitor serial. Ademas, si quieren dejarlo sin incio serial, deben comentarlo con // para que deje de funcionar.
    
    pinMode(ledDenegado, OUTPUT); //
    pinMode(ledAceptado, OUTPUT); // Salida de los pines RGB
    pinMode(ledControl, OUTPUT);  //
    
    /*---Inicio del proceso para obtener el tiempo ---*/
    if (!date.running())  {
      date.begin("date");
      date.addParameter("+%T");
      date.run();
    }
    /*--- FIN del proceso para obtener el tiempo ---*/
    
    
    mensajeBienvenida(); //Muestra el mensaje de bienvenida en el LCD
    
    Serial.println("Procesos iniciados.");
    Serial.println("Listo para leer");

}



void loop () {
  
  /* -- Estos son los datos temporales para obtner el String con el codigo RFID -- */
  String tagString;
  byte i = 0;
  byte val = 0;
  byte code[6];
  byte checksum = 0;
  byte bytesread = 0;
  byte tempbyte = 0;

  if(rfid.available() > 0) {
    if((val = rfid.read()) == 2) {
      bytesread = 0; 
      while (bytesread < 12) {
        if( rfid.available() > 0) { 
          val = rfid.read();
          if((val == 0x0D)||(val == 0x0A)||(val == 0x03)||(val == 0x02)) {
            break;
          }
          /* -- Esta es la conversion de datos Ascii/Hex -- */
          if ((val >= '0') && (val <= '9')) {
            val = val - '0';
          } else if ((val >= 'A') && (val <= 'F')) {
            val = 10 + val - 'A';
          }

          if (bytesread & 1 == 1) {
            code[bytesread >> 1] = (val | (tempbyte << 4));

            if (bytesread >> 1 != 5) { 
              checksum ^= code[bytesread >> 1];
            }
          } else {
            tempbyte = val;
          }

          bytesread++;
        } 
      } 
      if (bytesread == 12) {
        byte checksum = 0;
        for(int i = 0; i < 5; ++i)
            checksum ^= code[i];
            
        if(checksum == code[5]) {
          for (i=0; i<5; i++) {
            if (code[i] < 16){
              tagString += "0" + String(code[i], HEX);
            }else{
              tagString += String(code[i], HEX);
            } 
          }
          tagString.toUpperCase(); // el String resultante con el ID de la tarjeta se transforma por completo en mayuscula
          bytesread = 0;
           /* -- Aqui terminan los procesos para obtener el String con el codigo RFID, Recuenden que son TEMPORALES -- */
          /*-----------------------------------------------*/
          //En esta parte se termina la lectura de la targeta y se puede iniciar el trabajo con el string resultante con el codigo de la Tajeta RFID
          Serial.println("El codigo es:");
          Serial.print(tagString);
          Serial.println();
          Serial.print("Verificando ...");
          verificarRFID(tagString);
          controlRFID(tagString);
          /*-----------------------------------------------*/
        }else{
          /*-----------------------------------------------*/
          //En esta parte se presenta si se encuenrta un erroro en la verificacion del checksum,
          // esto no influye en el trabajo con el ID de la tarjeta ya que simplemente no se obtiene.
          Serial.print ("Error en el CHECKSUM, favor de revisar la tarjeta.");
          /*-----------------------------------------------*/
        }  
      }
    }
  }
}

//---------- Esta es la funcion que verifica si el String con el codigo de la tarjeta esta en la base de datos --------- //
void verificarRFID(String IDrfid){
  Process verificar;
  verificar.begin("python");
  verificar.addParameter("/mnt/sda1/arduino/python/comprobar.py");
  verificar.addParameter(IDrfid); // Esta es el resultado de la variable argv[1] del programa Python
  verificar.run();
  while (verificar.available()){
    String resultadoPython = verificar.readString(); // Con esto obtenemos la respuesta en forma de String desde el scritp de Python
    resultadoPython.trim(); // Limpiamos de espacios en el incio y fin del string obtenido
    if(resultadoPython == "1"){ //Estos son los procesos que realiza si el codigo de la tarjeta esta en la base de datos, el scritp de Python devuelve un "1" si esta.
      Serial.print("Acceso aceptado");
      Serial.println();
      digitalWrite(ledAceptado, HIGH);
      delay(1000);
      digitalWrite(ledAceptado, LOW);
      delay(1000);
      mensajeAceptado(IDrfid);
      break;
    }else{ //Estos son los procesos que realiza si el codigo de la tarjeta NO esta en la base de datos, el scritp de Python devuelve un "2" si NO esta.
      Serial.print("Acceso denegado");
      Serial.println();
      digitalWrite(ledDenegado, HIGH);
      delay(1000);
      digitalWrite(ledDenegado, LOW);
      delay(1000);
      mensajeDenegado(IDrfid);
      break;
    }
  }
}

//---------------------------- Estas las funciones de trabajo para el LCD Serial ----------------------------- //
void clearScreen()
{
  //limpia el LCD de registros.
  rfid.write(0xFE);
  rfid.write(0x01); 
}
//--------------------------------------------------------------------------------------------------------------
void selectLineOne()
{ 
  // pone el cursor en la línea 1 del LCD
  rfid.write(0xFE); //command flag
  rfid.write(128); //position
}
//--------------------------------------------------------------------------------------------------------------
void selectLineTwo()
{ 
  // pone el cursor en la línea 2 del LCD
  rfid.write(0xFE); //comando
  rfid.write(192); //posicion
}
//--------------------------------------------------------------------------------------------------------------
void moveCursorRightOne()
{
  // mueve el cursor hacia la derecha
  rfid.write(0xFE); //command flag
  rfid.write(20); // 0x14
}
//--------------------------------------------------------------------------------------------------------------
void moveCursorLeftOne()
{
  // mueve el cursor hacia la izquierda
  rfid.write(0xFE); //comando
  rfid.write(16); // 0x10
}
//--------------------------------------------------------------------------------------------------------------
void scrollRight()
{
  rfid.write(0xFE); //comando
  rfid.write(20); // 0x14
}
//--------------------------------------------------------------------------------------------------------------
void scrollLeft()
{
  rfid.write(0xFE); //comando
  rfid.write(24); // 0x18
}
//--------------------------------------------------------------------------------------------------------------
void turnDisplayOff()
{
  // esto apaga la pantalla
  rfid.write(0xFE); //comando
  rfid.write(8); // 0x08
}
//--------------------------------------------------------------------------------------------------------------
void turnDisplayOn()
{
  // esto prende la pantalla
  rfid.write(0xFE); //comando
  rfid.write(12); // 0x0C
}
//--------------------------------------------------------------------------------------------------------------
void boxCursorOn()
{
  rfid.write(0xFE); //comando
  rfid.write(13); // 0x0D
}
//--------------------------------------------------------------------------------------------------------------
void boxCursorOff()
{
  rfid.write(0xFE); //comando
  rfid.write(12); // 0x0C
}
//--------------------------------------------------------------------------------------------------------------
void toggleSplash()
{
  rfid.write(0x7C); //comando
  rfid.write(9); // 0x09
}
//--------------------------------------------------------------------------------------------------------------
int backlight(int brightness) //128 = OFF, 157 = Totalmente ON, todo en el medio = variada brillo 
{
  // esta función toma un int entre 128-157 y apaga la iluminación en consecuencia
  rfid.write(0x7C);
  rfid.write(brightness);
}
//---------------------------- Fin de las funciones de trabajo para el LCD Serial ----------------------------- //



/*-------- Estos con la funciones para obtener un mesaje de Aceptado o denejado si el codigo de la tarjeta esta en la base de datos, esto lo muestra en el LCD Serial ---- */
void mensajeAceptado(String idTAG)
{
  clearScreen();
  selectLineOne();
  rfid.print("Acceso  Aceptado");
  selectLineTwo();
  rfid.print("Su ID:");
  rfid.print(idTAG);
  delay(2000);
  clearScreen();
}
void mensajeDenegado(String idTAG)
{
  clearScreen();
  selectLineOne();
  rfid.print("Acceso  Denegado");
  selectLineTwo();
  rfid.print("Su ID:");
  rfid.print(idTAG);
  delay(2000);
  clearScreen();
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


/* --------- Este es el mesaje de bienvenida que se muestra en el LCD Display, con la hora ---*/
void mensajeBienvenida(){
  clearScreen();
  selectLineOne();
  rfid.print("Saludos Amigo !!");
  selectLineTwo();
  rfid.print("Son las:");
  rfid.print(tiempo());
  delay(2000);
  clearScreen();
  selectLineOne();
  rfid.print("Los procesos ya ");
  selectLineTwo();
  rfid.print("estan preparados");
  delay(2000);
  clearScreen();
}
/*--------------------------------------------------------------------------------------------*/


/*----- Con esta funcion obetenmos un String con el tiempo del Arduino YUN -----*/
String tiempo(){
  Process date;
  date.begin("date");
  date.addParameter("+%T");
  date.run();
  if (date.available()) {
    String timeString = date.readString();    
    return timeString;
  }
 date.close(); 
}
/*-------------------------------------------------------------------------------*/

/*---------- esta es la funcion que envia los datos para guardar en la Base del Arduino en la tabla de ControlUsuarios -----*/
void controlRFID(String IDtarjeta){
  Process controlRFID;
  controlRFID.begin("python");
  controlRFID.addParameter("/mnt/sda1/arduino/python/control.py");
  controlRFID.addParameter(IDtarjeta); //argv[1] para control
  controlRFID.run();
  if (controlRFID.available()){
    String resultadoControl = controlRFID.readString(); // respuesta
    resultadoControl.trim();
    clearScreen();
    selectLineOne();
    Serial.print("Se ha guardado");
    rfid.print("Se ha guardado");
    if(resultadoControl == "1"){  //Datos gurdados correctamente
      byte ij = 0;
      for (ij=1; ij<=2; ij++) {
        digitalWrite(ledControl, HIGH);
        delay(200);
        digitalWrite(ledControl, LOW);
        delay(200);
      }
      selectLineTwo();
      rfid.print("Bien su RFID c:");
      delay(1000);
      clearScreen();
    }
    else if(resultadoControl != "1"){
      byte ij = 0;
      for (ij=1; ij<=2; ij++) {
        digitalWrite(ledControl, HIGH);
        delay(400);
        digitalWrite(ledControl, LOW);
        delay(400);
         digitalWrite(ledDenegado, HIGH);
        delay(400);
        digitalWrite(ledDenegado, LOW);
        delay(400);
      }
      Serial.print("Mal su RFID :c");
      selectLineTwo();
      rfid.print("Mal su RFID :c");
      delay(1000);
      clearScreen();
    }else{
      Serial.print("No conecto el archivo");
    }
  }  
}
/*---------------------------------------------------------------------------------------------------------------------------------*/

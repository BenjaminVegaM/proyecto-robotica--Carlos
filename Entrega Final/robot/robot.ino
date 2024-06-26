#define TRIGGER_PIN 12
#define ECHO_PIN 11
#define INT1 5
#define INT2 6
#define INT3 9
#define INT4 10
#include <Arduino_APDS9960.h>
#include <SoftwareSerial.h>
#include <LinkedList.h> // Necesitarás instalar esta biblioteca
#include <Wire.h>
#include <SparkFun_APDS9960.h>
SoftwareSerial BT1(3,2);

SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;

// Dimensiones de la matriz
const int filas = 7; // Número de filas
const int columnas = 7; // Número de columnas

  int mapa[filas][columnas] = {
    {1, 1, 1, 1, 1, 1, 1},
    {1, 0, -1, 0, 0, 1, 1},
    {1, 0, 1, 0, 0, 0 ,1},
    {1, 0, 1, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1},
    {1, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1}
  };

struct Punto {
  int x, y;
};

enum Accion {
  ADELANTE,
  GIRO_DERECHA,
  GIRO_IZQUIERDA
};

// Estructura para representar un nodo en el algoritmo A*
struct Nodo {
  Punto posicion;
  int costoG;
  int costoH;
  int costoF() { return costoG + costoH; }
  Nodo *padre;
};

LinkedList<Nodo*> abrirLista;
LinkedList<Nodo*> cerrarLista;

// Función heurística (distancia de Manhattan)


int speedMotor = 0;
int lastMove = 0;
int del = 600;
bool moved= false;
int delForward = 220;
int delRotation = 72;
bool primerPaso = false;

// Declaración de la matriz
//int mapa[filas][columnas];
int adjacent[4];

// Inicialización de la matriz
void inicializarMapa() {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            mapa[i][j] = -1; // Casilla desconocida
        }
    }

    for (int i = 0; i < filas; i++) {
      mapa[i][0] = 1;
      mapa[i][columnas-1] = 1;
    }

    for (int i = 0; i < filas; i++) {
      mapa[0][i] = 1;
      mapa[filas-1][i] = 1;
    }
    // Establecer la casilla inicial como libre
    mapa[1][1] = 0;
}

int direction = 0;
int frontPosition = -1;
int leftPosition = -1;
int rightPosition = -1;
int backPosition = -1;
int posX = 1;
int posY = 1;
int value = -1;
bool advance = false;
bool rotate = false;
bool start = false;
bool route = false;
bool discovered = false;
Nodo *ruta;
Nodo *nodoActual;
String color = "rojo";

long distancia = 20;
void setup(){
  inicializarMapa();
 BT1.begin(9600);
 Serial.begin(9600);

 pinMode(TRIGGER_PIN, OUTPUT);
 pinMode(ECHO_PIN, INPUT);
 pinMode(INT1,OUTPUT);
 pinMode(INT2,OUTPUT);
 pinMode(INT3,OUTPUT);
 pinMode(INT4,OUTPUT);

 if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Start running the APDS-9960 light sensor (no interrupts)
  if ( apds.enableLightSensor(false) ) {
    Serial.println(F("Light sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during light sensor init!"));
  }
  
  // Wait for initialization and calibration to finish
  delay(500);

}
void loop(){
  
 distancia=Ultrasonido(TRIGGER_PIN, ECHO_PIN);
  int umbral = 15;
  if (  !apds.readAmbientLight(ambient_light) ||
        !apds.readRedLight(red_light) ||
        !apds.readGreenLight(green_light) ||
        !apds.readBlueLight(blue_light) ) {
    Serial.println("Error reading light values");
    color = "No disponible";
  } else {
    if(green_light == red_light && red_light == blue_light){
      color = "Negro";
    }else if(red_light > green_light && red_light > blue_light){
      color = "Rojo";
    }else if(green_light > red_light && green_light > blue_light){
      color = "Verde";
    }else if(red_light > blue_light && green_light > blue_light && abs(red_light - green_light) < umbral){
      color = "Amarillo";
    }else{
      color = "Otro"; // Si no es ninguno de los anteriores
    }
  }

  

  //Conseguir vecinos 
  if(route == false){
    
    if(primerPaso){
      imprimirDatos();
    getAdjacent(posX, posY);

    //Dererminar casilla del frente
    
    if(direction == 0){
      frontPosition = adjacent[0];
      leftPosition = adjacent[3];
      rightPosition = adjacent[1];
      backPosition = adjacent[2];
    }else if(direction == 90){
      frontPosition = adjacent[3];
      leftPosition = adjacent[2];
      rightPosition = adjacent[0];
      backPosition = adjacent[1];
    }else if(direction == 180){
      frontPosition = adjacent[2];
      leftPosition = adjacent[1];
      rightPosition = adjacent[3];
      backPosition = adjacent[0];
    }else if(direction == 270){
      frontPosition = adjacent[1];
      leftPosition = adjacent[0];
      rightPosition = adjacent[2];
      backPosition = adjacent[3];
    }


    //Determinar si la casilla del frente está explorada o si se puede avanzar
    if (frontPosition == -1) {
      if (distancia > 16) {
          value = 0;
          advance = true;
          rotate = false;
      } else {
          value = 1;
          advance = false;
          rotate = true;
      }
      discovered = true;
    } else {
      
      if (discovered) {
          if(frontPosition == 0){
            advance = true;
            rotate = false;
          }else{
            advance = false;
            rotate = true;
          }
          
      } else {
          if (frontPosition != 1 && (rightPosition != 1 || leftPosition != 1 || backPosition != 1)) {
            // Avanzamos solo si frontPosition no es igual a 1
            advance = true;
            rotate = false;
        } else {
            advance = false;
            rotate = true;
        }
      }
    }


      if(frontPosition != -1 && backPosition != -1 && rightPosition != -1 && leftPosition != -1 && !discovered){
          Punto inicio ={posX, posY};
          Punto fin = buscarInexplorados();
          if(fin.x == -1 || fin.y == -1){
            fin = {0,0};
          }
          ruta = encontrarRuta(mapa, inicio,fin);
          invertirRuta(ruta);
          ruta = ruta -> padre;
          route = true;
          advance = false;
          rotate = false;
        }
    
      if(direction == 0 && frontPosition == -1 && !route){
          mapa[posX+1][posY] = value;
      }else if(direction == 90 && frontPosition == -1 && !route){
          mapa[posX][posY-1] = value;
      }else if(direction == 180 && frontPosition == -1 && !route){
          mapa[posX-1][posY] = value;
      }else if(direction == 270 && frontPosition == -1 && !route){
          mapa[posX][posY+1] = value;
      }

    //Avanzar hacia adelante
    speedMotor = 240;
    moved = false;
    
    if(advance){
      if(lastMove == 0){
        forward();
         if(direction == 0){
            posX++;
        }else if(direction == 90){
            posY--;
        }else if(direction == 180){
            posX--;
        }else if(direction == 270){
            posY++;
        }
        discovered = false;
        Serial.println(("Al frente"));
        del = delForward;
        moved = true;
        lastMove = 1;
      }
    }else if(rotate){
      if(lastMove == 0){

        if(rightPosition == -1 || rightPosition == 0){
          right();
          Serial.println(("Derecha"));
          direction = (direction - 90 + 360) % 360; 

        }else if(leftPosition == -1 || leftPosition == 0){
          left();
          Serial.println(("Izquierda"));
          direction = (direction + 90) % 360;

        }
        moved = true;
        del = delRotation;
        lastMove = 1;

      }
    }

    if(lastMove == 1 && !moved){
      stop();
      Serial.println(("Detenerse"));
      lastMove = 0;
      del = 200;
      moved = false;
    }

    imprimirMapa();

    }
    
  }else if(route == true){
    del = 300;
    advance = false;
    rotate = false;
    //Serial.println("astar");
    if(ruta != nullptr){
      //imprimirRuta(ruta);
      int movX = 0;
      int movY = 0;
      Serial.println(ruta->posicion.y);
      Serial.println(ruta->posicion.x);
      if(ruta->posicion.y != posY){
      movY = ruta->posicion.y - posY;
        if(movY == -1){
          if(direction == 90){
            advance = true;
          }else{
            rotate = true;
          }
        }else if(movY == 1){
          if(direction == 270){
            advance = true;
          }else{
            rotate = true;
          }
        }
      }else if(ruta->posicion.x != posX){
        movX = ruta->posicion.x - posX;
        if(movX == -1){
          if(direction == 180){
            advance = true;
          }else{
            rotate = true;
          }
        }else if(movX == 1){
          if(direction == 0){
              advance = true;
            }else{
              rotate = true;
            }
        }
      }

      if(advance){
        posX += movX;
        posY += movY;
        forward();
        ruta = ruta->padre;
        del = delForward;
      }else if(rotate){
        right();
        direction = (direction - 90 + 360) % 360;
        del = delRotation;
      }
      imprimirDatos();
      imprimirMapa();
    }else{
      stop();
      route = false;
      lastMove = 0;
      moved = false;
    }
  }
  primerPaso = true;
  enviarBluetooth();
 delay(del);
}


void motorA(char d, int velocity)
{
 if(d =='A'){
 analogWrite(INT1,LOW);
 analogWrite(INT2,velocity);
 }else if (d =='R'){
 analogWrite(INT1,velocity);
 analogWrite(INT2,LOW);
 }else{
 digitalWrite(INT1,LOW);
 digitalWrite(INT2,LOW);
 }
}

void motorB(char d,int velocity)
{
 if(d =='A'){
 analogWrite(INT3,LOW);
 analogWrite(INT4,velocity);
 }else if (d =='R'){
 analogWrite(INT3,velocity);
 analogWrite(INT4,LOW);
 }else{
 analogWrite(INT3,LOW);
 analogWrite(INT4,LOW);
 }
}

void forward(){
  motorA('R', speedMotor);
  motorB('R', speedMotor-15);
  //Serial.println(("Al frente"));
}

void right(){
  motorA('R', speedMotor);
  motorB('A', speedMotor);
  //serial.println(("Giro derecha"));
}

void left(){
  motorA('A', speedMotor);
  motorB('R', speedMotor);
  //serial.println(("Giro izquierda"));
}

void stop(){
  speedMotor = 0;
  motorA('R', speedMotor);
  motorB('R', speedMotor);
  //Serial.println(("Detenerse"));
}

void enviarBluetooth(){
    BT1.println("");
    BT1.print("dir:");
    BT1.print(direction);
    BT1.print(", dist:");
    BT1.print(distancia);
    BT1.print(", X:");
    BT1.print(posX);
    BT1.print(", Y:");
    BT1.print(posY);
    BT1.print(", color:");
    BT1.print(color);
}

void getAdjacent(int posX, int posY){
  //int adjacent[4];
  adjacent[0] = mapa[posX+1][posY];
  adjacent[1] = mapa[posX][posY+1];
  adjacent[2] = mapa[posX-1][posY];
  adjacent[3] = mapa[posX][posY-1];
  //return adjacent;
}

long Ultrasonido(int trigger, int eco){
 long duration; //timepo que demora en llegar el eco
 long distance; //distancia en centimetros

 digitalWrite(trigger,LOW);
 delayMicroseconds(2);
 digitalWrite(trigger,HIGH);
 delayMicroseconds(10);
 digitalWrite(trigger,LOW);
 duration = pulseIn(eco, HIGH); //obtenemos el ancho del pulso
 distance = (duration*.0343)/2;
 return distance;
}

void imprimirMapa() {
  for (int j = 0; j < columnas; j++) {
    for (int i = 0; i < filas; i++) {
      if(posX == i && posY == j){
        Serial.print(" X");
      }else{
        if(mapa[i][j] != -1){
          Serial.print(" ");
        }
        Serial.print(mapa[i][j]);
      }
      Serial.print(" "); // Espacio entre números
    }
    Serial.println(); // Nueva línea al final de cada columna
  }
  Serial.println(); // Línea adicional para separar impresiones sucesivas
}

void imprimirDatos(){
  Serial.print("A la izquierda: ");
    Serial.println(leftPosition);
    Serial.print("A la derecha: ");
    Serial.println(rightPosition);
    Serial.print("Dirección a la que mira: ");
    Serial.println(direction);
    Serial.print("Distancia: ");
    Serial.println(distancia);
}

int heuristica(Punto a, Punto b) {
  return abs(a.x - b.x) + abs(a.y - b.y);
}

// Verifica si un punto está dentro de la matriz y no es un obstáculo
bool esValido(int matriz[][columnas], Punto p) {
  return (p.x >= 0) && (p.x < filas) && (p.y >= 0) && (p.y < columnas) &&
         (matriz[p.x][p.y] == 0);
}

Accion determinarAccion(Punto actual, Punto siguiente) {
  if (siguiente.x > actual.x) {
    return ADELANTE;
  } else if (siguiente.y > actual.y) {
    return GIRO_DERECHA;
  } else if (siguiente.y < actual.y) {
    return GIRO_IZQUIERDA;
  }
  // Añadir más condiciones si es necesario
}

LinkedList<Accion> convertirEnAcciones(LinkedList<Punto> ruta) {
  LinkedList<Accion> listaAcciones;
  for (int i = 0; i < ruta.size() - 1; i++) {
    Punto actual = ruta.get(i);
    Punto siguiente = ruta.get(i + 1);
    Accion accion = determinarAccion(actual, siguiente);
    listaAcciones.add(accion);
  }
  return listaAcciones;
}

Nodo* encontrarRuta(int matriz[filas][columnas], Punto inicio, Punto fin) {
  Serial.println("Inicio");
  Nodo *nodoInicial = new Nodo();
  nodoInicial->posicion = inicio;
  nodoInicial->padre = nullptr;
  nodoInicial->costoG = 0;
  nodoInicial->costoH = heuristica(inicio, fin);
  abrirLista.add(nodoInicial);

  while (abrirLista.size() > 0) {
    Nodo *nodoActual = abrirLista.get(0);
    for (int i = 1; i < abrirLista.size(); i++) {
      if (abrirLista.get(i)->costoF() < nodoActual->costoF() ||
          (abrirLista.get(i)->costoF() == nodoActual->costoF() && abrirLista.get(i)->costoH < nodoActual->costoH)) {
        nodoActual = abrirLista.get(i);
      }
    }

    abrirLista.remove(nodoActual);
    cerrarLista.add(nodoActual);

    if (nodoActual->posicion.x == fin.x && nodoActual->posicion.y == fin.y) {
      return nodoActual; // Ruta encontrada
    }
    
    // Generar sucesores...
    generarSucesores(nodoActual, fin, matriz);
  }

  return nullptr; // No se encontró ruta
}

void accionesDeRuta(Nodo *nodoFinal) {
  LinkedList<Punto> ruta;
  Nodo *nodoActual = nodoFinal;
  while (nodoActual != nullptr) {
    ruta.add(nodoActual->posicion);
    nodoActual = nodoActual->padre;
  }

  // Aquí iría el código para convertir la lista de puntos en acciones
  // Por ejemplo, comparar la posición actual con la siguiente para determinar si se avanza o se gira

  LinkedList<Punto> rutaInvertida;
  for (int i = ruta.size() - 1; i >= 0; i--) {
    rutaInvertida.add(ruta.get(i));
  }

  // Convertir la lista de puntos en acciones
  LinkedList<Accion> acciones = convertirEnAcciones(rutaInvertida);

  // Imprimir las acciones o manipular según sea necesario
  for (int i = 0; i < acciones.size(); i++) {
    switch (acciones.get(i)) {
      case ADELANTE:
        Serial.println("Adelante");
        break;
      case GIRO_DERECHA:
        Serial.println("Giro a la derecha");
        break;
      case GIRO_IZQUIERDA:
        Serial.println("Giro a la izquierda");
        break;
    }
  }
}

bool estaEnCerrarLista(Punto p) {
  for (int i = 0; i < cerrarLista.size(); i++) {
    if (cerrarLista.get(i)->posicion.x == p.x && cerrarLista.get(i)->posicion.y == p.y) {
      return true;
    }
  }
  return false;
}

// Función para generar sucesores de un nodo actual
void generarSucesores(Nodo *nodoActual, Punto fin, int matriz[][columnas]) {
  // Direcciones posibles a moverse: Arriba, Derecha, Abajo, Izquierda
  Punto direcciones[4] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
  
  for (int i = 0; i < 4; i++) {
    Punto nuevaPos = {nodoActual->posicion.x + direcciones[i].x, nodoActual->posicion.y + direcciones[i].y};
    
    // Si la nueva posición es válida y no está en la lista cerrada
    if (esValido(matriz, nuevaPos) && !estaEnCerrarLista(nuevaPos)) {
      Nodo *sucesor = new Nodo;
      sucesor->posicion = nuevaPos;
      sucesor->padre = nodoActual;
      sucesor->costoG = nodoActual->costoG + 1; // Costo del movimiento, asumimos que siempre es 1
      sucesor->costoH = heuristica(sucesor->posicion, fin);
      
      // Verificar si el sucesor ya está en la lista abierta con un costo mayor
      bool enAbrirConMayorCosto = false;
      for (int j = 0; j < abrirLista.size(); j++) {
        Nodo *nodoEnAbrir = abrirLista.get(j);
        if (nodoEnAbrir->posicion.x == sucesor->posicion.x && nodoEnAbrir->posicion.y == sucesor->posicion.y) {
          if (nodoEnAbrir->costoG > sucesor->costoG) {
            abrirLista.remove(j); // Removemos el nodo con mayor costo
            abrirLista.add(sucesor); // Añadimos el sucesor con menor costo
          }
          enAbrirConMayorCosto = true;
          break;
        }
      }
      
      // Si no está en la lista abierta o no está con un costo mayor, lo añadimos
      if (!enAbrirConMayorCosto) {
        abrirLista.add(sucesor);
      }
    }
  }
}

void invertirRuta(Nodo *&ruta) {
  Nodo *actual = ruta;
  Nodo *anterior = NULL;
  Nodo *siguiente = NULL;

  while (actual != NULL) {
    siguiente = actual->padre; // Guarda el siguiente nodo
    actual->padre = anterior;  // Invierte el puntero del padre
    anterior = actual;         // Mueve el puntero anterior al nodo actual
    actual = siguiente;        // Avanza al siguiente nodo
  }
  ruta = anterior; // Actualiza la ruta al nuevo primer nodo
}

void imprimirRuta(Nodo *nodoFinal) {
  // Primero, construimos la lista de puntos desde el final hasta el inicio
  LinkedList<Punto> ruta;
  Nodo *nodoActual = nodoFinal;
  while (nodoActual != nullptr) {
    ruta.add(nodoActual->posicion);
    nodoActual = nodoActual->padre;
  }

  // Luego, invertimos la lista para que la ruta vaya desde el inicio hasta el final
  for (int i = ruta.size() - 1; i >= 0; i--) {
    Punto p = ruta.get(i);
    Serial.print("(");
    Serial.print(p.x);
    Serial.print(", ");
    Serial.print(p.y);
    Serial.print(")");
    if (i > 0) {
      Serial.print(" -> ");
    }
  }
  Serial.println(); // Para terminar la línea después de imprimir la ruta completa
}

Punto buscarInexplorados(){
  Punto masCercano = { -1, -1 };
  int distanciaMinima = filas + columnas;
  for (int j = 0; j < columnas; j++) {
    for (int i = 0; i < filas; i++) {
      if(mapa[i][j] == 0){
        if(mapa[i+1][j] == -1 || mapa[i-1][j] == -1 || mapa[i][j+1] == -1 || mapa[i][j-1] == -1){
          int distancia = abs(posX - i) + abs(posY - j);
          if(distancia < distanciaMinima){
            masCercano.x = i;
            masCercano.y = j;
            distanciaMinima = distancia;
          }
        }
      }
    }
  }
  return masCercano;
}

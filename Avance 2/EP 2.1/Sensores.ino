#define TRIGGER_PIN 12
#define ECHO_PIN 11
#define INT1 5
#define INT2 6
#define INT3 9
#define INT4 10

int speedMotor = 0;

long distancia;
void setup(){
 Serial.begin(9600);
 pinMode(TRIGGER_PIN, OUTPUT);
 pinMode(ECHO_PIN, INPUT);
 pinMode(INT1,OUTPUT);
 pinMode(INT2,OUTPUT);
 pinMode(INT3,OUTPUT);
 pinMode(INT4,OUTPUT);

}
void loop(){

 distancia=Ultrasonido(TRIGGER_PIN, ECHO_PIN);
 Serial.println((distancia));

//if (Serial.available() > 0) {
//    speedMotor = Serial.parseInt();
//  }


  //motorA('R', speedMotor);
  //motorB('R', speedMotor);

 delay(200);
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

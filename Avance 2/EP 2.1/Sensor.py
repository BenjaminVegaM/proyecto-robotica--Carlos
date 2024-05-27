import serial
import time
import json

arduino = serial.Serial('COM3', 9600)
i = 0
distancias = []
stop = False
ciclos = 100

while stop == False:
    if ciclos <= 0:
        stop = True
    time.sleep(0.2)
    data = arduino.readline()
    distancia = int(data)
    distancias.append(distancia)
    if(distancia <= 400):
        ciclos -= 1
        print("Distancia: ", distancia)

with open('distancias.json', 'w') as archivo:
    json.dump(distancias, archivo)
arduino.close()


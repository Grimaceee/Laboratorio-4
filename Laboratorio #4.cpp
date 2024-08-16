from machine import Pin, PWM, ADC
import time

servo = PWM(Pin(14), freq=50)
rojo = PWM(Pin(12), freq=5000)
verde = PWM(Pin(13), freq=5000)
azul = PWM(Pin(27), freq=5000)

boton1 = Pin(4, Pin.IN, Pin.PULL_UP)
boton2 = Pin(5, Pin.IN, Pin.PULL_UP)
boton3 = Pin(18, Pin.IN, Pin.PULL_UP)
boton4 = Pin(19, Pin.IN, Pin.PULL_UP)

servo_min = 40
servo_max = 115
posicion_servo = 77

colores = [rojo, verde, azul]
indice_color = 0
brillo = 0
modo = 0

adc = ADC(Pin(36))
adc.width(ADC.WIDTH_12BIT)
adc.atten(ADC.ATTN_11DB)

def mover_servo(posicion):
    posicion = max(min(posicion, servo_max), servo_min)
    servo.duty(posicion)

def ajustar_brillo(led, brillo):
    ciclo_trabajo = int(brillo / 100 * 1023)
    led.duty(ciclo_trabajo)

def mapear_rango(x, min_in, max_in, min_out, max_out):
    return (x - min_in) * (max_out - min_out) // (max_in - min_in) + min_out

while True:
    if not boton1.value():
        posicion_servo += 1
        mover_servo(posicion_servo)
        time.sleep(0.1)
    
    if not boton2.value():
        posicion_servo -= 1
        mover_servo(posicion_servo)
        time.sleep(0.1)

    if not boton3.value():
        indice_color = (indice_color + 1) % 4
        if indice_color == 3:
            modo = 1
        else:
            modo = 0
        time.sleep(0.3)
    
    if not boton4.value() and modo == 0:
        brillo += 10
        if brillo > 100:
            brillo = 0
        ajustar_brillo(colores[indice_color], brillo)
        time.sleep(0.3)

    if modo == 1:
        posicion_adc = adc.read()
        valor_rojo = mapear_rango(posicion_adc, 0, 4095, 0, 1023)
        valor_verde = mapear_rango(posicion_adc, 0, 4095, 1023, 0)
        valor_azul = mapear_rango(posicion_adc, 0, 4095, 0, 1023)

        rojo.duty(valor_rojo)
        verde.duty(valor_verde)
        azul.duty(valor_azul)

    time.sleep(0.1)

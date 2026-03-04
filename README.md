hace un tiempo compre en ali express esta cosita....
![esp32matrix1](https://github.com/user-attachments/assets/9122d3a7-0c69-4042-b326-7ec08ad73e53)
![esp32matrix2](https://github.com/user-attachments/assets/a6f5e992-4ea1-4b2f-8697-119879209f23)
no tengo mas documentacion del producto porque ya no esta disponible, solo me quedo esta pequña imagen en el historial de mis compras.

El producto es simple, un esp32 con panel matricial rgb de 64x64 con un reloj preinstalado de super mario, muy lindo , y con la promesa de que, se le podia instalar otros motivos de relojoes ingresando a la pagina de 
clockwise... pues no es tan facil...

basicamente esto anda solamente con el firmware ya trae... 
al reinstalarle cualquier otra cosa desde clockwise lo que pasa es que la mitad del panel deja de funcionar, porque la conexion del panel matricial con el ESP tiene de estandar lo que yo de modelo de calendario asi que me di a la tarea de mapear las conecciones y ya que estamos hacer algo un poco mas interesante que un reloj

Buzzer = GPIO2 Activo-bajo (GPIO2 = 0 suena, GPIO2 = 1 se calla)

LDR = GPIO34 (ADC1)


esta aberracion es la coneccion a la matriz como viene:

1=GPIO25
2=GPIO27
3=GPIO14
4=GPIO09
5=GPIO23
6=GPIO05
7=GPIO16
8=GPIO15
9=GND
10=GPIO04
11=GPIO17
12=GPIO19
13=GPIO32=====(gpio32-PIN E MUY IMPORTANTE)
14=GPIO12
15=GND
16=GPIO26







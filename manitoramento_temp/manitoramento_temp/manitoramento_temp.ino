#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Configurações de Pinos
#define DHTPIN 8
#define DHTTYPE DHT11

#define R 9
#define G 10
#define B 11

// Inicialização dos componentes
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// Função auxiliar para controlar o LED RGB
void setCor(bool r, bool g, bool b) {
  digitalWrite(R, r);
  digitalWrite(G, g);
  digitalWrite(B, b);
}

void setup() {
  // Inicializa LCD e Sensor
  lcd.init();
  lcd.backlight();
  dht.begin();

  // Configura pinos do LED
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  // Inicia Comunicação Serial para a Interface Web (9600 baud)
  Serial.begin(9600);

  // Tela de Inicialização
  lcd.print("Monitoramento");
  lcd.setCursor(0, 1);
  lcd.print("SmartFridge");
  setCor(LOW, LOW, HIGH); // Inicia em azul
  delay(2000);
  lcd.clear();
}

void loop() {
  // Leitura dos dados
  float t = dht.readTemperature();
  float u = dht.readHumidity();

  // Verifica se o sensor está respondendo
  if (isnan(t) || isnan(u)) {
    lcd.clear();
    lcd.print("Erro sensor");
    setCor(HIGH, LOW, LOW); // Vermelho para erro
    
    // Avisa a interface web sobre o erro
    Serial.println("{\"erro\": \"Sensor offline\"}");
    delay(2000);
    return;
  }

  // --- ENVIO DE DADOS PARA O PYTHON (INTERFACE WEB) ---
  // Formato JSON para leitura automática do script
  Serial.print("{\"temperatura\": ");
  Serial.print(t);
  Serial.print(", \"umidade\": ");
  Serial.print(u);
  Serial.println("}");

  // --- EXIBIÇÃO NO LCD ---
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(t, 1); // Uma casa decimal
  lcd.print(" C ");

  lcd.setCursor(0, 1);
  lcd.print("Umid: ");
  lcd.print(u, 1);
  lcd.print(" % ");

  // --- LÓGICA DE ALERTAS (LED E LCD) ---
  if (t < 28 && u < 65) {
    // ESTADO SEGURO
    setCor(LOW, HIGH, LOW); // Verde
  } else if (t < 30 && u < 70) {
    // ESTADO DE ATENÇÃO
    setCor(HIGH, HIGH, LOW); // Amarelo
  } else {
    // ESTADO CRÍTICO
    setCor(HIGH, LOW, LOW); // Vermelho
  }

  delay(2000); // Aguarda 2 segundos para a próxima leitura
}

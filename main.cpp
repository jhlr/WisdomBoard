#include <WiFi.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "SEU_SSID"
#define WIFI_PASSWORD "SUA_SENHA"
#define FIREBASE_HOST "SEU_PROJETO.firebaseio.com"
#define FIREBASE_AUTH "SEU_TOKEN_FIREBASE"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const int pinPIR = 21;
const int pinLDR = 34;
const int pinLED = 2;
const int LIMIAR_CLARO = 1500;
const unsigned long TEMPO_VAZIO_MS = 5UL * 60UL * 1000UL;

unsigned long lastMotion = 0;
String estadoAtual = "";

void setup() {
	Serial.begin(115200);
	pinMode(pinPIR, INPUT);
	pinMode(pinLED, OUTPUT);

	Serial.println("Conectando ao Wi-Fi");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nWi-Fi conectado!");

	config.host = FIREBASE_HOST;
	config.signer.tokens.legacy_token = FIREBASE_AUTH;
	Firebase.begin(&config, &auth);
	Firebase.reconnectWiFi(true);
}

void enviarEstado(String estado) {
	if (estado != estadoAtual) {
		estadoAtual = estado;
		Firebase.setString(fbdo, "/lab/01/estado", estadoAtual);
		Firebase.setInt(fbdo, "/lab/01/timestamp", millis());
		Serial.println("Publicado: " + estadoAtual);
	}
}

void loop() {
	int pir = digitalRead(pinPIR);
	int ldr = analogRead(pinLDR);
	unsigned long agora = millis();

	if (pir == HIGH) {
		lastMotion = agora;
		if (ldr < LIMIAR_CLARO) {
			digitalWrite(pinLED, LOW);
			enviarEstado("OK:ocupado_luz_apagada");
		} else {
			digitalWrite(pinLED, HIGH);
			enviarEstado("OK:ocupado_luz_acesa");
		}
	} else {
		if (agora - lastMotion >= TEMPO_VAZIO_MS) {
			if (ldr >= LIMIAR_CLARO) {
				digitalWrite(pinLED, HIGH);
				enviarEstado("ALERTA:vazio_luz_acesa");
			} else {
				digitalWrite(pinLED, LOW);
				enviarEstado("OK:vazio");
			}
		}
	}
	delay(200);
}


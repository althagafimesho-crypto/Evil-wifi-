⚠️ ESP8266 Real-Verification Captive Portal 🚀
This project is an advanced Social Engineering & Security Testing tool based on the ESP8266. It creates a fake TP-Link login page and performs Real-Time Password Verification by attempting to connect to the target network.
🌟 Key Features
• Real-Time Verification ✅: It doesn't just save the password; it tries to connect to the actual router to verify if the internet works!
• Dual Mode (WiFi Want) 🔄:
• OFF (Hunting Mode): Broadcasts a fake open "TP-Link" network.
• ON (Admin Mode): Broadcasts a secure "Dr.Ahbat" network for configuration.
• Admin Panel 📱: A mobile-friendly web interface to scan and select the target network.
• OLED Display Support 📺: Shows live status, network names, and "Success" notifications.
• Discord Integration 🤖: Sends captured credentials and verification results to your Discord server via Webhook.
• Single Button Control 🔘: Use the BOOT button to switch modes (Long press 2s).
🛠️ Hardware Requirements
• ESP8266 (NodeMCU or Wemos D1 Mini).
• OLED Display (SSD1306 I2C).
• Built-in BOOT Button (GPIO 0).
⚙️ How it Works (Technical Logic) 🧠
1. The "WiFi Want" Logic 🛡️
The system toggles between two states using the physical BOOT button:
• Admin Mode: The ESP8266 acts as a secure Access Point. You connect to it to access the /scan page and pick your target.
• Hunting Mode: The ESP8266 acts as an open Access Point with a Captive Portal.
2. The Verification Engine 🔍
When a user submits a password on the fake page:
1.	The ESP8266 pauses the fake network.
2.	It switches to Station Mode and tries to log into the target SSID using the provided password.
3.	If WiFi.status() == WL_CONNECTED, the password is confirmed as CORRECT.
4.	It logs the result to LittleFS and sends a notification to Discord.
5.	It resumes the fake network so the user stays in the "syncing" loop.
3. File System (LittleFS) 📁
All logs are stored internally:
• /SUCCESS.txt: For verified working passwords.
• /FAKE.txt: For incorrect attempts.
🚀 Installation
1.	Install Arduino IDE.
2.	Add ESP8266 Board support.
3.	Install libraries: Adafruit_SSD1306, Adafruit_GFX, LittleFS.
4.	Paste your Discord Webhook URL in the code.
5.	Upload & Enjoy! ⚡
📜 Disclaimer
This project is for educational and ethical security testing purposes only. The author is not responsible for any misuse or damage caused by this tool. Use it only on networks you own or have permission to test. 🛡️
Happy Hacking! 💻🔥

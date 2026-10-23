# Wi-Fi Analyzer Pro

Menü: `WiFi → Wi-Fi Analyzer Pro`. Mevcut Wi-Fi araçları ve bağlantı kodu korunur. Arduino/ESP-IDF'nin pasif tarama seçeneği kullanılır; probe, paket enjeksiyonu, deauth, yeni ağ bağlantısı veya trafik bozma eklenmez. Var olan bir bağlantının olağan trafiği devam edebilir; radyo sessizliği/spektrum ölçümü iddiası yoktur.

En çok 64 AP: SSID, BSSID, kanal, RSSI, SDK şifreleme tipi, gizli SSID göstergesi ve son görülme yaşı. Bant genişliği/standart bilgisi bu tarama API'sinden alınamadığında bilinmiyor/null bırakılır. 2,4 GHz kanalları SDK ülke ayarına göre taranır; 5 GHz yoktur.

Kanal ekranı ağ sayısı çubukları, RSSI grupları (>=-60 / >=-75 / daha zayıf) ve +/-4 yakın kanal merkezlerinin karşılaştırmasını sunar. Yakınlık hesabı 20 MHz varsayımına dayanır; gerçek bant genişliği bilinmediği için kesin girişim/kanal doluluğu/airtime yüzdesi değildir. Birden fazla AP karşılaştırmaya seçilebilir.

Seçilen BSSID için 30 s / 60 s / 5 dk RSSI geçmişi; en çok 61 örnek, tarama bitiminden sonra en az 5 saniye aralık. Kaybolan AP grafikte boşluk olarak görünür. Yeni veri geldiğinde çizilir; sürekli animasyon yoktur.

CSV/JSON: seçilen LittleFS veya başarıyla mount edilen SD'ye, uptime ve mevcutsa cihaz saatiyle kayıt. Saatin senkron olduğu ayrıca kanıtlanmaz. Varsayılan dışa aktarım maskelidir; SSID/BSSID oturuma özel tuzlu takma adla değiştirilir. Bu kriptografik anonimlik garantisi değildir. SSID kontrol baytları, ters bölü ve bozuk UTF-8 baytları ekranda/raporda \xNN olarak gösterilir; JSON bunu ssid_format alanında belirtir. Başında boşluk olsa da CSV formül başlangıçları metne çevrilir; tırnaklar kaçırılır. Bu yordamın 11 host testi vardır. Yazma/ölçüm/yeniden adlandırma hatasında başarı gösterilmez; eksik geçici rapor temizlenir.

AP/WebUI modu açıkken analiz reddedilir; aktif BLE HID varsa önce kapatılması istenir. Başka aktif tarama varsa sahiplenilmez. Boştan açılan STA modu çıkışta önceki moda döner; mevcut STA bağlantısı kasıtlı olarak kesilmez. İptalde yalnızca kendi taraması durdurulur. Yeni sürekli görev yoktur; SDK sonuçları her tarama sonrası bırakılır. Modül kayıtları 64 AP ile sınırlıdır; Arduino SDK geçici tarama önbelleği gördüğü AP sayısıyla ek bellek kullanabilir ve tahsis hatası verebilir. Başlamadan 70.000 bayt dahili heap eşiği kontrol edilir.

Fiziksel testler bekliyor: bilinen AP/kanal karşılaştırması; gizli SSID; tarama iptali; önce/sonra bağlantı; 20 aç/kapatta heap/PSRAM eğilimi; CSV/JSON parse; dolu FS ve SD çıkarma; Wi-Fi/BLE birlikteliği ve LCD titremesi.

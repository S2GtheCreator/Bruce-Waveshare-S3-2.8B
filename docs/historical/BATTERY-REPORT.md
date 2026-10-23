> Historical report for the original pre-publication build. It does not certify the sanitized release binary. See ../../BUILD-REPORT.md.

# Battery Manager Pro

Menü: `Config (Settings) → Battery Manager`. BAT terminal voltajı, yaklaşık yüzde, düşük voltaj, ölçüm geçerliliği ve güç/şarj durumunun bilinmediği gösterilir. Parlaklık ve ekran zaman aşımı mevcut Bruce ayarlarına bağlanır. Mevcut uyku/power menüsü korunur; yeni otomatik CPU düşürme, görevi kesen uyku veya yazılımsal güç kesme eklenmez.

Resmî şema: GPIO4 ADC1; R2 200 kΩ, R18 100 kΩ, ikisi %1. Nominal oran (R2+R18)/R18 = 3. Arduino analogReadMilliVolts IDF curve-fitting kalibrasyon yolunu kullanır; 12 bit/11 dB, 16 ölçümden uç değerleri çıkaran ortalama, 5 saniye örnekleme, 15 saniye eskime sınırı korunur. Demo cihazına ait ampirik offset bu cihaza körlemesine uygulanmaz. Multimetreyle kazanç ayarı NVS'ye kayıt edilir; hata/geçersiz okuma gizlenir.

Gerilim yüzdesi genel tek hücre LiPo tablosunun yaklaşık sonucudur; kalibre fuel gauge değildir. Yük, sıcaklık, pil tipi, ADC ve bölücü toleransı belirsizlik yaratır. Şarj entegresi boş BAT ucunu da yükseltebildiğinden voltaj tek başına pil varlığını kanıtlamaz. Yüzde, `Battery presence → I verified a battery is fitted` seçilene kadar gizlidir; durum çubuğu BAT ? gösterir. Bu kullanıcı onayıdır, otomatik sensör değildir. Pil çıkarılırsa onayı kapatın. Düşük voltaj 3 ardışık <=3,45 V ölçümle; bırakma >=3,60 V. Koruma devresinin yerine geçmez.

Güç donanımı: ETA6098 STAT yalnızca LED'e bağlıdır; MCU şarj/USB-VBUS algılama hattı yoktur. SW1/Q2 fiziksel BAT→BAT_OUT yolunu kontrol eder; MCU latch/power-cut pini yoktur. USB diyot yolu VCC'yi ayrıca besleyebilir. USB HID bağlı olması güç kaynağını veya şarj durumunu kanıtlamaz. USB/batarya kaynağı, şarj oluyor/tamamlandı otomatik durumları **Bilinmiyor** kalır.

USB çıkarılınca kapanma yazılımla çözüldü denmiyor. Fiziksel teşhis:

1. Doğru konnektörde takılı pilin BAT–GND voltajını USB yokken ölçün; RTC pili başlığını karıştırmayın.
2. SW1'in iki konumunda BAT_OUT–GND ölçün. Açık konumda pil yolunun BAT'a yakın gerilim taşıdığını doğrulayın; anahtar yönü tahmin edilmez.
3. USB çıkarılmış, anahtar açıkken VCC ve 3V3'ü ölçün. BAT var/BAT_OUT yoksa anahtar/Q2/yol; BAT_OUT var/3V3 yoksa besleme-regülatör yolu incelenmelidir. Bu sonuçlar ölçülmeden arıza yeri kesinleştirilemez.
4. LCD açılırken yük altında düşüşü kontrol edin; konnektör/pil/koruma devresini inceleyin. Koruma elemanlarını köprülemeyin.
5. BAT voltajını ekrandaki terminal ölçümüyle karşılaştırın; yalnızca gerçek multimetre değerine göre kalibre edin.
6. USB/batarya ile ayrı açılış ve USB çıkarma testini kaydedin. Bu testler yapılmadı; bağımsız batarya çalışması doğrulanmadı.

[Resmî şema](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8B/ESP32-S3-Touch-LCD-2.8B_schematic_diagram.pdf), SHA-256 `9c7d84529d421a86b907ff29063155ddb54a01bc1805d1f81769097b9af95947`. Host testi yalnızca yüzde/debounce modelini doğrular; gerçek ADC doğruluğunu veya güç yolunu doğrulamaz.

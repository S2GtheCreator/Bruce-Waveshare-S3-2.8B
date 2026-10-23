# Yükleme / geri dönüş

Kart: yalnızca Waveshare ESP32-S3-Touch-LCD-2.8B. Pro fiziksel test bekleyen adaydır. Çalışan yedek `backup/` içindedir. Partition ve LittleFS yerleşimi değişmedi. Tam flash silme gerekmez.

USB veri kablosunu bağlayın, seri monitörü kapatın ve Windows'ta gerçek COM portunu seçin. Gerekirse BOOT basılıyken RESET'e basıp bırakarak indirme moduna girin, ardından BOOT'u bırakın. Örnek COM5'i kendi portunuzla değiştirin. Komutları bu çıktı klasöründe çalıştırın.

```powershell
python -m pip install esptool
```

**Önerilen güncelleme — mevcut NVS/FS ayarlarını korur:**

```powershell
python -m esptool --chip esp32s3 --port COM5 --baud 460800 write-flash 0x10000 Bruce-1.16.1-ws28b-pro-app-0x10000.bin
```

Bu yol mevcut çalışan portun aynı partition/bootloader yerleşimini kullanır. İsterseniz önce cihazın tamamını yerel olarak yedekleyin:

```powershell
python -m esptool --chip esp32s3 --port COM5 read-flash 0x0 0x1000000 device-backup.bin
```

**Birleşik 0x0 dosyası:** bootloader/partition/app doğru adreslerde doğrulandı. Ancak tek parça yazım NVS'nin 0x9000–0xEFFF aralığını da kapsar; kalibrasyon/pil onayı/SDK ayarları sıfırlanabilir. LittleFS 0x480000'de başlar ve dosyada yer almaz; erase-flash çalıştırmayın.

```powershell
python -m esptool --chip esp32s3 --port COM5 --baud 460800 write-flash 0x0 Bruce-1.16.1-ws28b-pro-merged.bin
```

Yükleme sonrası RESET; BadUSB çalıştırmadan önce Windows/Bruce düzenini ve boş Notepad odağını kontrol edin. Testler otomatik başlamaz. Pil yüzdesi fiziksel varlık onayı yapılana kadar BAT ? olabilir.

**Ayarları koruyarak çalışan trq uygulamasına dönüş:**

```powershell
python -m esptool --chip esp32s3 --port COM5 --baud 460800 write-flash 0x10000 backup/Bruce-1.16.1-ws28b-trq-app-0x10000.bin
```

Tam eski birleşik dosya da `backup/Bruce-1.16.1-ws28b-trq-merged.bin` olarak korunmuştur; onun 0x0 yazımı da NVS'yi kapsar. Kaynak geri dönüş etiketi `stable-ws28b-trq`. Kaynak ZIP'te .pio/.git derleme önbelleği bulunmaz; Git geçmişi ayrı `development-history.bundle` dosyasındadır:

```powershell
git clone development-history.bundle project-git
cd project-git
git switch --detach stable-ws28b-trq
```

Pro kaynak derlemesi: proje klasöründe `pio run -e waveshare_28b`. Bağımlılıkları PlatformIO indirir. Donanım testi bu teslimatta yapılmadı.

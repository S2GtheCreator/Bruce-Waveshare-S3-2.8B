ARŞİV: Başarısız displayfix sürümüne aittir. Bu sürüm geri çekildi; güncel belge RECOVERY-BATARYA-RAPORU.md.

# USB yükleme ve ekran kabul testi

İmaj: Bruce-1.16.1-waveshare28b-displayfix-merged.bin, yükleme adresi 0x0. Sürüm: 1.16.1-waveshare28b-displayfix.
Yeni sürümün titreme düzeltmesi henüz fiziksel cihazda doğrulanmadı.
Eski çalışan BIN ve kaynak arşivi backup-working-28b klasöründe korunuyor.

## USB yükleme

COM7 aşağıdaki komutlarda örnektir; kartın gerçek portunu yazın.
Seri monitörü kapatın. Gerekirse BOOT'a basılı tutup RESET'e basın, BOOT'u bırakın.
USB CDC/JTAG bağlantısında esptool kartı ESP32-S3 olarak görmeli.

Tam cihaz yedeği almak isterseniz, yüklemeden önce:
~~~powershell
py -m pip install esptool
py -m esptool --chip esp32s3 --port COM7 --baud 460800 read-flash 0x0 0x1000000 board-backup-16MB.bin
~~~
Bu, teslim edilen eski BIN yedeğine ek olarak güncel NVS/flash içeriğini korur.
SD içeriği ayrıca yedeklenir. Birleşik 0x0 imajı NVS aralığını da kapsar;
yükleme sonrası kayıtlı ayarlar/bağlantı bilgileri sıfırlanabilir.

Bu dosyayı indirdiğiniz/aldığınız klasörde:
~~~powershell
py -m esptool --chip esp32s3 --port COM7 --baud 460800 write-flash --flash-size 16MB 0x0 Bruce-1.16.1-waveshare28b-displayfix-merged.bin
~~~
Yükleme bittiğinde RESET'e basın. Seri monitör:
~~~powershell
py -m platformio device monitor --port COM7 --baud 115200
~~~
Eski sürüme dönmek için aynı write-flash komutunda
backup-working-28b/Bruce-waveshare_28b.bin dosyasını seçin.
Bağlantı güvenilir değilse baud hızını 115200'e düşürün.

## Kısa fiziksel test

1. Üç soğuk açılış yapın. Bruce sürümünü kontrol edin. RGB 2×614400 PSRAM log'u,
   free/largest değerleri kaydedilsin; allocation, retirement timeout, DISPLAY FAULT,
   StoreProhibited veya açılış döngüsü olmamalı.
2. Bir menüde 60 saniye bekleyin; sonra 30 kez menü/geri geçişi yapın.
   Statik görüntüde parıltı, satır kayması, yarım kare veya siyah flash kontrol edin.
   PREV/SEL/NEXT ve üst sol geri alanını, BOOT geri kontrolünü deneyin.
3. Wi-Fi taramasını tekrarlayın; uzun ağ listesinde ilk/son seçeneklere gidip geri dönün.
   Uzun SSID'nin yatay kayması, 15×20 yazılar ve footer'ın taşmadan okunması kontrol edilsin.
   Bağlantı kurarken ve ayar kaydı yaparken ekranı izleyin. Kullanıcının eski sürümde
   çalıştırdığı araçları aynı koşullarda başlatıp durdurarak regresyonu denetleyin.
4. Uzun bildirim, dosya listesi ve klavyeyi açın; portre/yatay rotasyon seçeneği varsa
   her ikisinde satır çakışması ve touch koordinatlarını kontrol edin. SD/LittleFS,
   USB CLI ve BLE için önceki kullanımınıza uygun temel işlemleri deneyin.
5. 15 dakika boyunca menü ve Wi-Fi yükünü tekrarlayın; ilk/son PSRAM ve internal
   free/largest log'larını karşılaştırın. Görüntüyü video ile kaydedip eski sürümle
   aynı parlaklıkta karşılaştırın; kamera rolling-shutter etkisini gözle de kontrol edin.

Başarı ölçütü: görüntü kararlı, metinler taşmıyor, touch işlevleri korunuyor,
yükte hata/yeniden başlama yok ve serbest bellekte sürekli azalma gözlenmiyor.
Sadece derleme SUCCESS bu ölçütleri sağlamaz.

## Kaynaktan yeniden derleme

Tam kaynak ZIP'ini çıkarın; proje kökünde:
~~~powershell
py -m pip install platformio==6.2.0
py -m platformio run -e waveshare_28b
~~~
Platform ve bağımlılıklar mevcut PlatformIO yapılandırmasından yüklenir.
build.py bootloader 0x0 + partition 0x8000 + app 0x10000 birleşimini
Bruce-waveshare_28b.bin olarak üretir.
Kaynak/firmware base commit ve sürüm bilgisi displayfix-build-result.json içinde.

Windows uzun yollar nedeniyle gerekirse proje klasörünü kısa bir sürücüye bağlayın:
~~~powershell
subst W: "C:\tam\yol\bruce-1.16.1-waveshare-flicker-fix"
cd W:\
py -m platformio run -e waveshare_28b
~~~
Mevcut W: sürücüsü varsa boş bir harf seçin. Python Scripts dizini PATH'te olmalıdır.
Host DMA/geometry testleri:
~~~powershell
py tests/waveshare_28b/display_contract.py g++
~~~
Test kodu kaynak içindeki gerçek sunum metotlarını çıkarıp mock olaylarla derler;
g++ C++11 desteği gerekir. Fiziksel DMA testi yerine geçmez.

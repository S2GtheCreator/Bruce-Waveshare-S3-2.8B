# Kurtarma/batarya imajı — yükleme ve kısa test

Yeni sürüm 1.16.1-ws28b-recovery-bat derlendi; fiziksel kararlılığı/batarya doğruluğu henüz test edilmedi.
Okunabilir görüntüye hemen dönmek için eski, kullanıcı tarafından doğrulanan
backup-working-28b/Bruce-waveshare_28b.bin dosyası korunuyor.
Yeni imaj kontrollü karşılaştırma için adaydır; önceki sürümden daha kötü olmadığı
sadece kaynak karşılaştırması/derlemeyle garanti edilemez.
1.16.1-waveshare28b-displayfix sürümü geri çekildi.

## USB yükleme

COM7 örnektir. Gerçek portu seçin; seri monitörü kapatın.
BOOT'a basılı tutup RESET'e basın, BOOT'u bırakın.
Birleşik imaj0x0 NVS aralığını da kapsar; kayıtlı ayarlar/gain sıfırlanabilir.
Mevcut cihaz flash/NVS yedeği isterseniz, yüklemeden önce:
~~~powershell
py -m pip install esptool
py -m esptool --chip esp32s3 --port COM7 --baud 460800 read-flash 0x0 0x1000000 board-backup-16MB.bin
~~~
Yeni aday:
~~~powershell
py -m esptool --chip esp32s3 --port COM7 --baud 460800 write-flash --flash-size 16MB 0x0 Bruce-1.16.1-ws28b-recovery-bat-merged.bin
~~~
Referansa dönüş aynı komutla eski Bruce-waveshare_28b.bin kullanılarak yapılır.
SD dosyaları ayrıca yedeklenir. Yükleme sonrası RESET.
~~~powershell
py -m platformio device monitor --port COM7 --baud 115200
~~~

## Ekranı dört ayrı koşulda teşhis

Config→LCD Diagnostics:
Fixed image / Touch polling OFF / WiFi OFF / WiFi scan ON.
Ekran okunmuyorsa, ana menüdeyken USB CLI girişleri:
~~~text
ws28b-fixed
ws28b-touch-off
ws28b-wifi-off
ws28b-wifi-on
ws28b-battery
~~~
Her ekran testi ana UI task'ına yönlendirilir; bir kere sabit resim çizilir.
60 saniyede biter; BOOT düğmesine basmak erken çıkarır.
BOOT'u reset sırasında tutmak test başlatmaz, ROM yükleyicisini seçer.
Wi-Fi isolasyonu AP/APSTA/WebUI aktifken reddedilir; önce bu servisleri durdurun.
WiFi ON testi STA taramasını her10 saniyede çalıştırır; parola/kalıcı config değiştirmez.
Çıkışta test taraması durdurulur, touch polling ve önceki Wi-Fi modu geri alınır;
eski STA bağlantısı varsa yeniden bağlantı asenkron başlar. BLE durumu değiştirilmez.
Touch OFF yalnızca GT911 polling'i durdurur; touch'ın beslemesi veya ortak I²C kapanmaz.

Aynı parlaklık/USB koşuluyla referans ve yeni adayı karşılaştırın:
1. Statik ekranda60s: siyah flash, renk kayması, tüm kare titremesi veya çizgi yırtılması.
2. Touch OFF: statik testle fark var mı; dönüşte touch/BOOT/menü düzgün mü.
3. WiFi OFF ve WiFi ON: tarama/bağlantı yükü görüntüyü bozuyor mu.
4. Normal menü/Wi-Fi ağ listesi:25% büyük yazılar taşmıyor mu; uzun SSID ve
   ilk/son seçenek kaydırması, PREV/SEL/NEXT alanları çalışıyor mu.
5. En az15dk normal kullanım; Wi-Fi/BLE/SD/CLI temel işlemleri, PSRAM free/largest
   eğilimi ve hata/yeniden başlama kaydı. Kalibrasyon veya ayar NVS yazımı sırasında
   görüntüyü ayrıca izleyin.

Test sırasında başka ekran çizen CLI komutlarını kullanmayın; sabit görüntü ölçümü bozulur.
Görüntü referanstan kötüleşirse referans BIN'e dönün ve hangi modda bozulduğunu kaydedin.
Sadece bir USB kameranın rolling shutter çizgileri yerine gözle görüntüyü de karşılaştırın.

## Batarya/güç test matrisi

| Koşul | Yapılacak gözlem |
|---|---|
| USB takılı, batarya sökülü | Kart USB ile açılmalı; ADC voltajı hücre varlığı sayılmamalı, source/charge unknown kalır |
| Batarya takılı, USB çıkarılmış | SW1 açıkken açılış ve ekran; voltaj BAT konnektörüyle karşılaştırılır |
| USB+batarya birlikte | Voltaj ve fiziksel şarj LED'i; yazılım LED'den şarj durumu algıladığını iddia etmez |
| SW1 kapalı, USB takılı | USB beslemesinin cihazı açık tutup tutmadığı kaydedilir |
| SW1 kapalı, USB çıkarılmış | Fiziksel batarya yolunun kesilmesi kontrol edilir; yazılım güç kesme testi değildir |

Config→Battery→Voltage/estimate veya ws28b-battery değerini multimetreyle
BAT-GND üzerinde karşılaştırın. 3:1 oran ve eFuse kalibrasyonu kaynakta doğrulandı;
gerçek doğruluk ölçümle belirlenir.
İsterseniz Calibrate with meter'a gerçek BAT voltajını girin; girilen değerin
hesaplanmış bir ADC değeri olmaması gerekir. Başarılı gain NVS'ye kaydedilir.
Farklı voltajlarda tekrar kontrol edin; tek nokta gain düzeltmesi tüm ADC eğrisini doğrulamaz.
Batarya-yalnız kullanımda USB seri bağlantısı kesilir; ölçümü ekrandan kaydedin.

Düşük voltaj davranışı yazılım eşikleri3.45V/3.60V ve üç5s örnektir.
LCD LOW, durum çubuğu yeniden çizildiğinde; seri uyarı eşik geçişinde görünür.
Donanım testi için uygun kontrollü batarya kaynağıyla voltajı ölçerek bu davranışı sınayın.
Şarj yüzdesi yaklaşık; gerçek kapasite/BMS testi yapılmış değildir.
Güvenli yazılım güç kesme desteklenmez. Deep Sleep, fiziksel güç kesme olarak değerlendirilmez.

## Kaynaktan derleme/test

~~~powershell
py -m pip install platformio==6.2.0
py -m platformio run -e waveshare_28b
py tests/waveshare_28b/recovery_contract.py g++
~~~
C++11 host compiler gerekir. PlatformIO bağımlılıkları mevcut yapılandırmadan yükler.
build.py bootloader0x0 + partition0x8000 + app0x10000 birleşimini
Bruce-waveshare_28b.bin adıyla üretir.
Windows uzun yol gerekirse kısa çalışma yolu kullanılmalıdır; bu derleme B: üzerinde yapıldı.
Arduino3.3.9 / pioarduino55.3.39 / IDF libs5.5.4+sha.735507283d / Xtensa GCC14.2 kullanıldı.
Yalnız firmware dosyasını yeniden adlandırmak yerine orijinal Bruce kaynakları derlendi.

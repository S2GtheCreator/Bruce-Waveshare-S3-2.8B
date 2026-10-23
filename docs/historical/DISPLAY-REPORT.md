> Historical report for the original pre-publication build. It does not certify the sanitized release binary. See ../../BUILD-REPORT.md.

# Touch UI Pro / ekran raporu

RGB panel/DMA kaynakları, ST7701 init dizisi, kart pinleri ve GT911/I²C/SD başlatma kodu başlangıç sürümüyle bayt düzeyinde aynıdır (`pro-preserved-source.json`). Çift framebuffer, yeni VSYNC callback'i, buffer swap veya cache/DMA deneyi eklenmedi. Bu nedenle önceki ağır displayfix sürücüsü kullanılmıyor. Titremenin fiziksel olarak giderildiği veya Pro'nun kartta daha kararlı olduğu iddia edilmiyor.

Resmî demo timing: 30 MHz; HSYNC pulse/back/front 10/70/60; VSYNC 10/20/20; yükselen PCLK. Bunlar korundu. Tek framebuffer 480×640×2 = 614.400 bayt PSRAM. Çalışan porttaki 480×10 piksellik iki dahili bounce tamponu toplam 19.200 bayt olarak korundu. Demo başlığındaki bounce makrosu 10×640 pikseldir; bu çalışmada mevcut portun tampon değeri değiştirilmedi.

Mevcut yaklaşık %25 büyütülmüş klasik font ölçüleri korundu. Ekran metni için 12 Türkçe harf glifi eklendi; USB HID eşlemesi değiştirilmedi. Tam Unicode fontu değildir; diğer desteklenmeyen ekran glifleri ? olarak görünür. HID'de desteklenmeyen Unicode ise açık hata verir.

Kaydırılabilir metin alanı: boş liste/sınır dışı erişim, tek adımlı kaydırma, unsigned taşma, yanlış font geri yükleme ve UTF-8 ortasından satır bölme düzeltildi. Görünüm sınırı 512 görsel satır, dosyadan okuma 32 KiB; aşımda kırpılma belirtilir. Uzun seçenek/dosya adları UTF-8 karakter sınırlarında kaydırılır. Mevcut büyük dokunmatik bölgeler, menü açılışındaki seçim bekleme koruması ve Prev/Sel/Next–Up/Down navigasyonu kullanılır; yeni kinetik swipe sürücüsü eklenmedi.

Shell (SSH/Telnet): sabit başlık/bağlam, ayrı çıktı ve giriş alanı, fonttan hesaplanan PTY ölçüleri, 256 görsel satırlık sınırlı geçmiş, yukarı/aşağı kaydırma, uzun girişte son bölümün gösterilmesi, Türkçe metin girişi ve <=10 Hz gövde yenileme. Geçmiş oturum sonunda serbest bırakılır. Ağ/SSH taşıma mantığı değiştirilmedi. BadUSB başlık/çıktı ölçüleri de fonta uyarlandı; Pro ilerleme alanı çıktıdan ayrıldı ve görünür STOP SCRIPT kontrolü eklendi.

`Config → Display & UI → Touch / Turkish text test`: dört köşe/merkez ve döndürmelerde eşlenmiş koordinat kontrolü. GT911 kalibrasyonuna tahmini ofset eklenmedi. `Memory snapshot` gerçek cihazda heap, en büyük bloklar, PSRAM ve UI stack marjını gösterir; bu değerler burada ölçülmedi.

Fiziksel kontrol: açılış/menüler, tüm döndürmeler, uzun dosya listesi, 200+ satırlık Türkçe Shell çıktısı; Wi-Fi kapalı/açık ve tarama sırasında 5'er dakika ekran gözlemi. Titreme artarsa önceki uygulamaya dönün ve karşılaştırmalı video/seri log alın. Donanım testi bekliyor.

Kaynaklar: [Waveshare kart dokümanı](https://docs.waveshare.com/ESP32-S3-Touch-LCD-2.8B), [resmî demo](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8B/ESP32-S3-Touch-LCD-2.8B-Demo.zip). Yerel resmî demo/schematic SHA kayıtları kart klasöründedir.

ARŞİV: Başarısız displayfix sürümüne aittir. Bu sürüm geri çekildi; güncel belge RECOVERY-BATARYA-RAPORU.md.

# PSRAM ve framebuffer raporu

Sürüm 1.16.1-waveshare28b-displayfix; yeni imaj üzerinde fiziksel bellek ölçümü henüz yapılmadı.

| Kaynak | Önceki port | Yeni port |
|---|---:|---:|
| PSRAM kapasitesi, OPI 80MHz | 8.388.608 | 8.388.608 bayt |
| RGB565 framebuffer adedi | 1 | 2 |
| Tek framebuffer | 614.400 | 614.400 bayt |
| Toplam RGB PSRAM tahsisi | 614.400 | 1.228.800 bayt |
| Dahili bounce tamponları | 19.200 | 0 bayt |
| Yeni sunum task stack'i | yok | 4.096 bayt + TCB |
| Yeni ELF .dram0.data + .dram0.bss | — | 120,108 bayt |
| Yeni fiziksel statik .ext_ram.* bölümleri | — | 0 bayt |
| Yeni .ext_ram.dummy sanal adres alanı | — | 3,669,984 bayt |

Önceki rapordaki 3.669.984 bayt “statik external RAM” ifadesi yanlıştı.
Bu değer .ext_ram.dummy NOLOAD linker bölümüdür. SDK sections.ld açıklaması
“Dummy section to skip flash rodata sections” der; fiziksel PSRAM framebuffer/heap tahsisi değildir.
Yeni ELF bölüm dökümü boards/waveshare_28b/elf-section-sizes.txt içinde.
Bu sanal alanı fiziksel PSRAM'dan tekrar düşerek serbest bellek hesaplamayın.

IDF fb_in_psram=true ve num_fbs=2 ile kendi DMA uyumlu framebuffer'larını ayırır.
PSRAM aktarım hizası 64 bayt; SRAM hizası 8 bayt. Yeni yol bounce_buffer_size_px=0.
CPU arka tampondaki cache'li adresi kullanır; gönderimde IDF C2M cache senkronizasyonunu yapar.
Framebuffer'lar IDF'den alınır; sıradan malloc ile oluşturulup DMA'ya verilmez.
Tahsis, panel reset/init ve callback kaydı hataları kontrol edilir; başarısızlıkta
arka ışık açılmadan hata bildirimiyle güvenli duruş yapılır.
I²C/GT911/TCA9554 başlatma sırası aynıdır.

Ek PSRAM ihtiyacı 614.400 bayt. İki framebuffer kapasitenin yaklaşık %14,65'idir.
Diğer Bruce heap/sprite/Wi-Fi/BLE harcamaları bu hesaba dahil değildir.
LVGL veya ilave LVGL çizim tamponu yoktur. Bruce sprite tamponları önceki checked PSRAM yolunda kalır.
Free heap, largest block ve task/network yükü ELF statik RAM toplamından çıkarılamaz.

30MHz/(620×690) ≈70,126Hz. Aktif RGB565 veri ortalaması yaklaşık
614.400×70,126 ≈43,09 MB/s (ondalık MB); aktif satır sırasında 60 MB/s.
Bunlar timing'den hesaplanan LCD veri talepleridir; ölçülmüş PSRAM bant genişliği değildir.
Doğrudan EDMA, CPU ve flash erişimleri mevcut bellek altyapısını paylaşır.
Gönderim/cache flush ile emekli tampona değişen aralık kopyası ek trafik üretir;
tam ekran değişince aralık tam framebuffer olabilir. 50ms birleştirme ve değişmeyen
içeriği atlama bu maliyeti azaltmayı amaçlar.

SDK gerçek ayarları: CONFIG_SPIRAM_MODE_OCT=1, CONFIG_SPIRAM_SPEED_80M=1,
CONFIG_LCD_RGB_RESTART_IN_VSYNC=1. PSRAM XIP ve LCD RGB ISR IRAM-safe Kconfig etkin değil;
bunları salt -D ekleyerek değiştirmiş gibi gösterilmedi. Kendi callback kodu IRAM,
olay sayaçları/lock internal DRAM; SDK'nın flash erişimi sırasında IRQ gecikmesi
yeni cihaz testinde izlenmelidir.

Açılışta RGB tahsisi sonrası PSRAM free/largest log'u; sonrasında her 60 saniyede
[RGB] PSRAM free/largest ve internal free/largest çıktısı vardır.
Açılış, menü, tarama, bağlantı, kayıt/NVS, BLE/SD yükü ve 15 dakikalık test
sonunda bu değerleri kaydedin. Ölçülmemiş runtime free değeri uydurulmadı.

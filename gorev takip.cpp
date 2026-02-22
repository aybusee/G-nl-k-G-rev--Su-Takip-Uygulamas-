#include <stdio.h>      // printf, scanf gibi giriþ-çýkýþ fonksiyonlarý
#include <stdlib.h>     // dosya iþlemleri ve sistem fonksiyonlarý
#include <string.h>     // metin iþlemleri (strlen, strcpy vs.)
#include <unistd.h>     // sleep() fonksiyonu için (Linux/Unix)
// Windows kullanýcýlarý için: #include <windows.h> // Sleep() kullanýlýr

#define MAKS_GOREV 200  // maksimum görev kapasitesi ayarlanýyor

// --------------------------------------------------
// Buffer temizleme – scanf sonrasý kalan \n’i siler
// --------------------------------------------------
void temizleBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);  // tamponu temizler
}

// Satýr metni güvenli okuma fonksiyonu
void satirOku(char *str, int boyut) {
    fgets(str, boyut, stdin);     // satýr okuma
    int uzunluk = strlen(str);
    if (uzunluk > 0 && str[uzunluk-1] == '\n')
        str[uzunluk-1] = '\0';   // sondaki newline’ý siler
}

// --------------------------------------------------
// GÖREV VERÝ YAPISI (struct) – tutulan bilgiler
// --------------------------------------------------
typedef struct {
    int id;
    char baslik[50];
    char aciklama[200];
    int oncelik;
    int kategori;
    int gun, ay, yil;
    int durum; // 0 bekliyor, 1 tamamlandý
} Gorev;


// --------------------------------------------------
// GLOBAL DEÐÝÞKENLER
// --------------------------------------------------
Gorev gorevler[MAKS_GOREV]; // görev listesi
int gorevSayisi = 0;        // toplam görev sayýsý

int gunlukSuHedefi = 0;     // günlük su hedefi
int icilenSu = 0;           // içilen su miktarý


// --------------------------------------------------
// KATEGORÝ ve ÖNCELÝK isimlendirme fonksiyonlarý
// --------------------------------------------------
const char* kategoriAdi(int k) { // kategori numarasina gore kategori adini metin olarak donduren fonksiyon
    switch (k) {
        case 1: return "Okul";
        case 2: return "Is";
        case 3: return "Kisisel";
        case 4: return "Saglik";
        case 5: return "Diger";
        default: return "Bilinmiyor";
    }
}

const char* oncelikAdi(int o) { // girilen oncelik numarasinin adini metin olarak donduren fonksiyon
    switch (o) {
        case 1: return "Yuksek";
        case 2: return "Orta";
        case 3: return "Dusuk";
        default: return "Bilinmiyor";
    }
}

// Menü ve listelerde çizgi basmak için fonksiyon
void cizgi() {
    printf("-------------------------------------------------------------------------------\n");
}

// Görev detayýný ekrana yazdýrýr
void gorevYazdir(const Gorev *g) {
    printf("[%3d] %-20s | Onc:%-7s | Kat:%-8s | Tarih:%02d/%02d/%04d | Durum:%s\n",
           g->id, g->baslik,
           oncelikAdi(g->oncelik),
           kategoriAdi(g->kategori),
           g->gun, g->ay, g->yil,
           g->durum ? "Tamamlandi" : "Bekliyor");
}


// --------------------------------------------------
// Tarih kýyaslama — sýralama için kullanýlýr
// --------------------------------------------------
int dahaErkenMi(Gorev a, Gorev b) {
    if (a.yil != b.yil) return a.yil < b.yil;
    if (a.ay != b.ay) return a.ay < b.ay;
    return a.gun < b.gun;
}


// --------------------------------------------------
// SIRALAMA FONKSÝYONLARI
// --------------------------------------------------
void onceligeGoreSirala() {
    // bubble-sort benzeri bir sýralama
    for (int i = 0; i < gorevSayisi - 1; i++)
        for (int j = i + 1; j < gorevSayisi; j++)
            if (gorevler[i].oncelik > gorevler[j].oncelik) {
                Gorev g = gorevler[i];
                gorevler[i] = gorevler[j];
                gorevler[j] = g;
            }
    printf("Gorevler oncelige gore siralandi.\n");
}

void tariheGoreSirala() {
    for (int i = 0; i < gorevSayisi - 1; i++)
        for (int j = i + 1; j < gorevSayisi; j++)
            if (!dahaErkenMi(gorevler[i], gorevler[j])) {
                Gorev g = gorevler[i];
                gorevler[i] = gorevler[j];
                gorevler[j] = g;
            }
    printf("Gorevler tarihe gore siralandi.\n");
}


// --------------------------------------------------
// LÝSTELEME
// --------------------------------------------------
void gorevleriListele() {
    printf("\nTUM GOREVLER:\n");
    cizgi();
    if (gorevSayisi == 0) { 
        printf("Gorev yok.\n");
        return;
    }
    for (int i = 0; i < gorevSayisi; i++) // 0’dan baþlayarak tüm görevleri sýrayla dolaþan döngüdür.
        gorevYazdir(&gorevler[i]);
}

void tamamlanmamisListele() {
    printf("\nTAMAMLANMAMIS GOREVLER:\n");
    cizgi();
    int var = 0; // hiç tamamlanmamýþ görev olup olmadýðýný kontrol eder.
    for (int i = 0; i < gorevSayisi; i++)
        if (!gorevler[i].durum) {
            gorevYazdir(&gorevler[i]);
            var = 1;
    // tum gorevleri tara, durumu 0 olanlari yazdir ve bulundu bilgisini kaydet  
        }
    if (!var) printf("Tamamlanmamis gorev yok.\n");
}

void kategoriListele() {
    int kat;
    printf("Kategori (1=Okul,2=Is,3=Kisisel,4=Saglik,5=Diger): ");
    scanf("%d", &kat);
    temizleBuffer();

    printf("\n%s kategorisi:\n", kategoriAdi(kat));
    cizgi();
    int var = 0;
    for (int i = 0; i < gorevSayisi; i++)
        if (gorevler[i].kategori == kat) {
            gorevYazdir(&gorevler[i]); //seçilen kategoriye ait görevi ekrana yazdýrýr.
            var = 1;
    // tum gorevleri tara, secilen kategoriye uygun olanlari yazdir ve bulundu bilgisini kaydet 
        }
    if (!var) printf("Bu kategoride gorev yok.\n");
}


// --------------------------------------------------
// GÖREV EKLEME
// --------------------------------------------------
void gorevEkle() {
    temizleBuffer();  // Buffer temizleme – scanf sonrasý kalan \n’i siler

    if (gorevSayisi >= MAKS_GOREV) { // dizi dolu mu kontrol edilir
        printf("Kapasite dolu.\n");
        return;
    }

    Gorev g;
    g.id = gorevSayisi + 1; //yeni eklenen göreve mevcut görev sayýsýna göre sýradaki benzersiz kimlik numarasýný atar.

    printf("\n--- YENI GOREV EKLE ---\n");

    printf("Gorev basligi: ");
    satirOku(g.baslik, 50);

    printf("Aciklama: ");
    satirOku(g.aciklama, 200);

    printf("Oncelik (1=Yuksek,2=Orta,3=Dusuk): ");
    scanf("%d", &g.oncelik);
    temizleBuffer();

    printf("Kategori (1=Okul,2=Is,3=Kisisel,4=Saglik,5=Diger): ");
    scanf("%d", &g.kategori);
    temizleBuffer();

    printf("Gun: ");
    scanf("%d", &g.gun);
    temizleBuffer();

    printf("Ay: ");
    scanf("%d", &g.ay);
    temizleBuffer();

    printf("Yil: ");
    scanf("%d", &g.yil);
    temizleBuffer();

    g.durum = 0;  

    gorevler[gorevSayisi++] = g; // yeni gorevi diziye ekler ve gorevSayisini 1 artirir

    printf("\nGorev eklendi!\n");
}


// --------------------------------------------------
// TAMAMLAMA
// --------------------------------------------------
void goreviTamamla() {
    int id;
    printf("Gorev ID: ");
    scanf("%d", &id);
    temizleBuffer();

    for (int i = 0; i < gorevSayisi; i++)
        if (gorevler[i].id == id) {
            gorevler[i].durum = 1;
            printf("Gorev tamamlandi.\n");
            return;
        }
    printf("ID bulunamadi.\n");
}


// --------------------------------------------------
// GÖREV SÝLME
// --------------------------------------------------
void gorevSil() {
    int id;
    printf("Silinecek ID: ");
    scanf("%d", &id);
    temizleBuffer();

    int idx = -1;
    for (int i = 0; i < gorevSayisi; i++)
        if (gorevler[i].id == id) idx = i;

    if (idx == -1) {
        printf("Bulunamadi.\n");
        return;
    }

    for (int i = idx; i < gorevSayisi - 1; i++) {
        gorevler[i] = gorevler[i + 1];    
        gorevler[i].id = i + 1;          
    }

    gorevSayisi--;
    printf("Gorev silindi.\n");
}


// --------------------------------------------------
// GÖREV HATIRLATICI
// --------------------------------------------------
void gorevHatirlatici() {
    int id, sure;
    printf("Hatirlatilacak gorev ID: ");
    scanf("%d", &id);
    temizleBuffer();

    int idx = -1;
    for (int i = 0; i < gorevSayisi; i++)
        if (gorevler[i].id == id) idx = i;

    if (idx == -1) {
        printf("Gorev yok.\n");
        return;
    }

    printf("Kac saniye sonra?: ");
    scanf("%d", &sure);
    temizleBuffer();

    printf("Hatirlatici basladi...\n");
    sleep(sure);

    printf("\n*** HATIRLATICI ***\nGorev zamani: %s\n", gorevler[idx].baslik);
}


// --------------------------------------------------
// DOSYA KAYDET / YÜKLE
// --------------------------------------------------
void gorevKaydet() {
    FILE *f = fopen("gorevler.txt", "w");
    if (!f) return;

    for (int i = 0; i < gorevSayisi; i++)
        fprintf(f, "%d|%s|%s|%d|%d|%d|%d|%d|%d\n",
                gorevler[i].id, gorevler[i].baslik, gorevler[i].aciklama,
                gorevler[i].oncelik, gorevler[i].kategori,
                gorevler[i].gun, gorevler[i].ay, gorevler[i].yil,
                gorevler[i].durum);

    fclose(f);
}

void gorevYukle() {
    FILE *f = fopen("gorevler.txt", "r");
    if (!f) return;

    Gorev g;
    while (fscanf(f, "%d|%49[^|]|%199[^|]|%d|%d|%d|%d|%d|%d\n",
                  &g.id, g.baslik, g.aciklama,
                  &g.oncelik, &g.kategori,
                  &g.gun, &g.ay, &g.yil, &g.durum) == 9)
        gorevler[gorevSayisi++] = g;

    fclose(f);
}


// --------------------------------------------------
// SU TAKÝP SÝSTEMÝ
// --------------------------------------------------
void suHedefBelirle() {
    printf("Gunluk hedef (ml): ");
    scanf("%d", &gunlukSuHedefi);
    temizleBuffer(); // Buffer temizleme – scanf sonrasý kalan \n’i siler
    printf("Hedef kaydedildi.\n");
}

void suEkle() {
    int x;
    printf("Icilen su (ml): ");
    scanf("%d", &x);
    temizleBuffer();
    icilenSu += x; // icilen su miktarini toplama ekle
    printf("%d ml eklendi.\n", x);
}

void suDurumu() {
    printf("\n--- SU DURUMU ---\n");
    printf("Hedef: %d ml\n", gunlukSuHedefi);
    printf("Icilen: %d ml\n", icilenSu);
}

void suHatirlatici() {
    int t;
    printf("Kac saniye sonra hatirlatsin?: ");
    scanf("%d", &t);
    temizleBuffer();

    sleep(t);
    printf("\n*** SU HATIRLATICI ***\nBIR BARDAK SU IC!\n");
}

void suKaydet() {
    FILE *f = fopen("su.txt", "w"); // su.txt dosyasini yazmak icin ac (w = overwrite)
    if (!f) return; // dosya acilamazsa fonksiyondan cik
    fprintf(f, "%d|%d", gunlukSuHedefi, icilenSu); // gunluk hedef ve icilen suyu '|' ile ayirarak dosyaya yaz
    fclose(f); // dosyayi kapat
}

void suYukle() {
    FILE *f = fopen("su.txt", "r"); // su.txt dosyasini okumak icin ac (r = read)
    if (!f) return;
    fscanf(f, "%d|%d", &gunlukSuHedefi, &icilenSu); // dosyadaki iki sayiyi oku ve ilgili degiskenlere ata
    fclose(f);
}


// --------------------------------------------------
// SU MENÜSÜ
// --------------------------------------------------
void suMenu() {
    int c;
    while (1) {
        cizgi();
        printf("                SU MENUSU\n");
        cizgi();
        printf("1 - Su hedefi ayarla\n");
        printf("2 - Su ekle\n");
        printf("3 - Su durumunu goster\n");
        printf("4 - Su hatirlaticisi\n");
        printf("0 - Geri don\n");
        cizgi();
        printf("Secim: ");
        scanf("%d", &c);
        temizleBuffer();

        if (c == 0) return; // Kullanýcý 0’a basarsa, su menüsünden çýkar ve ana menüye geri döner.
        if (c == 1) suHedefBelirle(); // Kullanýcý 1’e bastýðýnda, suHedefBelirle() fonksiyonu çaðrýlýr.
        else if (c == 2) suEkle(); // Kullanýcý 2 seçtiðinde, suEkle() çalýþýr.
        else if (c == 3) suDurumu(); // Kullanýcý 3 seçtiðinde, suDurumu() çalýþýr.
        else if (c == 4) suHatirlatici(); // Kullanýcý 4'e basýnca, suHatirlatici() çalýþýr.
        else printf("Gecersiz secim.\n");
    }
}


// --------------------------------------------------
// SIRALAMA / FÝLTRE MENÜSÜ
// --------------------------------------------------
void siralamaMenu() {
    int c; // kullanici secimini tutacak degisken
    while (1) { // menu sürekli acik kalacak, cikis secilene kadar doner
        cizgi();
        printf("               SIRALAMA / FILTRE MENUSU\n");
        cizgi();
        printf("1 - Oncelige gore sirala\n");
        printf("2 - Tarihe gore sirala\n");
        printf("3 - Kategoriye gore listele\n");
        printf("4 - Sadece tamamlanmamis gorevleri goster\n");
        printf("0 - Ana menuye don\n");
        cizgi();
        printf("Secim: "); // kullanicidan secim alma satiri
        scanf("%d", &c);  // secimi oku
        temizleBuffer(); // Buffer temizleme – scanf sonrasý kalan \n’i siler

        if (c == 0) return; // 0 secildi ise menu kapanýr ve ana menuye donulur
        if (c == 1) { onceligeGoreSirala(); gorevleriListele(); } // 1 secildi ise: önce gorevler onceliðe göre siralanýr, sonra listelenir
        else if (c == 2) { tariheGoreSirala(); gorevleriListele(); } // 2 secildi ise: gorevler tarihe gore siralanir ve listelenir
        else if (c == 3) kategoriListele(); // 3 secildi ise kategori filtresi uygulanir ve uygun gorevler gosterilir
        else if (c == 4) tamamlanmamisListele(); // 4 secildi ise sadece durumu tamamlanmamis gorevler listelenir
        else printf("Gecersiz secim.\n"); // yukaridakilerin hicbiri degilse hata mesaji verilir
    }
}


// --------------------------------------------------
// ANA MENÜ
// --------------------------------------------------
void anaMenu() {
    cizgi();
    printf("    GUNLUK GOREV + SU TAKIP UYGULAMASI\n");
    cizgi();
    printf("1 - Gorevleri listele\n");
    printf("2 - Yeni gorev ekle\n");
    printf("3 - Gorevi tamamla\n");
    printf("4 - Gorev sil\n");
    printf("5 - Siralama menusu\n");
    printf("6 - Gorev hatirlaticisi\n");
    printf("7 - Su menusu\n");
    printf("8 - Kaydet\n");
    printf("0 - Cikis\n");
    cizgi();
    printf("Secim: ");
}


// --------------------------------------------------
// MAIN PROGRAM AKIÞI
// --------------------------------------------------
int main() {
    gorevYukle();  // kayýtlý görevleri yükler
    suYukle();     // kayýtlý su verilerini yükler

    int secim;
    while (1) { // program, cikis secilene kadar surekli doner (sonsuz dongu)
        anaMenu();          // ana menüyü göster
        scanf("%d", &secim);
        temizleBuffer(); // Buffer temizleme – scanf sonrasý kalan \n’i siler

        if (secim == 0) { // eger kullanici 0'a basarsa cikis islemi yapilir
            gorevKaydet();  // görevleri kaydet
            suKaydet();     // su takibini kaydet
            printf("Cikiliyor...\n");
            return 0;
        }
        if (secim == 1) gorevleriListele(); 
        else if (secim == 2) gorevEkle();
        else if (secim == 3) goreviTamamla();
        else if (secim == 4) gorevSil();
        else if (secim == 5) siralamaMenu();
        else if (secim == 6) gorevHatirlatici();
        else if (secim == 7) suMenu();
        else if (secim == 8) { gorevKaydet(); suKaydet(); printf("Kaydedildi.\n"); } 
		// 8: elle kaydetme secenegi, görevleri ve su verilerini kaydeder 
        else printf("Gecersiz secim.\n"); // menude olmayan bir sayi girilirse uyarir
    }
}


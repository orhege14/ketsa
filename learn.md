# Ketsa Programming Language — Öğren

Ketsa, Python benzeri sözdizimi ile C++ performansını birleştiren statik tipli bir programlama dilidir.

**Başlarken:** `ketsa run dosya.ketsa` ile çalıştır, `ketsa doctor` ile sistem kontrolü yap.

---

## 1. İlk Program

```ketsa
// Bu bir yorum satırı
print "Merhaba Ketsa!"
println("Yeni satır ile yazdır")
```

```ketsa
// Değişken kullanarak
let mesaj = "Hello World"
print mesaj
```

---

## 2. Değişkenler ve Sabitler

```ketsa
let x = 42              // tür çıkarımı: int
let pi = 3.14           // float
let isim = "Ketsa"      // string
let aktif = true        // bool
let bos = null          // null

// Açık tür bildirimi
let y: int = 100
let oran: float = 2.5

// Sabit (değiştirilemez)
const SABIT = 10
```

---

## 3. Temel Tipler

| Tip    | Örnek               |
|--------|---------------------|
| int    | `42`, `-7`, `0`     |
| float  | `3.14`, `-0.5`      |
| string | `"merhaba"`         |
| bool   | `true`, `false`     |
| char   | `'a'`, `'\n'`       |
| null   | `null`              |

```ketsa
let c: char = 'K'
let yeni_satir = '\n'
```

---

## 4. Operatörler

### Aritmetik
```ketsa
let a = 10
let b = 3
print a + b   // 13
print a - b   // 7
print a * b   // 30
print a / b   // 3 (int bölme)
print a % b   // 1 (mod)
print a ** b  // 1000 (üs: 10^3)
```

### Karşılaştırma
```ketsa
print a == b   // false
print a != b   // true
print a > b    // true
print a < b    // false
print a >= b   // true
print a <= b   // false
```

### Mantıksal
```ketsa
let dogru = true
let yanlis = false
print dogru && yanlis  // false
print dogru || yanlis  // true
print !dogru           // false
```

### Bitwise
```ketsa
print 5 & 3    // 1  (AND)
print 5 | 3    // 7  (OR)
print 5 ^ 3    // 6  (XOR)
print 5 << 1   // 10 (left shift)
print 5 >> 1   // 2  (right shift)
```

---

## 5. String İşlemleri

```ketsa
let selam = "Merhaba"
let isim = "Ketsa"
print selam + " " + isim + "!"  // Merhaba Ketsa!
print toString(42)               // "42"
print len("Ketsa")               // 5
print type(42)                   // "int"
```

**Escape karakterleri:** `\n` (yeni satır), `\t` (tab), `\"` (tırnak), `\\` (backslash)

---

## 6. Atama Operatörleri

```ketsa
let x = 10
x += 5    // x = 15
x -= 3    // x = 12
x *= 2    // x = 24
x /= 4    // x = 6
x %= 3    // x = 0
x **= 2   // x = 0 (0^2)
```

---

## 7. Kontrol Akışı

### If / Else If / Else
```ketsa
let skor = 85
if (skor >= 90) {
    print "A"
} else if (skor >= 80) {
    print "B"
} else {
    print "C"
}
// Çıktı: B
```

`elif` de kullanılabilir:
```ketsa
if (skor >= 90) {
    print "A"
} elif (skor >= 80) {
    print "B"
} else {
    print "C"
}
```

### Switch / Case
```ketsa
let renk = 2
switch (renk) {
    case 1:
        print "Kırmızı"
    case 2:
        print "Yeşil"
    case 3:
        print "Mavi"
    default:
        print "Bilinmeyen"
}
// Çıktı: Yeşil
```

---

## 8. Döngüler

### While
```ketsa
let i = 0
while (i < 5) {
    print i
    i += 1
}
// Çıktı: 0 1 2 3 4
```

### For-In
```ketsa
for (el in [10, 20, 30]) {
    print el
}
// Çıktı: 10 20 30
```

### Break / Continue
```ketsa
let i = 0
while (i < 10) {
    i += 1
    if (i == 3) continue   // 3'ü atla
    if (i == 7) break      // 7'de dur
    print i
}
// Çıktı: 1 2 4 5 6
```

---

## 9. Diziler

```ketsa
let bos = []
let sayilar = [1, 2, 3, 4, 5]
print sayilar[0]        // 1
print len(sayilar)      // 5
print toString(sayilar) // "[1, 2, 3, 4, 5]"

sayilar[0] = 99
print sayilar[0]        // 99

// range() ile dizi oluşturma
let r1 = range(5)          // [0, 1, 2, 3, 4]
let r2 = range(1, 5)       // [1, 2, 3, 4]
```

---

## 10. Pattern Matching (match)

```ketsa
let deger = 3
match (deger) {
    case 1:
        print "Bir"
    case 3:
        print "Üç"
    else:
        print "Diğer"
}
// Çıktı: Üç
```

---

## 11. Fonksiyonlar

```ketsa
// Parametreli ve dönüş tipli
func topla(a: int, b: int): int {
    return a + b
}
print topla(5, 3)  // 8

// String parametre
func selamla(isim: string): string {
    return "Merhaba " + isim
}
print selamla("Ketsa")  // Merhaba Ketsa

// Void fonksiyon (dönüş tipi yok)
func yaz(sayi: int) {
    print sayi
}

// Recursive (fibonacci)
func fib(n: int): int {
    if (n <= 1) return n
    return fib(n - 1) + fib(n - 2)
}
print fib(10)  // 55
```

---

## 12. Sınıflar ve Nesneler

```ketsa
class Nokta {
    x
    y
    
    func toplam(): int {
        return x + y
    }
}

let p = new Nokta()
p.x = 10
p.y = 20
print p.x               // 10
print p.toplam()        // 30
```

### Object Literal
```ketsa
let kisi = { isim: "Ali", yas: 25 }
print kisi.isim  // Ali
kisi.yas = 26
```

---

## 13. Modüller (İçe Aktarma)

Dosya sistemi ile çalışır:

```ketsa
// mylib.ketsa dosyasını içe aktar
import mylib

// std/ dizininden içe aktar
import std.core

// Belirli sembolleri içe aktar
from mylib import fonksiyon1, fonksiyon2
```

---

## 14. Tip Dönüşümleri

```ketsa
print toInt("42")       // 42
print toFloat(3)        // 3.0
print toString(42)      // "42"
print toString(3.14)    // "3.14"
print toString(true)    // "true"
```

---

## 15. Yerleşik (Built-in) Fonksiyonlar

| Fonksiyon  | Açıklama                   | Örnek                     |
|------------|----------------------------|---------------------------|
| `print`    | Yazdır (alt satırsız)       | `print "merhaba"`         |
| `println`  | Yazdır (alt satırlı)        | `println("merhaba")`      |
| `input`    | Kullanıcıdan girdi al       | `let ad = input("Ad: ")`  |
| `type`     | Değerin tipini döndürür     | `type(42)` → `"int"`      |
| `len`      | Uzunluk (string/dizi)       | `len("abc")` → `3`        |
| `toString` | String'e çevir              | `toString(42)` → `"42"`   |
| `toInt`    | Integer'a çevir             | `toInt("42")` → `42`      |
| `toFloat`  | Float'a çevir               | `toFloat(3)` → `3.0`      |
| `range`    | Dizi aralığı oluştur        | `range(1, 5)` → `[1,2,3,4]` |

---

## 16. Tam Kapsamlı Örnek

```ketsa
// ---- Ketsa Demo ----

let isim = "Ketsa"
let versiyon = 3
const PI = 3.14159

print "=== " + isim + " v" + toString(versiyon) + " ==="

// Fonksiyon
func daire_alani(r: float): float {
    return PI * r * r
}

print "Alan: " + toString(daire_alani(5.0))

// Koşul
let yas = 18
if (yas >= 18) {
    println("Reşit")
} else {
    println("Reşit değil")
}

// Dizi + döngü
let sayilar = range(1, 6)
for (s in sayilar) {
    print s
}

// Match
match (yas) {
    case 18:
        print "Tam 18"
    else:
        print "Başka"
}

// Sınıf
class Araba {
    marka
    model
    
    func bilgi(): string {
        return marka + " " + model
    }
}

let arac = new Araba()
arac.marka = "Tesla"
arac.model = "Model 3"
print arac.bilgi()

// Object literal
let config = { dil: "Ketsa", aktif: true }
print config.dil

println("=== Bitti ===")
```

---

## Hata Kodları

| Kod  | Anlamı                   |
|------|--------------------------|
| 1xxx | Lexer hatası             |
| 2xxx | Parser hatası            |
| 3xxx | Tip hatası               |
| 4xxx | Runtime hatası           |
| 5xxx | Modül hatası             |

---

## CLI Komutları

| Komut                     | Açıklama              |
|---------------------------|-----------------------|
| `ketsa run dosya.ketsa`   | Dosyayı çalıştır      |
| `ketsa --version`         | Sürümü göster         |
| `ketsa doctor`            | Sistem kontrolü       |
| `ketsa new proje_adi`     | Yeni proje oluştur    |

---

---

## 17. V2 — Gelişmiş Tip Sistemi

### Union Tipleri
```ketsa
let status: int | string = 200
status = "ok"
```

### Optional Tipler
```ketsa
let yas: int? = null
let deger: int? = 42
```

### Tip Alias
```ketsa
type UserId = int
let uid: UserId = 100
```

### Generic Fonksiyonlar
```ketsa
func identity<T>(value: T): T {
    return value
}
print identity(42)
print identity("hello")
```

Generic kısıtlamalar:
```ketsa
func max<T extends Comparable>(a: T, b: T): T {
    if (a > b) return a
    return b
}
```

---

## 18. V2 — Immutable-by-Default (Bellek Güvenliği)

```ketsa
let x = 10          // immutable (varsayılan)
// x = 20           // HATA: const değişkene atama yapılamaz

var y = 20          // mutable
y = 30              // çalışır

const SABIT = 42    // her zaman immutable
```

---

## 19. V2 — Lambda İfadeleri

```ketsa
let double = (x: int) => x * 2
print double(5)               // 10

let topla = (a: int, b: int) => a + b
print topla(3, 4)            // 7
```

---

## 20. V2 — Varsayılan Parametreler

```ketsa
func selamla(isim: string = "Dünya") {
    print "Merhaba " + isim
}
selamla()              // Merhaba Dünya
selamla("Ketsa")       // Merhaba Ketsa
```

---

## 21. V2 — İsimli Argümanlar

```ketsa
func baglan(host: string, port: int) {
    print host + ":" + toString(port)
}
baglan(host: "localhost", port: 8080)
```

---

## 22. V2 — Hata Yönetimi (try/catch/throw)

```ketsa
try {
    throw "Bir hata oluştu"
} catch (msg) {
    print "Yakalandı: " + msg
}
```

Tipli catch:
```ketsa
try {
    // riskli kod
} catch (e: string) {
    print e
}
```

---

## 23. V2 — Fonksiyonel Operasyonlar

```ketsa
let sayilar = [1, 2, 3, 4, 5]

// map: her elemana fonksiyon uygula
func kare(x: int): int {
    return x * x
}
print map(sayilar, kare)        // [1, 4, 9, 16, 25]

// filter: koşula uyanları seç
func cift(x: int): bool {
    return x % 2 == 0
}
print filter(sayilar, cift)     // [2, 4]

// reduce: biriktirerek işle
func topla_accum(acc: int, x: int): int {
    return acc + x
}
print reduce(sayilar, topla_accum, 0)  // 15

// foreach: her eleman için çalıştır
func yazdir(x: int) {
    println(x)
}
foreach(sayilar, yazdir)
```

---

## 24. V2 — Generic Sınıflar

```ketsa
class Kutu<T> {
    deger

    func al(): T {
        return deger
    }
}

let intKutu = new Kutu()
intKutu.deger = 42
print intKutu. al()
```

---

## 25. V2 — Interface Sistemi

```ketsa
interface Yazdirilabilir {
    func yazdir()
}

class Rapor {
    icerik

    func yazdir() {
        print icerik
    }
}
// class Rapor implements Yazdirilabilir
```

---

*Ketsa V2 — Modern, güvenli ve yüksek performanslı programlama dili.*

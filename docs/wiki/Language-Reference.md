# Language Reference

## Lexical Yapı

### Yorumlar

```ketsa
// Tek satır yorum

/*
   Çok satırlı yorum
*/
```

### Anahtar Kelimeler

```
print     let       const     if        else      elif
while     for       in        func      return    break
continue  class     struct    import    from      as
true      false     null      match     case      default
switch    new       this      base      int       float
string    bool      void      any       public    private
static    final     abstract  enum      interface extends
```

### Tanımlayıcılar

Harf veya alt çizgi ile başlar, harf, rakam veya alt çizgi içerebilir:

```ketsa
let yas = 25
let _ozel = "gizli"
let degisken1 = 100
```

Bazı anahtar kelimeler (`pi`, `version`) değişken adı olarak kullanılabilir.

### Literaller

| Tür | Örnek |
|---|---|
| **Integer** | `42`, `-10`, `0` |
| **Float** | `3.14`, `-0.5`, `100.0` |
| **String** | `"merhaba"`, `"escape \n \t"` |
| **Char** | `'a'`, `'\n'`, `'0'` |
| **Boolean** | `true`, `false` |
| **Null** | `null` |

String kaçış dizileri:
| Dizi | Anlamı |
|---|---|
| `\n` | Yeni satır |
| `\t` | Sekme |
| `\r` | Satır başı |
| `\0` | Null karakter |
| `\\` | Ters bölü |
| `\"` | Çift tırnak |

### Operatörler

#### Aritmetik
| Operatör | Anlamı |
|---|---|
| `+` | Toplama / Birleştirme |
| `-` | Çıkarma |
| `*` | Çarpma |
| `/` | Bölme |
| `%` | Mod alma |
| `**` | Üs alma |

#### Karşılaştırma
| Operatör | Anlamı |
|---|---|
| `==` | Eşitlik |
| `!=` | Eşitsizlik |
| `>` | Büyüktür |
| `<` | Küçüktür |
| `>=` | Büyük eşit |
| `<=` | Küçük eşit |
| `<=>` | Üç yönlü karşılaştırma |

#### Mantıksal
| Operatör | Anlamı |
|---|---|
| `&&` | Ve |
| `\|\|` | Veya |
| `^^` | XOR |
| `!` | Değil |

#### Atama
| Operatör | Anlamı |
|---|---|
| `=` | Atama |
| `+=` | Toplayarak atama |
| `-=` | Çıkararak atama |
| `*=` | Çarparak atama |
| `/=` | Bölerek atama |
| `%=` | Mod alarak atama |
| `**=` | Üs alarak atama |

#### Bitwise
| Operatör | Anlamı |
|---|---|
| `&` | Bit AND |
| `\|` | Bit OR |
| `^` | Bit XOR |
| `<<` | Sola kaydırma |
| `>>` | Sağa kaydırma |
| `~` | Bit NOT |

#### Diğer
| Operatör | Anlamı |
|---|---|
| `..` | Aralık |
| `->` | Ok (fonksiyon) |
| `=>` | Fat arrow |
| `?.` | Opsiyonel erişim |
| `?:` | Elvis operatörü |

---

## Tip Sistemi

### İlkel Tipler

| Tip | Açıklama | Örnek |
|---|---|---|
| `int` | Tamsayı (64-bit) | `42` |
| `float` | Ondalıklı sayı (64-bit) | `3.14` |
| `string` | Metin | `"hello"` |
| `bool` | Mantıksal | `true`, `false` |
| `char` | Karakter | `'a'` |
| `void` | Geri dönüşsüz | `func foo(): void {}` |
| `any` | Herhangi bir tip | `let x: any = 42` |
| `null` | Boş değer | `null` |

### Bileşik Tipler

| Tip | Açıklama | Örnek |
|---|---|---|
| `[int]` | Tamsayı dizisi | `[1, 2, 3]` |
| `[string]` | Metin dizisi | `["a", "b"]` |
| `object` | Nesne | `{ x: 1, y: 2 }` |
| `(int) -> bool` | Fonksiyon | `func isEven(n): bool` |

### Tip Belirtme

```ketsa
let x: int = 42
let isim: string = "Ali"
let oran: float = 3.14
let aktif: bool = true
```

### Tip Çıkarımı

Tip belirtilmezse, derleyici değerden tipi çıkarır:

```ketsa
let x = 42        // int
let pi = 3.14     // float
let isim = "Ali"  // string
```

---

## Değişkenler

### Değişken Tanımlama (`let`)

```ketsa
let x = 42
let isim = "Ketsa"
let pi: float = 3.14
```

### Sabitler (`const`)

```ketsa
const PI = 3.14159
const SITE = "ketsa.dev"
// PI = 3.14  // HATA: sabit değiştirilemez
```

### Atama

```ketsa
let x = 10
x = 20            // Basit atama
x += 5            // x = x + 5
x -= 3            // x = x - 3
x *= 2            // x = x * 2
x /= 4            // x = x / 4
```

---

## İfadeler

### Aritmetik İfadeler

```ketsa
let toplam = 10 + 20
let fark = 100 - 30
let carpim = 6 * 7
let bolum = 42 / 2
let kalan = 17 % 5
let us = 2 ** 10        // 1024
```

### Karşılaştırma İfadeleri

```ketsa
10 == 20      // false
10 != 20      // true
10 > 20       // false
10 < 20       // true
10 >= 10      // true
10 <= 5       // false
```

### Mantıksal İfadeler

```ketsa
true && false   // false
true || false   // true
!true           // false
true ^^ false   // true (XOR)
```

### String Birleştirme

```ketsa
let selam = "Merhaba " + "Dunya"  // "Merhaba Dunya"
```

### Öncelik Sırası

1. `**` (üs)
2. `*`, `/`, `%` (çarpma/bölme)
3. `+`, `-` (toplama/çıkarma)
4. `==`, `!=`, `<`, `>`, `<=`, `>=` (karşılaştırma)
5. `&&` (ve)
6. `||` (veya)
7. `=` (atama)

---

## Kontrol Akışı

### If / Else If / Else

```ketsa
let skor = 85

if (skor >= 90) {
    print "A"
} else if (skor >= 80) {
    print "B"
} else if (skor >= 70) {
    print "C"
} else {
    print "F"
}
```

### While Döngüsü

```ketsa
let sayac = 0
while (sayac < 5) {
    print sayac
    sayac += 1
}
```

### For Döngüsü

```ketsa
for (i in [1, 2, 3, 4, 5]) {
    print i
}

for (isim in ["Ali", "Veli", "Ayse"]) {
    print "Merhaba " + isim
}
```

### Break / Continue

```ketsa
let n = 0
while (n < 10) {
    n += 1
    if (n == 3) {
        continue    // 3'ü atla
    }
    if (n == 7) {
        break       // 7'de dur
    }
    print n
}
// Çıktı: 1, 2, 4, 5, 6
```

### Switch

```ketsa
switch (skor) {
    case 100:
        print "Mukemmel!"
    case 85:
        print "Harika!"
    default:
        print "Devam et!"
}
```

---

## Fonksiyonlar

### Fonksiyon Tanımlama

```ketsa
// Parametresiz, dönüşsüz
func selamla() {
    print "Merhaba!"
}

// Parametreli
func karsila(isim: string) {
    print "Merhaba " + isim
}

// Dönüş değerli
func topla(a: int, b: int): int {
    return a + b
}

// Tip belirtmeden
func ekranaYaz(deger) {
    print toString(deger)
}
```

### Fonksiyon Çağırma

```ketsa
selamla()
karsila("Ali")
let sonuc = topla(5, 3)
```

### Erken Dönüş

```ketsa
func faktoriyel(n: int): int {
    if (n <= 1) return 1
    return n * faktoriyel(n - 1)
}
```

---

## Diziler

### Dizi Literalleri

```ketsa
let sayilar = [1, 2, 3, 4, 5]
let isimler = ["Ali", "Veli"]
let karisik = [1, "metin", true]
```

### Dizi Erişimi

```ketsa
let ilk = sayilar[0]       // 1
let son = sayilar[4]       // 5
sayilar[0] = 10            // Değiştirme
```

### Dizi İşlemleri

```ketsa
let dizi = [1, 2, 3]
print len(dizi)             // 3
print dizi                  // [1, 2, 3]
```

---

## Pattern Matching

### Match İfadesi

```ketsa
let deger = 3

match (deger) {
    case 1:
        print "Bir"
    case 2:
        print "Iki"
    case 3:
        print "Uc"
    else:
        print "Diger"
}
```

### Desen Türleri

#### Literal Deseni
```ketsa
case 42:
case "merhaba":
case true:
```

#### Değişken Deseni
```ketsa
case x:        // Değeri x'e bağla
```

#### Wildcard Deseni
```ketsa
case _:        // Her şeyi eşle
```

#### Dizi Yapı Bozma
```ketsa
case [a, b, c]:  // [1, 2, 3] -> a=1, b=2, c=3
```

#### Nesne Yapı Bozma
```ketsa
case Point { x, y }:  // Point nesnesini parçala
```

#### Guard Koşulu
```ketsa
case x if (x > 10):   // Sadece x > 10 ise
```

---

## Sınıflar ve Nesneler

### Sınıf Tanımlama

```ketsa
class Kisi {
    // Alanlar
    isim
    yas

    // Metod
    func selam() {
        print "Merhaba, ben " + isim
    }

    func yaslandir() {
        yas += 1
    }
}
```

### Nesne Oluşturma

```ketsa
let kisi = new Kisi()
kisi.isim = "Ali"
kisi.yas = 25
kisi.selam()          // "Merhaba, ben Ali"
```

### Kalıtım

```ketsa
class Hayvan {
    func sesCikar() {
        print "..."
    }
}

class Kopek extends Hayvan {
    func sesCikar() {
        print "Hav hav!"
    }
}
```

### Nesne Literalleri

```ketsa
let nokta = { x: 10, y: 20 }
print nokta.x         // 10
nokta.y = 30
```

---

## Modüller

### Import

```ketsa
import matematik
import std.io
```

### From Import

```ketsa
from matematik import topla, carp
from std.io import print, readLine
```

### Modül Yapısı

Modüller şu yollarla aranır:
1. `modul_adı.ketsa`
2. `std/modul_adı.ketsa`
3. `lib/modul_adı.ketsa`
4. `modul_adı/mod.ketsa`

---

## Yerleşik Fonksiyonlar

| Fonksiyon | İmza | Açıklama |
|---|---|---|
| `print` | `print(value)` | Değeri yazdır (yeni satırsız) |
| `println` | `println(value)` | Değeri yazdır (yeni satırlı) |
| `input` | `input(prompt: string): string` | Kullanıcıdan girdi al |
| `type` | `type(value): string` | Değerin tipini döndür |
| `len` | `len(value): int` | Uzunluğu döndür |
| `toString` | `toString(value): string` | Metne dönüştür |
| `toInt` | `toInt(value): int` | Tamsayıya dönüştür |
| `toFloat` | `toFloat(value): float` | Ondalıklıya dönüştür |
| `range` | `range(start, end, step?): [int]` | Aralık oluştur |

---

## Program Yapısı

### Tam Örnek

```ketsa
// Ketsa Programı

// Değişkenler
let isim = "Ketsa"
let surum = 3
const PI = 3.14159

// Fonksiyon
func daireAlan(yaricap: float): float {
    return PI * yaricap * yaricap
}

// Kontrol akışı
let yas = 25
if (yas >= 18) {
    print "Yetiskin"
} else {
    print "Cocuk"
}

// Döngü
for (i in range(0, 5)) {
    print i
}

// Dizi
let sayilar = [1, 2, 3, 4, 5]
print "Toplam: " + toString(len(sayilar))

// Pattern matching
match (sayilar[0]) {
    case 1:
        print "Bir"
    else:
        print "Diger"
}
```

---

## Tip Dönüşümleri

### Implicit Dönüşümler

```ketsa
let x: float = 42    // int -> float (otomatik)
```

### Explicit Dönüşümler

```ketsa
let x = toInt("42")      // string -> int: 42
let y = toFloat(3)       // int -> float: 3.0
let z = toString(42)     // int -> string: "42"
```

---

## Hata Kodları

### Lexer Hataları (1xxx)
| Kod | Açıklama |
|---|---|
| 1001 | Beklenmeyen karakter |
| 1002 | Sonlandırılmamış string |
| 1005 | Geçersiz char literal |

### Parser Hataları (2xxx)
| Kod | Açıklama |
|---|---|
| 2001 | Beklenmeyen token |
| 2011 | Geçersiz atama hedefi |
| 2013 | Yinelenen parametre |

### Tip Hataları (3xxx)
| Kod | Açıklama |
|---|---|
| 3001 | Tip uyuşmazlığı |
| 3002 | Tanımsız değişken |
| 3004 | Sabite atama |
| 3009 | Yanlış argüman sayısı |
| 3013 | Fonksiyon dışında return |

### Runtime Hataları (4xxx)
| Kod | Açıklama |
|---|---|
| 4001 | Sıfıra bölme |
| 4003 | İndeks sınır dışı |
| 4004 | Stack taşması |

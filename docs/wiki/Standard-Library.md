# Standard Library

## core.ketsa

Ketsa standart kütüphanesi `std/core.ketsa` dosyasında bulunur. Temel yardımcı fonksiyonlar ve sabitler sağlar.

### İçe Aktarma

```ketsa
import core
```

### Fonksiyonlar

#### `typeOf(value): string`

Bir değerin tipini metin olarak döndürür.

```ketsa
print typeOf(42)        // "int"
print typeOf(3.14)      // "float"
print typeOf("merhaba") // "string"
print typeOf(true)      // "boolean"
```

#### `rangeTo(end: int): [int]`

0'dan başlayıp `end`'e kadar (hariç) tamsayı dizisi oluşturur.

```ketsa
let sayilar = rangeTo(5)  // [0, 1, 2, 3, 4]
```

### Sabitler

| Sabit | Değer |
|---|---|
| `PI` | 3.141592653589793 |
| `E` | 2.718281828459045 |

### Kullanım

```ketsa
import core

// Matematik
let daireAlan = PI * 5 * 5

// Tip sorgulama
print typeOf("test")

// Aralık
for (i in rangeTo(10)) {
    print i
}
```

---

## Yerleşik Fonksiyonlar (Built-in)

Bu fonksiyonlar import gerektirmez, doğrudan kullanılabilir.

### print

```ketsa
print(value)
```

Değeri standart çıktıya yazdırır. Yeni satır eklemez.

```ketsa
print "Merhaba"
print 42
print 3.14
print true
```

### println

```ketsa
println(value)
```

Değeri yazdırır ve sonuna yeni satır ekler.

```ketsa
println "Satir 1"
println "Satir 2"
```

### input

```ketsa
input(prompt: string): string
```

Kullanıcıdan girdi alır. İsteğe bağlı prompt mesajı gösterir.

```ketsa
let isim = input("Adiniz: ")
print "Merhaba " + isim
```

### type

```ketsa
type(value): string
```

Bir değerin tipini metin olarak döndürür.

```ketsa
print type(42)        // "int"
print type("text")    // "string"
print type([1, 2])    // "[int]"
```

| Değer | Dönüş |
|---|---|
| `42` | `"int"` |
| `3.14` | `"float"` |
| `"hello"` | `"string"` |
| `true` | `"boolean"` |
| `null` | `"null"` |
| `[1, 2]` | `"[int]"` |
| `{a: 1}` | `"object"` |

### len

```ketsa
len(value): int
```

Dizi veya string uzunluğunu döndürür.

```ketsa
print len("Ketsa")    // 5
print len([1, 2, 3])  // 3
```

### toString

```ketsa
toString(value): string
```

Herhangi bir değeri metne dönüştürür.

```ketsa
toString(42)      // "42"
toString(3.14)    // "3.14"
toString(true)    // "true"
toString([1, 2])  // "[1, 2]"
```

### toInt

```ketsa
toInt(value): int
```

Değeri tamsayıya dönüştürür.

```ketsa
toInt("42")      // 42
toInt(3.14)      // 3
toInt("abc")     // 0 (hata durumunda)
```

### toFloat

```ketsa
toFloat(value): float
```

Değeri ondalıklı sayıya dönüştürür.

```ketsa
toFloat(42)      // 42.0
toFloat("3.14")  // 3.14
toFloat("abc")   // 0.0 (hata durumunda)
```

### range

```ketsa
range(start: int, end: int, step?: int): [int]
```

Tamsayı aralığı oluşturur.

```ketsa
range(0, 5)          // [0, 1, 2, 3, 4]
range(1, 10, 2)      // [1, 3, 5, 7, 9]
range(10, 0, -2)     // [10, 8, 6, 4, 2]
range(5)             // [0, 1, 2, 3, 4]
```

---

## Tip Dönüşüm Tablosu

| Kaynak | toInt | toFloat | toString |
|---|---|---|---|
| `42` | `42` | `42.0` | `"42"` |
| `3.14` | `3` | `3.14` | `"3.14"` |
| `"42"` | `42` | `42.0` | `"42"` |
| `"3.14"` | `3` | `3.14` | `"3.14"` |
| `"abc"` | `0` | `0.0` | `"abc"` |
| `true` | `-` | `-` | `"true"` |
| `null` | `0` | `0.0` | `"null"` |

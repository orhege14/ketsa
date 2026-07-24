# Examples

## Merhaba Dünya

```ketsa
print "Hello Ketsa!"
```

## Değişkenler ve Tipler

```ketsa
let isim = "Ketsa"
let surum = 3
let pi = 3.14
let harika = true
const SABIT = 42
let tipli: int = 100
```

## String İşlemleri

```ketsa
let selam = "Merhaba"
let dunya = "Dunya"
print selam + " " + dunya
print "Uzunluk: " + toString(len(selam + " " + dunya))
```

## Aritmetik Operatörler

```ketsa
let a = 10
let b = 3

print "a + b = " + toString(a + b)    // 13
print "a - b = " + toString(a - b)    // 7
print "a * b = " + toString(a * b)    // 30
print "a / b = " + toString(a / b)    // 3
print "a % b = " + toString(a % b)    // 1
print "a ** 3 = " + toString(a ** 3)  // 1000
```

## Bileşik Atama

```ketsa
let x = 5
x += 10    // x = 15
x -= 3     // x = 12
x *= 2     // x = 24
x /= 3     // x = 8
```

## Karşılaştırma

```ketsa
let a = 10
let b = 3

print a == b    // false
print a != b    // true
print a > b     // true
print a < b     // false
print a >= b    // true
print a <= b    // false
```

## Boolean Mantığı

```ketsa
print true && false   // false
print true || false   // true
print !true           // false
```

## If / Else If / Else

```ketsa
let skor = 85

if (skor >= 90) {
    print "Grade: A"
} else if (skor >= 80) {
    print "Grade: B"
} else if (skor >= 70) {
    print "Grade: C"
} else {
    print "Grade: F"
}
```

## While Döngüsü

```ketsa
let sayac = 0
while (sayac < 5) {
    print "Count: " + toString(sayac)
    sayac += 1
}
```

## For Döngüsü

```ketsa
for (i in [1, 2, 3, 4, 5]) {
    print "For: " + toString(i)
}
```

## Break / Continue

```ketsa
let n = 0
while (n < 10) {
    n += 1
    if (n == 3) { continue }
    if (n == 7) { break }
    print "n: " + toString(n)
}
// Çıktı: 1, 2, 4, 5, 6
```

## Fonksiyonlar

```ketsa
// Basit fonksiyon
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

// Recursive
func fibonacci(n: int): int {
    if (n <= 1) return n
    return fibonacci(n - 1) + fibonacci(n - 2)
}

// Kullanım
print fibonacci(10)  // 55
```

## Diziler

```ketsa
let dizi = [1, 2, 3, 4, 5]
print "Dizi: " + toString(dizi)
print "Uzunluk: " + toString(len(dizi))
print "Ilk: " + toString(dizi[0])
print "Son: " + toString(dizi[4])
```

## Pattern Matching

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

## Switch

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

## Sınıflar

```ketsa
class Kisi {
    isim
    yas

    func selam() {
        print "Merhaba, ben " + isim
    }
}

let ali = new Kisi()
ali.isim = "Ali"
ali.yas = 25
ali.selam()
```

## Nesne Literalleri

```ketsa
let nokta = { x: 10, y: 20 }
print nokta.x    // 10
```

## Aralık (Range)

```ketsa
let r = range(0, 5)
print r  // [0, 1, 2, 3, 4]

for (i in range(1, 10, 2)) {
    print i  // 1, 3, 5, 7, 9
}
```

## Tip Sorgulama

```ketsa
print type(42)        // "int"
print type(3.14)      // "float"
print type("hello")   // "string"
print type(true)      // "boolean"
print type(null)      // "null"
print type([1, 2])    // "[int]"
```

## Kullanıcı Girdisi

```ketsa
let isim = input("Adiniz: ")
print "Merhaba, " + isim + "!"
```

## Fibonacci Karşılaştırması

```ketsa
func fib(n: int): int {
    if (n <= 1) return n
    return fib(n - 1) + fib(n - 2)
}

print "fib(10) = " + toString(fib(10))
print "fib(20) = " + toString(fib(20))
print "fib(30) = " + toString(fib(30))
```

## Tam Program Örneği

```ketsa
// Ketsa Tam Program

// Sabitler
const PI = 3.14159

// Fonksiyon
func daireAlan(yaricap: float): float {
    return PI * yaricap * yaricap
}

// Değişkenler
let isim = "Ketsa"
let surum = 3

// Kontrol akışı
if (surum >= 3) {
    print isim + " v" + toString(surum) + " hazir!"
}

// Dizi ve döngü
for (i in range(0, 5)) {
    print "Sayi: " + toString(i)
}

// Pattern matching
match (surum) {
    case 1:
        print "Eski surum"
    case 3:
        print "Guncel surum"
    else:
        print "Diger"
}

// Sınıf
class Araba {
    marka
    model
    func bilgi() {
        print marka + " " + model
    }
}

let araba = new Araba()
araba.marka = "Ketsa"
araba.model = "X1"
araba.bilgi()

// Hesaplama
let alan = daireAlan(5.0)
print "Alan: " + toString(alan)
```

## Kapsamlı Test

```ketsa
// Ketsa Kapsamlı Test
let name = "Ketsa"
let ver = 3
let pi_val = 3.14
let awesome = true
const con = 42
let tv: int = 100

print "=== Ketsa Language Test ==="
print "Name: " + name
print "Version: " + toString(ver)
print "Pi: " + toString(pi_val)
print "Constant: " + toString(con)

let a = 10, b = 3
print "a + b = " + toString(a + b)
print "a - b = " + toString(a - b)
print "a * b = " + toString(a * b)
print "a / b = " + toString(a / b)

let score = 85
if (score >= 90) { print "Grade: A" }
else if (score >= 80) { print "Grade: B" }
else { print "Grade: F" }

let cnt = 0
while (cnt < 3) {
    print cnt
    cnt += 1
}

func add(a: int, b: int): int { return a + b }
print "add(5,3) = " + toString(add(5, 3))

let arr = [1, 2, 3]
print "len: " + toString(len(arr))
print "first: " + toString(arr[0])

match (3) {
    case 1: print "One"
    case 3: print "Three"
    else:   print "Other"
}

print "=== Done ==="
```

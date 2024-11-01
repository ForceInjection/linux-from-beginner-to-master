# 各类安全证书简介及操作

## 一、证书标准

### X.509  
X.509 是一种证书标准，定义了证书应包含的内容。详情可参考 [RFC5280](https://link.jianshu.com/?t=https%3A%2F%2Fzh.wikipedia.org%2Fwiki%2FX.509)，SSL 证书使用该标准。  
证书常见编码格式：PEM 和 DER，文件扩展名可能不同（如 `.pem`，`.cer`，`.der`），但可相互转换。  

### 文件形式的证书格式
- **P12（含私钥）**：PKCS#12 标准，二进制格式，后缀 `.pfx`，常用于 Windows。
- **DER（二进制编码，无私钥）**：后缀 `.cer`，常用于 Java、Linux。
- **PEM（Base64 编码，无私钥）**：后缀 `.cer`，常用于 Apache。

## 二、常见证书格式

### 1. PEM 格式  
PEM（Privacy Enhanced Mail）是 OpenSSL 常用的格式，包含 `-----BEGIN CERTIFICATE-----` 和 `-----END CERTIFICATE-----`。  
特点：
- Base64 编码的 ASCII 文件
- 文件扩展名：`.pem`, `.crt`, `.cer`, `.key`
- 适用于 Apache 服务器

**查看证书信息**：

```bash
$ openssl x509 -in certificate.pem -text -noout
```

### 2. DER 格式  
DER 是二进制格式的证书，常用于 Java 平台。它是 PEM 格式的编码基础，可以相互转换。  
**查看证书信息**：

```bash
$ openssl x509 -in certificate.der -inform der -text -noout
```

### 3. PFX/P12 格式  
PFX/P12 是包含私钥和证书链的二进制格式，常用于 Windows。  
**查看证书信息**：

```bash
$ openssl pkcs12 -in certificate.pfx -info
```

## 三、其他格式

- **JKS**：Java 使用的 KeyStore 格式，适用于 Tomcat、WebLogic。
- **KDB**：IBM 使用的 Key Database 文件格式，适用于 WebSphere。
- **OCSP**：在线证书状态协议，用于实时验证证书有效性。
- **CRL**：证书吊销列表，离线方式发布已撤销的证书。
- **SCEP**：简单证书注册协议，可自动处理证书登记过程。

## 四、证书不同格式间的转换

### 1. PEM 转 DER
```bash
$ openssl x509 -outform der -in certificate.pem -out certificate.der
```

### 2. PEM 转 P7B
```bash
$ openssl crl2pkcs7 -nocrl -certfile certificate.cer -out certificate.p7b -certfile CAcert.cer
```

### 3. PEM 转 PFX
```bash
$ openssl pkcs12 -export -out certificate.pfx -inkey privateKey.key -in certificate.crt -certfile CAcert.crt
```

### 4. DER 转 PEM
```bash
$ openssl x509 -inform der -in certificate.cer -out certificate.pem
```

### 5. P7B 转 PEM
```bash
$ openssl pkcs7 -print_certs -in certificate.p7b -out certificate.cer
```

### 6. PFX 转 PEM
```bash
$ openssl pkcs12 -in certificate.pfx -out certificate.cer -nodes
```

## 五、其他扩展名的证书文件

### 1. CRT
- **定义**: 常见于 Unix 系统，编码可能是 PEM 或 DER。

### 2. CER
- **定义**: 常见于 Windows 系统，编码可能是 PEM 或 DER。

### 3. KEY
- **定义**: 通常用来存放公钥或私钥，编码可能是 PEM 或 DER。

## 五、一些其他格式的介绍

### 1. JKS
- **定义**: Java Key Storage，Java 专用格式。

### 2. KDB
- **定义**: IBM 的 WEB 服务器、应用服务器、中间件使用。

### 3. OCSP
- **定义**: 在线证书状态协议，用于实时表明证书状态。

### 4. CRL
- **定义**: 证书吊销列表，用于公布某些数字证书不再有效。

### 5. SCEP
- **定义**: 简单证书注册协议，基于文件的证书登记方式。


## 六、参考资料

1.  [What are the differences between PEM, DER, P7B/PKCS#7, PFX/PKCS#12 certificates](https://myonlineusb.wordpress.com/2011/06/19/what-are-the-differences-between-pem-der-p7bpkcs7-pfxpkcs12-certificates/)
2.  [那些证书相关的玩意儿（SSL,X.509,PEM,DER,CRT,CER,KEY,CSR,P12等）](http://www.360doc.com/content/15/0520/10/21412_471902987.shtml) 
3.  [常见数字证书及协议介绍](https://www.jianshu.com/p/f32852523f1b)
4.  [RSA私钥和公钥文件格式 (pkcs#7, pkcs#8, pkcs#12, pem)](https://blog.csdn.net/tuhuolong/article/details/42778945)
5.  [ASN.1 key structures in DER and PEM](https://mbed-tls.readthedocs.io/en/latest/kb/cryptography/asn1-key-structures-in-der-and-pem/)